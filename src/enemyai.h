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
#ifndef EP_ENENMYAI_H
#define EP_ENENMYAI_H

#include <lcf/rpg/fwd.h>
#include <lcf/rpg/system.h>
#include <lcf/rpg/saveactor.h>
#include <game_battler.h>
#include <memory>

class Game_Actor;
class Game_Enemy;

namespace EnemyAi {
class AlgorithmBase;

/**
 * Enemy AI algorithm factory function which creates enemy ai algorithm from the given name
 *
 * @param name of the algo.
 * @return An auto battle algorithm to be used in battles.
 */
std::unique_ptr<AlgorithmBase> CreateAlgorithm(StringView name);

/**
 * Base class for enemy ai algorithm implementations.
 */
class AlgorithmBase {
public:
	virtual ~AlgorithmBase() {}

	/** @return the id of this algorithm */
	virtual int GetId() const = 0;

	/** @return the name of this algorithm */
	virtual StringView GetName() const = 0;

	/**
	 * Calculates the enemy ai algorithm and sets the algorithm on source.
	 * This computation ignores states on the actor, it is  the resposibility of the caller
	 * to handle death, confuse, provoke, etc..
	 *
	 * @param source The source actor to set the action for.
	 * @post source will have a BattleAlgorithm set.
	 */
	void SetEnemyAiAction(Game_Enemy& source);
private:
	virtual void vSetEnemyAiAction(Game_Enemy& source) = 0;
};

/**
 * The default autobattle algorithm which is strictly compatible with RPG_RT.
 */
class RpgRtCompat: public AlgorithmBase {
public:
	static constexpr auto id = 0;

	static constexpr auto name = "RPG_RT";

	int GetId() const override { return id; }

	StringView GetName() const override { return name; }
private:
	void vSetEnemyAiAction(Game_Enemy& source) override;
};

/**
 * The default autobattle algorithm which is strictly compatible with RPG_RT.
 */
class RpgRtImproved: public AlgorithmBase {
public:
	static constexpr auto id = 1;

	static constexpr auto name = "RPG_RT+";

	int GetId() const override { return id; }

	StringView GetName() const override { return name; }
private:
	void vSetEnemyAiAction(Game_Enemy& source) override;
};

/**
 * Runs the RPG_RT algorithm for selecting a battle action for an enemy.
 *
 * @param source the enemy who will take the action
 * @param emulate_bugs if true, emulate RPG_RT bugs
 * @post If an action was selected, the source will have a new battle algorithm attached.
 */
void SelectEnemyAiActionRpgRtCompat(Game_Enemy& source, bool emulate_bugs);

/**
 * Checks if the skill will be effective on a target.
 *
 * @param skill the skill to check
 * @param emulate_bugs if true, emulate RPG_RT bugs
 * @return true if a normal skill or a 2k3 subskill.
 */
bool IsSkillEffectiveOn(const lcf::rpg::Skill& skill,
		const Game_Battler& target,
		bool emulate_bugs);

/**
 * Sets a battle action on the enemy if they have a state restriction that forces a particular action.
 *
 * @param source The enemy to check
 * @return true if an action was set
 */
bool SetStateRestrictedAction(Game_Enemy& source);

} // namespace AutoBattle

#endif
