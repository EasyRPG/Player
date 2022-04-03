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
#ifndef EP_AUTOBATTLE_H
#define EP_AUTOBATTLE_H

#include <lcf/rpg/fwd.h>
#include <lcf/rpg/system.h>
#include <lcf/rpg/saveactor.h>
#include <game_battler.h>
#include <memory>

class Game_Actor;
class Game_Enemy;

namespace AutoBattle {
class AlgorithmBase;

/**
 * Auto battle algorithm factory function which creates auto battle algorithm from the given name
 *
 * @param name of the algo.
 * @return An auto battle algorithm to be used in battles.
 */
std::unique_ptr<AlgorithmBase> CreateAlgorithm(StringView name);

/**
 * Base class for auto battle algorithm implementations.
 */
class AlgorithmBase {
public:
	virtual ~AlgorithmBase() {}

	/** @return the id of this algorithm */
	virtual int GetId() const = 0;

	/** @return the name of this algorithm */
	virtual StringView GetName() const = 0;

	/**
	 * Calculates the auto battle algorithm and sets the algorithm on source.
	 * This computation ignores states on the actor, it is  the resposibility of the caller
	 * to handle death, confuse, provoke, etc..
	 *
	 * @param source The source actor to set the action for.
	 * @post source will have a BattleAlgorithm set.
	 */
	void SetAutoBattleAction(Game_Actor& source);
private:
	virtual void vSetAutoBattleAction(Game_Actor& source) = 0;
};

/**
 * The default autobattle algorithm which is strictly compatible with RPG_RT, bugs included.
 */
class RpgRtCompat: public AlgorithmBase {
public:
	static constexpr auto id = 0;

	static constexpr auto name = "RPG_RT";

	int GetId() const override { return id; }

	StringView GetName() const override { return name; }
private:
	void vSetAutoBattleAction(Game_Actor& source) override;
};

/**
 * An autobattle algorithm which only does physical attacks.
 */
class AttackOnly: public AlgorithmBase {
public:
	static constexpr auto id = 2;

	static constexpr auto name = "ATTACK";

	int GetId() const override { return id; }

	StringView GetName() const override { return name; }
private:
	void vSetAutoBattleAction(Game_Actor& source) override;
};

/**
 * A custom autobattle algorithm which is similar to RPG_RT but fixes bugs and has improved logic.
 */
class RpgRtImproved: public AlgorithmBase {
public:
	static constexpr auto id = 1;

	static constexpr auto name = "RPG_RT+";

	int GetId() const override { return id; }

	StringView GetName() const override { return name; }
private:
	void vSetAutoBattleAction(Game_Actor& source) override;
};

/**
 * Calculate the auto battle effectiveness rank of source using healing skill on target.
 *
 * @param source the user of the skill
 * @param target the target of the skill
 * @param skill the skill
 * @param cond the battle condition
 * @param apply_variance If true, apply variance to the damage
 * @param emulate_bugs Emulate all RPG_RT bugs for accuracy
 *
 * @pre skill Must be a normal or subskill or the result is undefined.
 * @pre skill must target self, ally, or ally party or the result is undefined.
 */
double CalcSkillHealAutoBattleTargetRank(const Game_Actor& source, const Game_Battler& target, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs);

/**
 * Calculate the auto battle effectiveness rank of source using damage skill on target.
 *
 * @param source the user of the skill
 * @param target the target of the skill
 * @param skill the skill
 * @param cond the battle condition
 * @param apply_variance If true, apply variance to the damage
 * @param emulate_bugs Emulate all RPG_RT bugs for accuracy
 *
 * @pre skill Must be a normal or subskill or the result is undefined.
 * @pre skill must target enemy, or enemy party or the result is undefined.
 */
double CalcSkillDmgAutoBattleTargetRank(const Game_Actor& source, const Game_Battler& target, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs);

/**
 * Calculate the auto battle total effectiveness rank of using a skill.
 *
 * @param source the user of the skill
 * @param skill the skill
 * @param cond the battle condition
 * @param apply_variance If true, apply variance to the damage
 * @param emulate_bugs Emulate all RPG_RT bugs for accuracy
 */
double CalcSkillAutoBattleRank(const Game_Actor& source, const lcf::rpg::Skill& skill, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs);

/**
 * Calculate the auto battle effectiveness rank of source attacking target.
 *
 * @param source the user of the skill
 * @param target the target of the skill
 * @param cond the battle condition
 * @param apply_variance If true, apply variance to the damage
 * @param emulate_bugs Emulate all RPG_RT bugs for accuracy
 */
double CalcNormalAttackAutoBattleTargetRank(const Game_Actor& source, const Game_Battler& target, Game_Battler::Weapon weapon, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs);

/**
 * Calculate the auto battle total effectiveness rank of using a normal attack.
 *
 * @param source the user of the skill
 * @param weapon Which weapon to use in the calculation
 * @param cond the battle condition
 * @param apply_variance If true, apply variance to the damage
 * @param emulate_bugs Emulate all RPG_RT bugs for accuracy
 */
double CalcNormalAttackAutoBattleRank(const Game_Actor& source, Game_Battler::Weapon weapon, lcf::rpg::System::BattleCondition cond, bool apply_variance, bool emulate_bugs);

/**
 * Runs the RPG_RT auto battle algorithm and sets a Game_BattlerAlgorithm on the source.
 *
 * @param actor Which actor to select auto battle action for
 * @param weapon Which weapon to use in the calculation
 * @param cond the battle condition
 * @param do_skills Whether to include skills or not
 * @param attack_variance Whether to include variance in normal attack ranking
 * @param skill_variance Whether to include variance in skill ranking
 * @param emulate_bugs Emulate all RPG_RT bugs for accuracy
 * @post actor may have a battle algorithm set, unless an error occured.
 */
void SelectAutoBattleAction(Game_Actor& source,
		Game_Battler::Weapon weapon,
		lcf::rpg::System::BattleCondition cond,
		bool do_skills,
		bool attack_variance,
		bool skill_variance,
		bool emulate_bugs);

/**
 * Calls SelectAutoBattleAction() with RPG_RT strictly compatible flags.
 *
 * @param actor Which actor to select auto battle action for
 * @param cond the battle condition
 * @post actor may have a battle algorithm set, unless an error occured.
 */
inline void SelectAutoBattleActionRpgRtCompat(Game_Actor& source, lcf::rpg::System::BattleCondition cond) {
	SelectAutoBattleAction(source, Game_Battler::WeaponAll, cond, true, false, true, true);
}

} // namespace AutoBattle

#endif
