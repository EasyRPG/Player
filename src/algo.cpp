#include "algo.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "attribute.h"
#include "player.h"
#include <lcf/rpg/skill.h>

#include <algorithm>

namespace Algo {

bool IsRowAdjusted(lcf::rpg::SaveActor::RowType row, lcf::rpg::System::BattleCondition cond, bool offense) {
	return (cond == lcf::rpg::System::BattleCondition_surround
			|| (row == (1 - offense)
				&& (cond == lcf::rpg::System::BattleCondition_none || cond == lcf::rpg::System::BattleCondition_initiative))
			|| (row == offense
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
	if (source.GetType() == Game_Battler::Type_Ally
		&& static_cast<const Game_Actor&>(source).AttackIgnoresEvasion(weapon)) {
		return to_hit;
	}

	// AGI adjustment.
	to_hit = CalcToHitAgiAdjustment(to_hit, source, target, weapon);

	// If target has physical dodge evasion:
	if (target.GetType() == Game_Battler::Type_Ally
			&& static_cast<const Game_Actor&>(target).HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	// Defender row adjustment
	if (Player::IsRPG2k3() && IsRowAdjusted(target, cond, false, emulate_2k3_enemy_row_bug)) {
		to_hit -= 25;
	}

	return to_hit;
}


int CalcSkillToHit(const Game_Battler& source, const Game_Battler& target, const lcf::rpg::Skill& skill) {
	auto to_hit = skill.hit;

	if (skill.failure_message != 3
		   || (skill.scope != lcf::rpg::Skill::Scope_enemy && skill.scope != lcf::rpg::Skill::Scope_enemies)) {
		return to_hit;
	}

	// RPG_RT BUG: rm2k3 editor doesn't let you set the failure message for skills, and so you can't make them physical type anymore.
	// Despite that, RPG_RT still checks the flag and run the below code?
	// FIXME: Verify if skills ported from 2k retain this flag and exercise the evasion logic in 2k3?
	// RPG_RT BUG: RPG_RT does not check for "EvadesAllPhysicaAttacks() states here

	// If target has Restriction "do_nothing", the attack always hits
	if (!target.CanAct()) {
		return 100;
	}

	// Modify hit chance for each state the source has
	to_hit = (to_hit * source.GetHitChanceModifierFromStates()) / 100;

	// Stop here if attacker ignores evasion.
	if (source.GetType() == Game_Battler::Type_Ally
		&& static_cast<const Game_Actor&>(source).AttackIgnoresEvasion(Game_Battler::WeaponAll)) {
		return to_hit;
	}

	// AGI adjustment.
	to_hit = CalcToHitAgiAdjustment(to_hit, source, target, Game_Battler::WeaponAll);

	// If target has physical dodge evasion:
	if (target.GetType() == Game_Battler::Type_Ally
			&& static_cast<const Game_Actor&>(target).HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	return to_hit;
}

int CalcCriticalHitChance(const Game_Battler& source, const Game_Battler& target, Game_Battler::Weapon weapon) {
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
	// FIXME: RPG_RT 2k3 doesn't apply variance if negative attribute flips damage
	if (var > 0 && (base > 0 || Player::IsLegacy())) {
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
		Game_Battler::Weapon weapon,
		bool is_critical_hit,
		bool apply_variance,
		lcf::rpg::System::BattleCondition cond,
		bool emulate_2k3_enemy_row_bug)
{
	const auto atk = source.GetAtk(weapon);
	const auto def = target.GetDef();

	// Base damage
	auto dmg = std::max(0, atk / 2 - def / 4);

	// Attacker row adjustment
	if (Player::IsRPG2k3() && IsRowAdjusted(source, cond, true, false)) {
		dmg = 125 * dmg / 100;
	}

	// Attacker weapon attribute adjustment
	dmg = Attribute::ApplyAttributeNormalAttackMultiplier(dmg, source, target, weapon);

	// Defender row adjustment
	if (Player::IsRPG2k3() && IsRowAdjusted(target, cond, false, emulate_2k3_enemy_row_bug)) {
		dmg = 75 * dmg / 100;
	}

	// Critical and charge adjustment
	if (is_critical_hit) {
		dmg *= 3;
	} else if (source.IsCharged()) {
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
		bool apply_variance) {

	auto effect = skill.power;
	effect += skill.physical_rate * source.GetAtk() / 20;
	effect += skill.magical_rate * source.GetSpi() / 40;

	if ((skill.scope == lcf::rpg::Skill::Scope_enemy
			|| skill.scope == lcf::rpg::Skill::Scope_enemies)
			&& !skill.ignore_defense) {
		effect -= skill.physical_rate * target.GetDef() / 40;
		effect -= skill.magical_rate * target.GetSpi() / 80;
	}

	effect = std::max(0, effect);

	effect = Attribute::ApplyAttributeSkillMultiplier(effect, target, skill);

	if (apply_variance) {
		effect = VarianceAdjustEffect(effect, skill.variance);
	}

	return effect;
}

int CalcSelfDestructEffect(const Game_Battler& source,
		const Game_Battler& target,
		bool apply_variance) {

	auto effect = source.GetAtk() - target.GetDef() / 2;
	effect = std::max(0, effect);

	if (apply_variance) {
		effect = VarianceAdjustEffect(effect, 4);
	}

	return effect;
}

} // namespace Algo
