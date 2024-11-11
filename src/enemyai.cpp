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
#include "enemyai.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_battlealgorithm.h"
#include "game_battle.h"
#include "algo.h"
#include "player.h"
#include "output.h"
#include "rand.h"
#include <lcf/reader_util.h>
#include <lcf/data.h>

namespace EnemyAi {

#ifdef EP_DEBUG_ENEMYAI
template <typename... Args>
static void DebugLog(const char* fmt, Args&&... args) {
	Output::Debug(fmt, std::forward<Args>(args)...);
}
#else
template <typename... Args>
static void DebugLog(const char*, Args&&...) {}
#endif

constexpr decltype(RpgRtCompat::name) RpgRtCompat::name;
constexpr decltype(RpgRtImproved::name) RpgRtImproved::name;

static std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> MakeAttack(Game_Enemy& enemy, int hits) {
	return std::make_shared<Game_BattleAlgorithm::Normal>(&enemy, Main_Data::game_party->GetRandomActiveBattler(), hits);
}

static std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> MakeAttackAllies(Game_Enemy& enemy, int hits) {
	return std::make_shared<Game_BattleAlgorithm::Normal>(&enemy, Main_Data::game_enemyparty->GetRandomActiveBattler(), hits);
}


std::unique_ptr<AlgorithmBase> CreateAlgorithm(StringView name) {
	if (Utils::StrICmp(name, RpgRtImproved::name) == 0) {
		return std::make_unique<RpgRtImproved>();
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

void AlgorithmBase::SetEnemyAiAction(Game_Enemy& source) {
	vSetEnemyAiAction(source);
	if (source.GetBattleAlgorithm() == nullptr) {
		source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::None>(&source));
	}
}

void RpgRtCompat::vSetEnemyAiAction(Game_Enemy& source) {
	SelectEnemyAiActionRpgRtCompat(source, true);
}

void RpgRtImproved::vSetEnemyAiAction(Game_Enemy& source) {
	SelectEnemyAiActionRpgRtCompat(source, false);
}

static std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> MakeBasicAction(Game_Enemy& enemy, const lcf::rpg::EnemyAction& action) {
	switch (action.basic) {
		case lcf::rpg::EnemyAction::Basic_attack:
			return MakeAttack(enemy, 1);
		case lcf::rpg::EnemyAction::Basic_dual_attack:
			return MakeAttack(enemy, 2);
		case lcf::rpg::EnemyAction::Basic_defense:
			return std::make_shared<Game_BattleAlgorithm::Defend>(&enemy);
		case lcf::rpg::EnemyAction::Basic_observe:
			return std::make_shared<Game_BattleAlgorithm::Observe>(&enemy);
		case lcf::rpg::EnemyAction::Basic_charge:
			return std::make_shared<Game_BattleAlgorithm::Charge>(&enemy);
		case lcf::rpg::EnemyAction::Basic_autodestruction:
			return std::make_shared<Game_BattleAlgorithm::SelfDestruct>(&enemy, Main_Data::game_party.get());
		case lcf::rpg::EnemyAction::Basic_escape:
			return std::make_shared<Game_BattleAlgorithm::Escape>(&enemy);
		case lcf::rpg::EnemyAction::Basic_nothing:
			return std::make_shared<Game_BattleAlgorithm::DoNothing>(&enemy);
	}
	return nullptr;
}

static Game_Battler* GetRandomSkillTarget(Game_Party_Base& party, const lcf::rpg::Skill& skill, bool emulate_bugs) {
	std::vector<Game_Battler*> battlers;
	party.GetBattlers(battlers);
	for (auto iter = battlers.begin(); iter != battlers.end();) {
		if (IsSkillEffectiveOn(skill, **iter, emulate_bugs)) {
			++iter;
		} else {
			iter = battlers.erase(iter);
		}
	}
	auto choice = Rand::GetRandomNumber(0, battlers.size() - 1);
	return battlers[choice];
}

static std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> MakeSkillAction(Game_Enemy& enemy, const lcf::rpg::EnemyAction& action, bool emulate_bugs) {
	const auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, action.skill_id);
	if (!skill) {
		Output::Warning("EnemyAi::MakeSkillAction: Enemy can't use invalid skill {}", action.skill_id);
		return nullptr;
	}

	switch (skill->scope) {
		case lcf::rpg::Skill::Scope_enemy:
			return std::make_shared<Game_BattleAlgorithm::Skill>(&enemy, Main_Data::game_party->GetRandomActiveBattler(), *skill);
		case lcf::rpg::Skill::Scope_ally:
			return std::make_shared<Game_BattleAlgorithm::Skill>(&enemy, GetRandomSkillTarget(*Main_Data::game_enemyparty, *skill, emulate_bugs), *skill);
		case lcf::rpg::Skill::Scope_enemies:
			return std::make_shared<Game_BattleAlgorithm::Skill>(&enemy, Main_Data::game_party.get(), *skill);
		case lcf::rpg::Skill::Scope_self:
			return std::make_shared<Game_BattleAlgorithm::Skill>(&enemy, &enemy, *skill);
		case lcf::rpg::Skill::Scope_party:
			return std::make_shared<Game_BattleAlgorithm::Skill>(&enemy, Main_Data::game_enemyparty.get(), *skill);
	}
	return nullptr;
}

static std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> MakeAction(Game_Enemy& enemy, const lcf::rpg::EnemyAction& action, bool emulate_bugs) {
	switch (action.kind) {
		case lcf::rpg::EnemyAction::Kind_basic:
			return MakeBasicAction(enemy, action);
		case lcf::rpg::EnemyAction::Kind_skill:
			return MakeSkillAction(enemy, action, emulate_bugs);
		case lcf::rpg::EnemyAction::Kind_transformation:
			return std::make_shared<Game_BattleAlgorithm::Transform>(&enemy, action.enemy_id);
	}
	return nullptr;
}

void SetEnemyAction(Game_Enemy& enemy, const lcf::rpg::EnemyAction& action, bool emulate_bugs) {
	auto algo = MakeAction(enemy, action, emulate_bugs);

	if (algo) {
		if (action.switch_on) {
			algo->SetSwitchEnable(action.switch_on_id);
		}
		if (action.switch_off) {
			algo->SetSwitchDisable(action.switch_off_id);
		}
	}

	enemy.SetBattleAlgorithm(std::move(algo));
}


bool IsSkillEffectiveOn(const lcf::rpg::Skill& skill,
		const Game_Battler& target,
		bool emulate_bugs) {

	if (skill.type == lcf::rpg::Skill::Type_switch) {
		return true;
	}

	if (!Algo::IsNormalOrSubskill(skill)) {
		return false;
	}

	if (Algo::SkillTargetsEnemies(skill)) {
		return target.Exists();
	}

	if (!target.Exists()) {
		// RPG_RT Bug: Ignores reverse_state_effects_flag
		// RPG_RT Bug: Allows revival to target hidden enemies
		return (skill.state_effects.size() > 0 && skill.state_effects[0])
			&& (emulate_bugs || (!skill.reverse_state_effect && target.IsDead()));
	}

	if (skill.affect_hp
			|| skill.affect_sp
			|| skill.affect_attack
			|| skill.affect_defense
			|| skill.affect_spirit
			|| skill.affect_agility) {
		return true;
	}

	for (int id = 1; id <= static_cast<int>(skill.state_effects.size()); ++id) {
		// RPG_RT Bug: Ignores reverse_state_effects_flag
		if (skill.state_effects[id - 1] && (target.HasState(id) || (skill.reverse_state_effect && !emulate_bugs))) {
			return true;
		}
	}

	if (skill.affect_attr_defence) {
		for (auto& attr: skill.attribute_effects) {
			if (attr) {
				return true;
			}
		}
	}

	return false;
}

bool IsActionValid(const Game_Enemy& source, const lcf::rpg::EnemyAction& action) {
	if (action.kind == action.Kind_skill) {
		if (!source.IsSkillUsable(action.skill_id)) {
			return false;
		}
	}

	switch (action.condition_type) {
	case lcf::rpg::EnemyAction::ConditionType_always:
		return true;
	case lcf::rpg::EnemyAction::ConditionType_switch:
		return Main_Data::game_switches->Get(action.switch_id);
	case lcf::rpg::EnemyAction::ConditionType_turn:
		{
			int turns = Game_Battle::GetTurn();
			return Game_Battle::CheckTurns(turns, action.condition_param2, action.condition_param1);
		}
	case lcf::rpg::EnemyAction::ConditionType_actors:
		{
			std::vector<Game_Battler*> battlers;
			Main_Data::game_enemyparty->GetActiveBattlers(battlers);
			int count = (int)battlers.size();
			return count >= action.condition_param1 && count <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_hp:
		{
			int hp_percent = source.GetHp() * 100 / source.GetMaxHp();
			return hp_percent >= action.condition_param1 && hp_percent <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_sp:
		{
			int sp_percent = source.GetSp() * 100 / source.GetMaxSp();
			return sp_percent >= action.condition_param1 && sp_percent <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_party_lvl:
		{
			int party_lvl = Main_Data::game_party->GetAverageLevel();
			return party_lvl >= action.condition_param1 && party_lvl <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_party_fatigue:
		{
			int party_exh = Main_Data::game_party->GetFatigue();
			return party_exh >= action.condition_param1 && party_exh <= action.condition_param2;
		}
	default:
		return true;
	}
}

static bool IsSkillEffectiveOnAnyTarget(Game_Enemy& source, int skill_id, bool emulate_bugs) {
	const auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	assert(skill);
	if (!Algo::IsNormalOrSubskill(*skill)) {
		return true;
	}

	switch (skill->scope) {
		case lcf::rpg::Skill::Scope_enemy:
		case lcf::rpg::Skill::Scope_enemies:
			break;
		case lcf::rpg::Skill::Scope_self:
			return IsSkillEffectiveOn(*skill, source, emulate_bugs);
		case lcf::rpg::Skill::Scope_ally:
		case lcf::rpg::Skill::Scope_party:
			for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
				if (IsSkillEffectiveOn(*skill, *enemy, emulate_bugs)) {
					return true;
				}
			}
			return false;
	}

	return true;
}

void SelectEnemyAiActionRpgRtCompat(Game_Enemy& source, bool emulate_bugs) {
	const auto& actions = source.GetDbEnemy().actions;
	std::vector<int> prios(actions.size(), 0);
	int max_prio = 0;
	for (int i = 0; i < static_cast<int>(actions.size()); ++i) {
		const auto& action = actions[i];
		if (IsActionValid(source, action)) {
			prios[i] = action.rating;
			max_prio = std::max<int>(max_prio, action.rating);
			DebugLog("ENEMYAI: Enemy {}({}) Allow Action id={} kind={} basic={} rating={}", source.GetName(), source.GetTroopMemberId(), action.ID, action.kind, action.basic, action.rating);
		} else {
			DebugLog("ENEMYAI: Enemy {}({}) Discard Action id={} kind={} basic={} rating={}", source.GetName(), source.GetTroopMemberId(), action.ID, action.kind, action.basic, action.rating);
		}
	}

	DebugLog("ENEMYAI: Enemy {}({}) max_prio={}", source.GetName(), source.GetTroopMemberId(), max_prio);

	if (max_prio) {
		for (auto& pr: prios) {
			if (pr > 0) {
				pr = std::max<int>(0, pr - max_prio + 10);
			}
		}
	}

	for (int i = 0; i < static_cast<int>(actions.size()); ++i) {
		const auto& action = actions[i];
		if (action.kind == lcf::rpg::EnemyAction::Kind_skill) {
			if (prios[i] > 0 && !IsSkillEffectiveOnAnyTarget(source, action.skill_id, emulate_bugs)) {
				DebugLog("ENEMYAI: Enemy {}({}) Discard Action id={} kind={} basic={}, rating={}: No effective targets!", source.GetName(), source.GetTroopMemberId(), action.ID, action.kind, action.basic, action.rating);
				prios[i] = 0;
			}
		}
	}

	int sum_prios = 0;
	for (auto& pr: prios) {
		sum_prios += pr;
	}


	if (sum_prios == 0) {
		DebugLog("ENEMYAI: Enemy {}({}) No Available Actions!", source.GetName(), source.GetTroopMemberId());
		return;
	}

	int which = Rand::GetRandomNumber(0, sum_prios - 1);
	DebugLog("ENEMYAI: Enemy {}({}) sum_prios={} which={}", source.GetName(), source.GetTroopMemberId(), sum_prios, which);
	const lcf::rpg::EnemyAction* selected_action = nullptr;
	for (int i = 0; i < static_cast<int>(actions.size()); ++i) {
		auto& action = actions[i];
		selected_action = &action;
		which -= prios[i];
		if (which < 0) {
			break;
		}
	}

	if (selected_action) {
		DebugLog("ENEMYAI: Enemy {}({}) Selected Action id={} kind={} basic={}, rating={}", source.GetName(), source.GetTroopMemberId(), selected_action->ID, selected_action->kind, selected_action->basic, selected_action->rating);
		SetEnemyAction(source, *selected_action, emulate_bugs);
	} else {
		DebugLog("ENEMYAI: Enemy {}({}) No Selected Action!", source.GetName(), source.GetTroopMemberId());
	}
}

bool SetStateRestrictedAction(Game_Enemy& source) {
	if (!source.CanAct()) {
		source.SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::None>(&source));
		return true;
	}

	if (source.GetSignificantRestriction() == lcf::rpg::State::Restriction_attack_ally) {
		source.SetBattleAlgorithm(MakeAttackAllies(source, 1));
		return true;
	}

	if (source.GetSignificantRestriction() == lcf::rpg::State::Restriction_attack_enemy) {
		source.SetBattleAlgorithm(MakeAttack(source, 1));
		return true;
	}

	if (source.IsCharged()) {
		source.SetBattleAlgorithm(MakeAttack(source, 1));
		return true;
	}

	return false;
}

} // namespace EnemyAi
