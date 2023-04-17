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
#include "algo.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "game_system.h"
#include "main_data.h"
#include "game_player.h"
#include "game_targets.h"
#include "game_battle.h"
#include "attribute.h"
#include "player.h"
#include "rand.h"
#include <lcf/rpg/skill.h>
#include <lcf/reader_util.h>
#include "feature.h"

#include <algorithm>

namespace Algo {

bool IsRowAdjusted(lcf::rpg::SaveActor::RowType row, lcf::rpg::System::BattleCondition cond, bool offense) {
	return (cond == lcf::rpg::System::BattleCondition_surround
			|| (row != static_cast<int>(offense)
				&& (cond == lcf::rpg::System::BattleCondition_none || cond == lcf::rpg::System::BattleCondition_initiative))
			|| (row == static_cast<int>(offense)
				&& (cond == lcf::rpg::System::BattleCondition_back))
		   );
}

bool IsRowAdjusted(const Game_Battler& battler, lcf::rpg::System::BattleCondition cond, bool offense, bool allow_enemy) {
	lcf::rpg::SaveActor::RowType row = lcf::rpg::SaveActor::RowType_front;
	if (battler.GetType() == Game_Battler::Type_Ally) {
		row = static_cast<const Game_Actor&>(battler).GetBattleRow();
	}

	if (battler.GetType() == Game_Battler::Type_Ally || allow_enemy) {
		return IsRowAdjusted(row, cond, offense);
	}
	return false;
}

static int CalcToHitAgiAdjustment(int to_hit, const Game_Battler& source, const Game_Battler& target, Game_Battler::Weapon weapon) {
	// NOTE: RPG_RT 2k3 has a bug where if the source is an actor with a selected weapon, the agi
	// calculation calls a bespoke function which doesn't consider states which can cause half or double AGI.
	const float src_agi = std::max(1, source.GetAgi(weapon));
	const float tgt_agi = target.GetAgi();

	return 100 - (100 - to_hit) * (1.0f + (tgt_agi / src_agi - 1.0f) / 2.0f);
}

int CalcNormalAttackToHit(const Game_Battler &source,
		const Game_Battler &target,
		Game_Battler::Weapon weapon,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug) {
	auto to_hit = source.GetHitChance(weapon);

	// If target has rm2k3 state which grants 100% dodge.
	if (target.EvadesAllPhysicalAttacks()) {
		return 0;
	}

	// If target has Restriction "do_nothing", the attack always hits
	if (!target.CanAct()) {
		return 100;
	}

	// Modify hit chance for each state the source has
	to_hit = (to_hit * source.GetHitChanceModifierFromStates()) / 100;

	// Stop here if attacker ignores evasion.
	if (source.AttackIgnoresEvasion(weapon)) {
		return to_hit;
	}

	// AGI adjustment.
	to_hit = CalcToHitAgiAdjustment(to_hit, source, target, weapon);

	// If target has physical dodge evasion:
	if (target.HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	// Defender row adjustment
	if (Feature::HasRow() && IsRowAdjusted(target, cond, false, emulate_2k3_enemy_row_bug)) {
		to_hit -= 25;
	}

	return to_hit;
}


int CalcSkillToHit(const Game_Battler& source, const Game_Battler& target, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool emulate_2k3_enemy_row_bug) {
	auto to_hit = (skill.hit == -1 ? source.GetHitChance(Game_Battler::WeaponAll) : skill.hit);

	if (!SkillTargetsAllies(skill) && skill.easyrpg_affected_by_evade_all_physical_attacks && target.EvadesAllPhysicalAttacks()) {
		return 0;
	}

	// RPG_RT BUG: rm2k3 editor doesn't let you set the failure message for skills, and so you can't make them physical type anymore.
	// Despite that, RPG_RT still checks the flag and run the below code?
	if (skill.failure_message != 3 || SkillTargetsAllies(skill)) {
		return to_hit;
	}

	// RPG_RT BUG: RPG_RT 2k3 does not check for "EvadesAllPhysicaAttacks() states here

	// If target has Restriction "do_nothing", the attack always hits
	if (!target.CanAct()) {
		return 100;
	}

	// Modify hit chance for each state the source has
	to_hit = (to_hit * source.GetHitChanceModifierFromStates()) / 100;

	// Stop here if attacker ignores evasion.
	if (source.AttackIgnoresEvasion(Game_Battler::WeaponAll)) {
		return to_hit;
	}

	// AGI adjustment.
	to_hit = CalcToHitAgiAdjustment(to_hit, source, target, Game_Battler::WeaponAll);

	// If target has physical dodge evasion:
	if (target.HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	// Defender row adjustment
	if (Feature::HasRow() && skill.easyrpg_affected_by_row_modifiers && IsRowAdjusted(target, cond, false, emulate_2k3_enemy_row_bug)) {
		to_hit -= 25;
	}

	return to_hit;
}

int CalcCriticalHitChance(const Game_Battler& source, const Game_Battler& target, Game_Battler::Weapon weapon, int fixed_chance) {
	auto crit_chance = (fixed_chance < 0 ? static_cast<int>(source.GetCriticalHitChance(weapon) * 100.0) : fixed_chance);
	if (target.PreventsCritical()) {
		crit_chance = 0;
	}
	if (source.GetType() == target.GetType()) {
		crit_chance = 0;
	}
	return crit_chance;
}

int VarianceAdjustEffect(int base, int var) {
	// FIXME: RPG_RT 2k3 doesn't apply variance if negative attribute flips damage
	if (var > 0 && (base > 0 || Player::IsLegacy())) {
		int adj = std::max(1, var * base / 10);
		return base + Rand::GetRandomNumber(0, adj) - adj / 2;
	}
	return base;
}

int AdjustDamageForDefend(int dmg, const Game_Battler& target) {
	if (target.IsDefending()) {
		dmg /= 2;
		if (target.HasStrongDefense()) {
			dmg /= 2;
		}
	}
	return dmg;
}

int CalcNormalAttackEffect(const Game_Battler& source,
		const Game_Battler& target,
		Game_Battler::Weapon weapon,
		bool is_critical_hit,
		bool is_charged,
		bool apply_variance,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug)
{
	const auto atk = source.GetAtk(weapon);
	const auto def = target.GetDef();

	// Base damage
	auto dmg = std::max(0, atk / 2 - def / 4);

	// Attacker row adjustment
	if (Feature::HasRow() && IsRowAdjusted(source, cond, true, false)) {
		dmg = 125 * dmg / 100;
	}

	// Attacker weapon attribute adjustment
	dmg = Attribute::ApplyAttributeNormalAttackMultiplier(dmg, source, target, weapon);

	// Defender row adjustment
	if (Feature::HasRow() && IsRowAdjusted(target, cond, false, emulate_2k3_enemy_row_bug)) {
		dmg = 75 * dmg / 100;
	}

	// Critical and charge adjustment
	if (is_critical_hit) {
		dmg *= 3;
	} else if (is_charged) {
		dmg *= 2;
	}

	if (apply_variance) {
		dmg = VarianceAdjustEffect(dmg, 4);
	}

	return dmg;
}

int CalcSkillEffect(const Game_Battler& source,
		const Game_Battler& target,
		const lcf::rpg::Skill& skill,
		bool apply_variance,
		bool is_critical_hit,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug) {

	auto effect = skill.power;
	effect += skill.physical_rate * source.GetAtk() / 20;
	effect += skill.magical_rate * source.GetSpi() / 40;

	if (SkillTargetsEnemies(skill) && !skill.ignore_defense) {
		effect -= skill.physical_rate * target.GetDef() / 40;
		effect -= skill.magical_rate * target.GetSpi() / 80;
	}

	effect = std::max<int>(0, effect);

	if (Feature::HasRow() && skill.easyrpg_affected_by_row_modifiers && IsRowAdjusted(source, cond, true, false)) {
		effect = 125 * effect / 100;
	}

	effect = Attribute::ApplyAttributeSkillMultiplier(effect, target, skill);

	if (Feature::HasRow() && skill.easyrpg_affected_by_row_modifiers && IsRowAdjusted(target, cond, false, emulate_2k3_enemy_row_bug)) {
		effect = 75 * effect / 100;
	}

	if (is_critical_hit) {
		effect *= 3;
	}

	if (apply_variance) {
		effect = VarianceAdjustEffect(effect, skill.variance);
	}

	return effect;
}

int CalcSelfDestructEffect(const Game_Battler& source,
		const Game_Battler& target,
		bool apply_variance) {

	auto effect = source.GetAtk() - target.GetDef() / 2;
	effect = std::max<int>(0, effect);

	if (apply_variance) {
		effect = VarianceAdjustEffect(effect, 4);
	}

	return effect;
}

int CalcSkillCost(const lcf::rpg::Skill& skill, int max_sp, bool half_sp_cost) {
	const auto div = half_sp_cost ? 2 : 1;
	return (Player::IsRPG2k3() && skill.sp_type == lcf::rpg::Skill::SpType_percent)
		? max_sp * skill.sp_percent / 100 / div
		: (skill.sp_cost + static_cast<int>(half_sp_cost)) / div;
}

int CalcSkillHpCost(const lcf::rpg::Skill& skill, int max_hp) {
	return (Player::IsRPG2k3() && skill.easyrpg_hp_type == lcf::rpg::Skill::HpType_percent)
		? std::min<int>(max_hp - 1, max_hp * skill.easyrpg_hp_percent / 100)
		: skill.easyrpg_hp_cost;
}

bool IsSkillUsable(const lcf::rpg::Skill& skill,
		bool require_states_persist)
{
	const auto in_battle = Game_Battle::IsBattleRunning();

	if (skill.type == lcf::rpg::Skill::Type_escape) {
		return !in_battle && Main_Data::game_system->GetAllowEscape() && Main_Data::game_targets->HasEscapeTarget() && !Main_Data::game_player->IsFlying();
	}

	if (skill.type == lcf::rpg::Skill::Type_teleport) {
		return !in_battle && Main_Data::game_system->GetAllowTeleport() && Main_Data::game_targets->HasTeleportTargets() && !Main_Data::game_player->IsFlying();
	}

	if (skill.type == lcf::rpg::Skill::Type_switch) {
		return in_battle ? skill.occasion_battle : skill.occasion_field;
	}

	if (in_battle) {
		return true;
	}

	if (SkillTargetsEnemies(skill)) {
		return false;
	}

	if (skill.affect_hp || skill.affect_sp) {
		return true;
	}

	bool affects_state = false;
	for (int i = 0; i < static_cast<int>(skill.state_effects.size()); ++i) {
		const bool inflict = skill.state_effects[i];
		if (inflict) {
			const auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, i + 1);
			if (state && (!require_states_persist || state->type == lcf::rpg::State::Persistence_persists)) {
				affects_state = true;
				break;
			}
		}
	}

	return affects_state;
}

int GetNumberOfAttacks(int actor_id, const lcf::rpg::Item& weapon) {
	assert(weapon.type == lcf::rpg::Item::Type_weapon);
	int hits = weapon.dual_attack ? 2 : 1;
	if (Player::IsRPG2k3()) {
		auto& cba = weapon.animation_data;
		if (actor_id >= 1 && actor_id <= static_cast<int>(cba.size())) {
			int cba_hits = cba[actor_id - 1].attacks + 1;
			hits *= cba_hits;
		}
	}
	return hits;
}

} // namespace Algo
