#include "algo.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "attribute.h"
#include "player.h"
#include <lcf/rpg/skill.h>

#include <algorithm>

namespace Algo {

bool IsRowAdjusted(const Game_Actor& actor, lcf::rpg::System::BattleCondition cond, bool offense) {
	return (cond == lcf::rpg::System::BattleCondition_surround
			|| (actor.GetBattleRow() == (1 - offense)
				&& (cond == lcf::rpg::System::BattleCondition_none || cond == lcf::rpg::System::BattleCondition_initiative))
			|| (actor.GetBattleRow() == offense
				&& (cond == lcf::rpg::System::BattleCondition_back))
		   );
}

int CalcNormalAttackToHit(const Game_Battler &source, const Game_Battler &target, int weapon, lcf::rpg::System::BattleCondition cond) {
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
	if (source.GetType() == Game_Battler::Type_Ally
		&& static_cast<const Game_Actor&>(source).AttackIgnoresEvasion(weapon)) {
		return to_hit;
	}

	// AGI adjustment.
	// NOTE: RPG_RT 2k3 has a bug where if the source is an actor with a selected weapon, the agi
	// calculation calls a bespoke function which doesn't consider states which can cause half or double AGI.
	to_hit = 100 - (100 - to_hit) * (1.0f + (float(target.GetAgi()) / float(source.GetAgi(weapon)) - 1.0f) / 2.0f) ;

	// If target has physical dodge evasion:
	if (target.GetType() == Game_Battler::Type_Ally
			&& static_cast<const Game_Actor&>(target).HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	// Defender row adjustment
	if (Player::IsRPG2k3() && target.GetType() == Game_Battler::Type_Ally) {
		if (IsRowAdjusted(static_cast<const Game_Actor&>(target), cond, false)) {
			to_hit -= 25;
		} else if(source.GetType() == Game_Battler::Type_Ally
				&& target.GetType() == Game_Battler::Type_Enemy
				&& cond == lcf::rpg::System::BattleCondition_back) {
			// FIXME: RPG_RT always adjusts damage down for back attack, regardless of row - when to handle and not handle this?
			to_hit -= 25;
		}
	}

	return to_hit;
}


int CalcSkillToHit(const Game_Battler& source, const Game_Battler& target, const lcf::rpg::Skill& skill) {
	auto to_hit = skill.hit;

	if (skill.failure_message != 3
		   || (skill.scope != lcf::rpg::Skill::Scope_enemy && skill.scope != lcf::rpg::Skill::Scope_enemies)) {
		return to_hit;
	}

	// If target has Restriction "do_nothing", the attack always hits
	if (!target.CanAct()) {
		return 100;
	}

	// Modify hit chance for each state the source has
	to_hit = (to_hit * source.GetHitChanceModifierFromStates()) / 100;

	// Stop here if attacker ignores evasion.
	if (source.GetType() == Game_Battler::Type_Ally
		&& static_cast<const Game_Actor&>(source).AttackIgnoresEvasion(Game_Battler::kWeaponAll)) {
		return to_hit;
	}

	// AGI adjustment.
	to_hit = 100 - (100 - to_hit) * (1.0f + (float(target.GetAgi()) / float(source.GetAgi()) - 1.0f) / 2.0f) ;

	// If target has physical dodge evasion:
	if (target.GetType() == Game_Battler::Type_Ally
			&& static_cast<const Game_Actor&>(target).HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	return to_hit;
}

int CalcCriticalHitChance(const Game_Battler& source, const Game_Battler& target, int weapon) {
	// FIXME: Make this function return int 0 to 100.
	auto crit_chance = static_cast<int>(source.GetCriticalHitChance(weapon) * 100.0);
	if (target.GetType() == Game_Battler::Type_Ally && static_cast<const Game_Actor&>(target).PreventsCritical()) {
		crit_chance = 0;
	}
	if (source.GetType() == target.GetType()) {
		crit_chance = 0;
	}
	return crit_chance;
}

int VarianceAdjustEffect(int base, int var) {
	if (var > 0) {
		int adj = std::max(1, var * base / 10);
		return base + Utils::GetRandomNumber(0, adj) - adj / 2;
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
		int weapon,
		bool is_critical_hit,
		bool apply_variance,
		lcf::rpg::System::BattleCondition cond) {
	const auto atk = source.GetAtk(weapon);
	const auto def = target.GetDef();

	// Base damage
	auto dmg = std::max(0, atk / 2 - def / 4);

	// Attacker row adjustment
	if (Player::IsRPG2k3() && source.GetType() == Game_Battler::Type_Ally) {
		if (IsRowAdjusted(static_cast<const Game_Actor&>(source), cond, true)) {
			dmg = 125 * dmg / 100;
		}
	}

	// Attacker weapon attribute adjustment
	if (source.GetType() == Game_Battler::Type_Ally) {
		Attribute::ApplyAttributeNormalAttackMultiplier(dmg, static_cast<const Game_Actor&>(source), target, weapon);
	}

	// Defender row adjustment
	if (Player::IsRPG2k3()) {
		if (target.GetType() == Game_Battler::Type_Ally && IsRowAdjusted(static_cast<const Game_Actor&>(target), cond, false)) {
			dmg = 75 * dmg / 100;
		} else if(source.GetType() == Game_Battler::Type_Ally
				&& target.GetType() == Game_Battler::Type_Enemy
				&& cond == lcf::rpg::System::BattleCondition_back) {
			// FIXME: RPG_RT always adjusts damage down for back attack, regardless of row - when to handle and not handle this?
			dmg = 75 * dmg / 100;
		}
	}

	// Critical and charge adjustment
	if (is_critical_hit) {
		dmg *= 3;
	} else if (source.IsCharged()) {
		dmg *= 2;
	}

	// Variance Adjustment
	// FIXME: RPG_RT 2k3 doesn't apply variance if negative attribute flips damage
	if (apply_variance && (dmg > 0 || Player::IsLegacy())) {
		dmg = VarianceAdjustEffect(dmg, 4);
	}

	return dmg;
}

int CalcSkillEffect(const Game_Battler& source,
		const Game_Battler& target,
		const lcf::rpg::Skill& skill,
		bool apply_variance) {

	auto effect = skill.power;
	effect += skill.physical_rate * source.GetAtk() / 20;
	effect += skill.magical_rate * source.GetSpi() / 40;

	if ((skill.scope == lcf::rpg::Skill::Scope_enemy
			|| skill.scope == lcf::rpg::Skill::Scope_enemies)
			&& !skill.ignore_defense) {
		effect -= skill.physical_rate * target.GetDef() / 40;
		effect -= skill.magical_rate * target.GetSpi() / 40;
	}

	effect = std::max(0, effect);

	effect = Attribute::ApplyAttributeSkillMultiplier(effect, target, skill);

	// FIXME: RPG_RT 2k3 doesn't apply variance if negative attribute flips damage
	if (apply_variance && (effect > 0 || Player::IsLegacy())) {
		effect = VarianceAdjustEffect(effect, skill.variance);
	}

	return effect;
}

int CalcSelfDestructEffect(const Game_Battler& source,
		const Game_Battler& target,
		bool apply_variance) {

	auto effect = source.GetAtk() - target.GetDef() / 2;
	effect = std::max(0, effect);

	// FIXME: RPG_RT 2k3 doesn't apply variance if negative attribute flips damage
	if (apply_variance && (effect > 0 || Player::IsLegacy())) {
		effect = VarianceAdjustEffect(effect, 4);
	}

	return effect;
}

} // namespace Algo
