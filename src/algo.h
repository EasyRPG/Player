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

#ifndef EP_ALGO_H
#define EP_ALGO_H

#include <lcf/rpg/fwd.h>
#include <lcf/rpg/system.h>
#include <lcf/rpg/saveactor.h>
#include <lcf/rpg/skill.h>
#include "game_battler.h"

class Game_Actor;
class Game_Enemy;

namespace Algo {

/**
 * Compute whether a row adjustment should occur.
 * 
 * @param row The row value to check
 * @param cond The current battle condition
 * @param offense Whether to adjust for an offensive action (true) or defensive action (false)
 *
 * @return Whether the row adjustment should apply or not.
 */
bool IsRowAdjusted(lcf::rpg::SaveActor::RowType row, lcf::rpg::System::BattleCondition cond, bool offense);

/**
 * Compute whether a row adjustment should occur.
 * 
 * @param actor The actor whose row to check
 * @param cond The current battle condition
 * @param offense Whether to adjust for an offensive action (true) or defensive action (false)
 * @param allow_enemy Compute row adjustment for enemies also and treat them as always in front row.
 *
 * @return Whether the row adjustment should apply or not.
 */
bool IsRowAdjusted(const Game_Battler& battler,
		lcf::rpg::System::BattleCondition cond,
		bool offense,
		bool allow_enemy);

/**
 * Uses RPG_RT algorithm for performing a variance adjument to damage/healing effects and returns the result.
 *
 * @param base - the base amount of the effect
 * @param var - the variance level from 0 to 10
 *
 * @return the adjusted damage amount
 */
int VarianceAdjustEffect(int base, int var);

/**
 * Compute the hit rate for a physical attack
 *
 * @param source The source of the action
 * @param target The target of the action
 * @param cond The current battle condition
 * @param weapon Which weapon to use or kWeaponAll for combined
 * @param emulate_2k3_enemy_row_bug Whether or not to emulate 2k3 bug where RPG_RT considers defending enemies in the front row
 *
 * @return Success hit rate
 */
int CalcNormalAttackToHit(const Game_Battler& source,
		const Game_Battler& target,
		Game_Battler::Weapon weapon,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug);

/**
 * Compute the hit rate for a skill
 *
 * @param source The source of the action
 * @param target The target of the action
 * @param skill Which skill to calculate hit rate for
 * @param cond The current battle condition
 * @param emulate_2k3_enemy_row_bug Whether or not to emulate 2k3 bug where RPG_RT considers defending enemies in the front row
 *
 * @return Success hit rate
 */
int CalcSkillToHit(const Game_Battler& source, const Game_Battler& target, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool emulate_2k3_enemy_row_bug);

/**
 * Compute the critical hit rate if source attacks target.
 *
 * @param source The attacker
 * @param target The defender
 * @param weapon Which weapon to use or kWeaponAll for combined
 * @param fixed_chance Fixed critical hit rate percentage
 *
 * @return Critical hit rate.
 */
int CalcCriticalHitChance(const Game_Battler& source, const Game_Battler& target, Game_Battler::Weapon weapon, int fixed_chance);

/**
 * Check if target is defending and perform damage adjustment if so.
 *
 * @param dmg The base amount of damage.
 * @param target who is to receive damage
 *
 * @return The adjusted damage.
 */
int AdjustDamageForDefend(int dmg, const Game_Battler& target);

/**
 * Compute the base damage for a normal attack
 * This includes: source atk, target def, source row, attributes, target row, critical, source charged, and variance
 * It does not include: hit rate, critical hit rate, target defend adjustment, value clamping
 *
 * @param source The source of the action
 * @param target The target of the action
 * @param weapon Which weapon to use or kWeaponAll for combined
 * @param is_critical_hit If true, apply critical hit bonus
 * @param is_charged If true, the attacker was charged
 * @param apply_variance If true, apply variance to the damage
 * @param cond The current battle condition
 * @param emulate_2k3_enemy_row_bug Whether or not to emulate 2k3 bug where RPG_RT considers defending enemies in the front row
 *
 * @return effect amount
 */
int CalcNormalAttackEffect(const Game_Battler& source,
		const Game_Battler& target,
		Game_Battler::Weapon weapon,
		bool is_critical_hit,
		bool is_charged,
		bool apply_variance,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug);

/**
 * Compute the base damage for a skill
 * This includes: power, source atk/mag, target def/mag, attributes, variance
 * It does not include: hit rate, target defend adjustment, value clamping
 *
 * @param source The source of the action
 * @param target The target of the action
 * @param skill The skill to use
 * @param apply_variance If true, apply variance to the damage
 * @param is_critical_hit If true, apply critical hit bonus
 * @param cond The current battle condition
 * @param emulate_2k3_enemy_row_bug Whether or not to emulate 2k3 bug where RPG_RT considers defending enemies in the front row
 *
 * @return effect amount
 */
int CalcSkillEffect(const Game_Battler& source,
		const Game_Battler& target,
		const lcf::rpg::Skill& skill,
		bool apply_variance,
		bool is_critical_hit,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug);

/**
 * Compute the base damage for self-destruct
 * This includes: power, source atk, target def, variance
 * It does not include: hit rate, target defend adjustment, value clamping
 *
 * @param source The source of the action
 * @param target The target of the action
 * @param apply_variance If true, apply variance to the damage
 *
 * @return effect amount
 */
int CalcSelfDestructEffect(const Game_Battler& source,
		const Game_Battler& target,
		bool apply_variance);

/**
 * Calculate the sp cost for a skill.
 * This includes: power, source atk, target def, variance
 * It does not include: hit rate, target defend adjustment, value clamping
 *
 * @param skill The skill to compute
 * @param max_sp the max sp of the user
 * @param half_sp_cost if user has half_sp_cost modifier
 *
 * @return sp cost
 */
int CalcSkillCost(const lcf::rpg::Skill& skill, int max_sp, bool half_sp_cost);

/**
 * Calculates the hp cost for a skill.
 *
 * @param skill The skill to compute
 * @param max_sp the max hp of the user
 *
 * @return hp cost
 */
int CalcSkillHpCost(const lcf::rpg::Skill& skill, int max_hp);

/*
 * Determine whether a skill is usable.
 *
 * @param skill the skill to check
 * @param require_states_persist If we should require persistent states for non-battle.
 * @return Whether the skill can be used.
 */
bool IsSkillUsable(const lcf::rpg::Skill& skill,
		bool require_states_persist);

/**
 * Checks if the skill is a normal or subskill type.
 *
 * @param skill the skill to check
 * @return true if a normal skill or a 2k3 subskill.
 */
inline bool IsNormalOrSubskill(const lcf::rpg::Skill& skill) {
	return skill.type == lcf::rpg::Skill::Type_normal
		|| skill.type >= lcf::rpg::Skill::Type_subskill;
}

/**
 * Checks if the skill targets the opposing party.
 *
 * @param skill the skill to check
 * @return true if targets opposing party
 */
inline bool SkillTargetsEnemies(const lcf::rpg::Skill& skill) {
	return skill.scope == lcf::rpg::Skill::Scope_enemy
		|| skill.scope == lcf::rpg::Skill::Scope_enemies;
}

/**
 * Checks if the skill targets the allied party.
 *
 * @param skill the skill to check
 * @return true if targets allied party
 */
inline bool SkillTargetsAllies(const lcf::rpg::Skill& skill) {
	return !SkillTargetsEnemies(skill);
}

/**
 * Checks if the skill has a single target
 *
 * @param skill the skill to check
 * @return true if targets a single battler
 */
inline bool SkillTargetsOne(const lcf::rpg::Skill& skill) {
	return skill.scope == lcf::rpg::Skill::Scope_ally
				|| skill.scope == lcf::rpg::Skill::Scope_enemy
				|| skill.scope == lcf::rpg::Skill::Scope_self;
}

/**
 * Returns the number of attacks the weapon can do.
 *
 * @param the id of the actor to check
 * @param weapon the item to check
 * @pre If weapon is not a weapon type, the result is undefined.
 * @return the number of attacks.
 */
int GetNumberOfAttacks(int actor_id, const lcf::rpg::Item& weapon);

} // namespace Algo


#endif
