/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GAME_BATTLER_H_
#define _GAME_BATTLER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rpg_state.h"
#include "system.h"

class Game_Actor;

////////////////////////////////////////////////////////////
/// Game_Battler class
////////////////////////////////////////////////////////////
class Game_Battler {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	////////////////////////////////////////////////////////
	// Game_Battler();

	////////////////////////////////////////////////////////
	/// Get if battler has a state.
	/// @param state_id : database state id
	/// @return whether the battler has the state
	////////////////////////////////////////////////////////
	bool HasState(int state_id) const;

	/// @return Vector containing the ids of all states the battler has
	virtual const std::vector<int16_t>& GetStates() const = 0;
	virtual std::vector<int16_t>& GetStates() = 0;

	/// @return The highest priority state affecting the battler
	///  returns NULL if no states
	const RPG::State* GetState();

	/// @return current hp
	virtual int GetHp() const = 0;

	/// Sets the current hp
	virtual void SetHp(int hp) = 0;

	/// @return current maxhp
	virtual int GetMaxHp() const;

	/// @return current sp
	virtual int GetSp() const = 0;

	/// Sets the current sp
	virtual void SetSp(int _sp) = 0;

	/// @return current maxsp
	virtual int GetMaxSp() const;

	/// @return current atk
	virtual int GetAtk() const;

	/// @return current def
	virtual int GetDef() const;

	/// @return current spi
	virtual int GetSpi() const;

	/// @return current agi
	virtual int GetAgi() const;

	////////////////////////////////////////////////////////
	/// Gets the maximum hp for the current level
	/// @return max hp
	////////////////////////////////////////////////////////
	virtual int GetBaseMaxHp() const = 0;

	////////////////////////////////////////////////////////
	/// Gets the maximum sp for the current level
	/// @return max sp
	////////////////////////////////////////////////////////
	virtual int GetBaseMaxSp() const= 0;

	/// @return atk
	virtual int GetBaseAtk() const = 0;

	/// @return def
	virtual int GetBaseDef() const = 0;

	/// @return spi
	virtual int GetBaseSpi() const = 0;

	/// @return agi
	virtual int GetBaseAgi() const = 0;

	virtual bool IsHidden() const;
	virtual bool IsImmortal() const;

	bool Exists() const;
	bool IsDead() const;

	////////////////////////////////////////////////////////
	/// Checks if the actor can use the skill
	/// @param skill_id : id of skill to check
	/// @return true if skill can be used
	////////////////////////////////////////////////////////
	virtual bool IsSkillUsable(int skill_id);

	////////////////////////////////////////////////////////
	/// Calculates the Skill costs including all modifiers.
	/// @param skill_id : id of skill to calculate
	/// @return needed skill cost
	////////////////////////////////////////////////////////
	int CalculateSkillCost(int skill_id);

	////////////////////////////////////////////////////////
	/// Adds a State
	/// @param state_id : id of state to add
	////////////////////////////////////////////////////////
	void AddState(int state_id);

	////////////////////////////////////////////////////////
	/// Removes a State
	/// @param state_id : id of state to remove
	////////////////////////////////////////////////////////
	void RemoveState(int state_id);

	////////////////////////////////////////////////////////
	/// Removes all States which end after battle
	////////////////////////////////////////////////////////
	void RemoveStates();

	////////////////////////////////////////////////////////
	/// Removes all States
	////////////////////////////////////////////////////////
	void RemoveAllStates();
};

#endif
