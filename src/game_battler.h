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

#ifndef _GAME_BATTLER_H_
#define _GAME_BATTLER_H_

// Headers
#include <string>
#include <vector>
#include "rpg_state.h"
#include "system.h"

class Game_Actor;

/**
 * Game_Battler class.
 */
class Game_Battler {
public:
	/**
	 * Constructor.
	 */
	// Game_Battler();

	/**
	 * Gets if battler has a state.
	 *
	 * @param state_id database state ID.
	 * @return whether the battler has the state.
	 */
	bool HasState(int state_id) const;

	/**
	 * Gets battler states.
	 *
	 * @return vector containing the IDs of all states the battler has.
	 */
	virtual const std::vector<int16_t>& GetStates() const = 0;
	virtual std::vector<int16_t>& GetStates() = 0;

	/**
	 * Gets current battler state with highest priority.
	 *
	 * @return the highest priority state affecting the battler.
	 *         Returns NULL if no states.
	 */
	const RPG::State* GetSignificantState();

	/**
	 * Gets battler HP.
	 *
	 * @return current HP.
	 */
	virtual int GetHp() const = 0;

	/**
	 * Sets the current battler HP.
	 */
	virtual void SetHp(int hp) = 0;

	/**
	 * Gets the battler max HP.
	 *
	 * @return current max HP.
	 */
	virtual int GetMaxHp() const;

	/**
	 * Gets battler SP.
	 *
	 * @return current SP.
	 */
	virtual int GetSp() const = 0;

	/**
	 * Sets the current battler SP.
	 */
	virtual void SetSp(int _sp) = 0;

	/**
	 * Gets the battler max SP.
	 *
	 * @return current max SP.
	 */
	virtual int GetMaxSp() const;

	/**
	 * Gets the current battler attack.
	 *
	 * @return current attack.
	 */
	virtual int GetAtk() const;

	/**
	 * Gets the current battler defense.
	 *
	 * @return current defense.
	 */
	virtual int GetDef() const;

	/**
	 * Gets the current battler spirit.
	 *
	 * @return current spirit.
	 */
	virtual int GetSpi() const;

	/**
	 * Gets the current battler agility.
	 *
	 * @return current agility.
	 */
	virtual int GetAgi() const;

	/**
	 * Gets the maximum HP for the current level.
	 *
	 * @return max HP.
	 */
	virtual int GetBaseMaxHp() const = 0;

	/**
	 * Gets the maximum SP for the current level.
	 *
	 * @return max SP.
	 */
	virtual int GetBaseMaxSp() const= 0;

	/**
	 * Gets the attack for the current level.
	 *
	 * @return attack.
	 */
	virtual int GetBaseAtk() const = 0;

	/**
	 * Gets the defense for the current level.
	 *
	 * @return defense.
	 */
	virtual int GetBaseDef() const = 0;

	/**
	 * Gets the spirit for the current level.
	 *
	 * @return spirit.
	 */
	virtual int GetBaseSpi() const = 0;

	/**
	 * Gets the agility for the current level.
	 *
	 * @return agility.
	 */
	virtual int GetBaseAgi() const = 0;

	virtual bool IsHidden() const;
	virtual bool IsImmortal() const;

	bool Exists() const;
	bool IsDead() const;

	/**
	 * Checks if the actor can use the skill.
	 *
	 * @param skill_id ID of skill to check.
	 * @return true if skill can be used.
	 */
	virtual bool IsSkillUsable(int skill_id) const;

	/**
	 * Calculates the Skill costs including all modifiers.
	 *
	 * @param skill_id ID of skill to calculate.
	 * @return needed skill cost.
	 */
	int CalculateSkillCost(int skill_id) const;

	/**
	 * Adds a State.
	 *
	 * @param state_id ID of state to add.
	 */
	void AddState(int state_id);

	/**
	 * Removes a State.
	 *
	 * @param state_id ID of state to remove.
	 */
	void RemoveState(int state_id);

	/**
	 * Removes all states which end after battle.
	 */
	void RemoveStates();

	/**
	 * Removes all states.
	 */
	void RemoveAllStates();
};

#endif
