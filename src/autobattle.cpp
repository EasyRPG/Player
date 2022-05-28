/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */
#include "autobattle.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_battlealgorithm.h"
#include "game_battle.h"
#include "algo.h"
#include "player.h"
#include "output.h"
#include "rand.h"
#include <lcf/reader_util.h>
#include <lcf/data.h>

namespace AutoBattle {

#ifdef EP_DEBUG_AUTOBATTLE
template <typename... Args>
static void DebugLog(const char* fmt, Args&&... args) {
	Output::Debug(fmt, std::forward<Args>(args)...);
}
#else
template <typename... Args>
static void DebugLog(const char*, Args&&...) {}
#endif

constexpr decltype(RpgRtCompat::name) RpgRtCompat::name;
constexpr decltype(AttackOnly::name) AttackOnly::name;
constexpr decltype(RpgRtImproved::name) RpgRtImproved::name;

std::unique_ptr<AlgorithmBase> CreateAlgorithm(StringView name) {
	if (Utils::StrICmp(name, RpgRtImproved::name) == 0) {
		return std::make_unique<RpgRtImproved>();
	}
	if (Utils::StrICmp(name, AttackOnly::name) == 0) {
		return std::make_unique<AttackOnly>();
	}
	if (Utils::StrICmp(name, RpgRtCompat::name) != 0) {
		static bool warned = false;
		if (!warned) {
			Output::Debug("Invalid AutoBattle algo name `{}' falling back to {} ...", name, RpgRtCompat::name);
			warned = true;
		}
	}
	return std::make_unique<RpgRtCompat>();
}

void AlgorithmBase::SetAutoBattleAction(Game_Actor& source) {
	vSetAutoBattleAction(source);
	if (source.GetBattleAlgorithm() == nullptr) {
		source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::None>(&source));
	}
}

void RpgRtCompat::vSetAutoBattleAction(Game_Actor& source) {
	SelectAutoBattleActionRpgRtCompat(source, Game_Battle::GetBattleCondition());
}

void AttackOnly::vSetAutoBattleAction(Game_Actor& source) {
	SelectAutoBattleAction(source, Game_Battler::WeaponAll, Game_Battle::GetBattleCondition(), false, false, false, false);
}

void RpgRtImproved::vSetAutoBattleAction(Game_Actor& source) {
	SelectAutoBattleAction(source, Game_Battler::WeaponAll, Game_Battle::GetBattleCondition(), true, false, false, false);
}

static int CalcSkillCostAutoBattle(const Game_Actor& source, const lcf::rpg::Skill& skill, bool emulate_bugs) {
	// RPG_RT autobattle ignores half sp cost modifier
	return emulate_bugs
		? Algo::CalcSkillCost(skill, source.GetMaxSp(), false)
		: source.CalculateSkillCost(skill.ID);
}

double CalcSkillHealAutoBattleTargetRank(const Game_Actor& source, const Game_Battler& target, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs) {
	assert(Algo::IsNormalOrSubskill(skill));
	assert(Algo::SkillTargetsAllies(skill));

	const double src_max_sp = source.GetMaxSp();
	const double tgt_max_hp = target.GetMaxHp();
	const double tgt_hp = target.GetHp();

	if (target.GetHp() > 0) {
		// Can the skill heal the target?
		if (!skill.affect_hp) {
			return 0.0;
		}

		const double base_effect = Algo::CalcSkillEffect(source, target, skill, apply_variance, false, cond, false);
		const double max_effect = std::min(base_effect, tgt_max_hp - tgt_hp);

		auto rank = static_cast<double>(max_effect) / static_cast<double>(tgt_max_hp);
		if (src_max_sp > 0) {
			const double cost = CalcSkillCostAutoBattle(source, skill, emulate_bugs);
			rank -= cost / src_max_sp / 8.0;
			rank = std::max(rank, 0.0);
		}
		return rank;
	}

	// Can the skill revive the target?
	if (skill.state_effects.size() > 1 && skill.state_effects[0]) {
		// BUG: RPG_RT does not check the reverse_state_effect flag to skip skills which would kill party members
		if (emulate_bugs || !skill.reverse_state_effect) {
			return static_cast<double>(skill.power) / 1000.0 + 1.0;
		}
	}
	return 0.0;
}

double CalcSkillDmgAutoBattleTargetRank(const Game_Actor& source, const Game_Battler& target, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs) {
	assert(Algo::IsNormalOrSubskill(skill));
	assert(Algo::SkillTargetsEnemies(skill));

	if (!(skill.affect_hp && target.Exists())) {
		return 0.0;
	}

	double rank = 0.0;
	const double src_max_sp = source.GetMaxSp();
	const double tgt_hp = target.GetHp();

	const double base_effect = Algo::CalcSkillEffect(source, target, skill, apply_variance, false, cond, false);
	rank = std::min(base_effect, tgt_hp) / tgt_hp;
	if (rank == 1.0) {
		rank = 1.5;
	}
	if (src_max_sp > 0) {
		const double cost = CalcSkillCostAutoBattle(source, skill, emulate_bugs);
		rank -= cost / src_max_sp / 4.0;
		rank = std::max(rank, 0.0);
	}

	// Bonus if the target is the first existing enemy?
	for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
		if (enemy->Exists()) {
			if (enemy == &target) {
				rank = rank * 1.5 + 0.5;
			}
			break;
		}
	}

	return rank;
}

double CalcSkillAutoBattleRank(const Game_Actor& source, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs) {
	if (!source.IsSkillUsable(skill.ID)) {
		return 0.0;
	}
	if (!Algo::IsNormalOrSubskill(skill)) {
		return 0.0;
	}

	double rank = 0.0;
	switch (skill.scope) {
		case lcf::rpg::Skill::Scope_ally:
			for (auto* target: Main_Data::game_party->GetActors()) {
				auto target_rank = CalcSkillHealAutoBattleTargetRank(source, *target, skill, cond, apply_variance, emulate_bugs);
				rank = std::max(rank, target_rank);
				DebugLog("AUTOBATTLE: Actor {} Check Skill Single Ally {} Rank : {}({}): {} -> {}", source.GetName(), target->GetName(), skill.name, skill.ID, rank, target_rank);
			}
			break;
		case lcf::rpg::Skill::Scope_party:
			for (auto* target: Main_Data::game_party->GetActors()) {
				auto target_rank = CalcSkillHealAutoBattleTargetRank(source, *target, skill, cond, apply_variance, emulate_bugs);
				rank += target_rank;
				DebugLog("AUTOBATTLE: Actor {} Check Skill Party Ally {} Rank : {}({}): {} -> {}", source.GetName(), target->GetName(), skill.name, skill.ID, rank, target_rank);
			}
			break;
		case lcf::rpg::Skill::Scope_enemy:
			for (auto* target: Main_Data::game_enemyparty->GetEnemies()) {
				auto target_rank = CalcSkillDmgAutoBattleTargetRank(source, *target, skill, cond, apply_variance, emulate_bugs);
				rank = std::max(rank, target_rank);
				DebugLog("AUTOBATTLE: Actor {} Check Skill Single Enemy {} Rank : {}({}): {} -> {}", source.GetName(), target->GetName(), skill.name, skill.ID, rank, target_rank);
			}
			break;
		case lcf::rpg::Skill::Scope_enemies:
			for (auto* target: Main_Data::game_enemyparty->GetEnemies()) {
				auto target_rank = CalcSkillDmgAutoBattleTargetRank(source, *target, skill, cond, apply_variance, emulate_bugs);
				rank += target_rank;
				DebugLog("AUTOBATTLE: Actor {} Check Skill Party Enemy {} Rank : {}({}): {} -> {}", source.GetName(), target->GetName(), skill.name, skill.ID, rank, target_rank);
			}
			break;
		case lcf::rpg::Skill::Scope_self:
			rank = CalcSkillHealAutoBattleTargetRank(source, source, skill, cond, apply_variance, emulate_bugs);
			DebugLog("AUTOBATTLE: Actor {} Check Skill Self Rank : {}({}): {}", source.GetName(), skill.name, skill.ID, rank);
			break;
	}
	if (rank > 0.0) {
		rank += Rand::GetRandomNumber(0, 99) / 100.0;
	}
	return rank;
}

double CalcNormalAttackAutoBattleTargetRank(const Game_Actor& source,
		const Game_Battler& target,
		Game_Battler::Weapon weapon,
		lcf::rpg::System::BattleCondition cond,
		bool apply_variance,
		bool emulate_bugs)
{
	if (!target.Exists()) {
		return 0.0;
	}
	const bool is_critical_hit = false;
	const bool is_charged = false;

	// RPG_RT BUG: Normal damage variance is not used
	// Note: RPG_RT does not do the "2k3_enemy_row_bug" when computing autobattle ranks.
	double base_effect = Algo::CalcNormalAttackEffect(source, target, weapon, is_critical_hit, is_charged, apply_variance, cond, false);
	// RPG_RT BUG: Dual Attack is ignored
	if (!emulate_bugs) {
		base_effect *= source.GetNumberOfAttacks(weapon);
	}
	const double tgt_hp = target.GetHp();

	auto rank = std::min(base_effect, tgt_hp) / tgt_hp;
	if (rank == 1.0) {
		rank = 1.5;
	}
	if (!emulate_bugs) {
		// EasyRPG customization - include sp cost of weapon attack using same logic as skill attack
		const auto cost = std::min(source.CalculateWeaponSpCost(weapon), source.GetSp());
		if (cost > 0) {
			const double src_max_sp = source.GetMaxSp();
			rank -= static_cast<double>(cost) / src_max_sp / 4.0;
			rank = std::max(rank, 0.0);
		}
	}

	// Bonus if the target is the first existing enemy?
	for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
		if (enemy->Exists()) {
			if (enemy == &target) {
				rank = rank * 1.5 + 0.5;
			}
			break;
		}
	}
	if (rank > 0.0) {
		rank = Rand::GetRandomNumber(0, 99) / 100.0 + rank * 1.5;
	}
	return rank;
}

double CalcNormalAttackAutoBattleRank(const Game_Actor& source, Game_Battler::Weapon weapon, const lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs) {
	double rank = 0.0;
	std::vector<Game_Battler*> targets;
	Main_Data::game_enemyparty->GetBattlers(targets);

	if (!emulate_bugs && source.HasAttackAll(weapon)) {
		for (auto* target: targets) {
			auto target_rank = CalcNormalAttackAutoBattleTargetRank(source, *target, weapon, cond, apply_variance, emulate_bugs);
			rank += target_rank;
			DebugLog("AUTOBATTLE: Actor {} Check Attack Party Enemy {} Rank : {} -> {}", source.GetName(), target->GetName(), rank, target_rank);
		}
	} else {
		for (auto* target: targets) {
			auto target_rank = CalcNormalAttackAutoBattleTargetRank(source, *target, weapon, cond, apply_variance, emulate_bugs);
			rank = std::max(rank, target_rank);
			DebugLog("AUTOBATTLE: Actor {} Check Attack Single Enemy {} Rank : {} -> {}", source.GetName(), target->GetName(), rank, target_rank);
		}
	}
	return rank;
}

void SelectAutoBattleAction(Game_Actor& source,
		Game_Battler::Weapon weapon,
		lcf::rpg::System::BattleCondition cond,
		bool do_skills,
		bool attack_variance,
		bool skill_variance,
		bool emulate_bugs)
{
	double skill_rank = 0.0;
	lcf::rpg::Skill* skill = nullptr;

	// Find the highest ranking skill
	if (do_skills) {
		for (auto& skill_id: source.GetSkills()) {
			auto* candidate_skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
			if (candidate_skill) {
				const auto rank = CalcSkillAutoBattleRank(source, *candidate_skill, cond, skill_variance, emulate_bugs);
				DebugLog("AUTOBATTLE: Actor {} Check Skill Rank : {}({}): {}", source.GetName(), candidate_skill->name, candidate_skill->ID, rank);
				if (rank > skill_rank) {
					skill_rank = rank;
					skill = candidate_skill;
				}
			}
		}
		DebugLog("AUTOBATTLE: Actor {} Best Skill Rank : {}({}): {}", source.GetName(), skill->name, skill->ID, skill_rank);
	}

	double normal_attack_rank = CalcNormalAttackAutoBattleRank(source, weapon, cond, attack_variance, emulate_bugs);
	DebugLog("AUTOBATTLE: Actor {} Normal Attack Rank : {}", source.GetName(), normal_attack_rank);

	auto best_target_rank = 0.0;
	Game_Battler* best_target = nullptr;
	std::vector<Game_Battler*> targets;

	if (skill != nullptr && normal_attack_rank < skill_rank) {
		// Choose Skill Target
		switch (skill->scope) {
			case lcf::rpg::Skill::Scope_enemies:
				DebugLog("AUTOBATTLE: Actor {} Select Skill Target : ALL ENEMIES", source.GetName());
				source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(&source, Main_Data::game_enemyparty.get(), *skill));
				return;
			case lcf::rpg::Skill::Scope_party:
				DebugLog("AUTOBATTLE: Actor {} Select Skill Target : ALL ALLIES", source.GetName());
				source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(&source, Main_Data::game_party.get(), *skill));
				return;
			case lcf::rpg::Skill::Scope_enemy:
				for (auto* target: Main_Data::game_enemyparty->GetEnemies()) {
					const auto target_rank = CalcSkillDmgAutoBattleTargetRank(source, *target, *skill, cond, skill_variance, emulate_bugs);
					if (target_rank > best_target_rank) {
						best_target_rank = target_rank;
						best_target = target;
					}
				}
				break;
			case lcf::rpg::Skill::Scope_ally:
				for (auto* target: Main_Data::game_party->GetActors()) {
					const auto target_rank = CalcSkillHealAutoBattleTargetRank(source, *target, *skill, cond, skill_variance, emulate_bugs);
					if (target_rank > best_target_rank) {
						best_target_rank = target_rank;
						best_target = target;
					}
				}
				break;
			case lcf::rpg::Skill::Scope_self:
				best_target = &source;
				break;
		}
		if (best_target) {
			DebugLog("AUTOBATTLE: Actor {} Select Skill Target : {}", source.GetName(), best_target->GetName());
			source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(&source, best_target, *skill));
		}
		return;
	}
	// Choose normal attack
	if (source.HasAttackAll(weapon)) {
		DebugLog("AUTOBATTLE: Actor {} Select Attack Target : ALL ENEMIES", source.GetName());
		source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(&source, Main_Data::game_enemyparty.get()));
		return;
	}

	for (auto* target: Main_Data::game_enemyparty->GetEnemies()) {
		const auto target_rank = CalcNormalAttackAutoBattleTargetRank(source, *target, weapon, cond, attack_variance, emulate_bugs);
		// On case of ==, prefer the first enemy
		if (target_rank > best_target_rank) {
			best_target_rank = target_rank;
			best_target = target;
		}
	}

	if (best_target != nullptr) {
		DebugLog("AUTOBATTLE: Actor {} Select Attack Target : {}", source.GetName(), best_target->GetName());
		source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(&source, best_target));
		return;
	}
}

} // namespace AutoBattle
