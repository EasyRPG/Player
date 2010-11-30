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

#ifndef _GAME_ACTOR_H_
#define _GAME_ACTOR_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "game_battler.h"

////////////////////////////////////////////////////////////
/// Game_Actor class
////////////////////////////////////////////////////////////
class Game_Actor : public Game_Battler {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param actor_id : database actor id
	////////////////////////////////////////////////////////
	Game_Actor(int actor_id);

	////////////////////////////////////////////////////////
	/// Setups the game actor with the database actor.
	/// This is automatically called in the constructor.
	/// @param actor_id : database actor id
	////////////////////////////////////////////////////////
	void Setup(int actor_id);

	////////////////////////////////////////////////////////
	/// Get if skill_id has already learned a skill.
	/// @param skill_id : database skill id
	/// @return whether the actor has the skill
	////////////////////////////////////////////////////////
	bool HasSkill(int skill_id) const;

	////////////////////////////////////////////////////////
	/// Learn a new skill.
	/// @param skill_id : database skill id
	////////////////////////////////////////////////////////
	void LearnSkill(int skill_id);

	////////////////////////////////////////////////////////
	/// Gets the actor ID.
	/// @return Actor ID
	////////////////////////////////////////////////////////
	int GetActorId() const;

	////////////////////////////////////////////////////////
	/// Gets the maximum hp for the current level
	/// @return max hp
	////////////////////////////////////////////////////////
	int GetBaseMaxHp() const;

	////////////////////////////////////////////////////////
	/// Gets the maximum sp for the current level
	/// @return max sp
	////////////////////////////////////////////////////////
	int GetBaseMaxSp() const;

	////////////////////////////////////////////////////////
	/// Calculates the Exp needed for a level up
	/// @param level : Level to calculate exp for
	/// @todo Add Formula for RPG2k3
	/// @return Needed experience
	////////////////////////////////////////////////////////
	int CalculateExp(int level);

	////////////////////////////////////////////////////////
	/// Calculates the Experience curve for every level
	////////////////////////////////////////////////////////
	void MakeExpList();

	////////////////////////////////////////////////////////
	/// Converts the currect Exp to a string
	/// @return Exp-String or ------ if Level is max
	////////////////////////////////////////////////////////
	std::string GetExpString();

	////////////////////////////////////////////////////////
	/// Converts the Exp for the next LV to a string
	/// @return Exp-String or ------ if Level is max
	////////////////////////////////////////////////////////
	std::string GetNextExpString();

	/// @return name.
	std::string GetName() const;

	/// @return character graphic filename.
	std::string GetCharacterName() const;

	/// @return character graphic index.
	int GetCharacterIndex() const;

	/// @return face graphic filename.
	std::string GetFaceName() const;

	/// @return face graphic index.
	int GetFaceIndex() const;

	/// @return title
	std::string GetTitle() const;

	/// @return equipped weapon id
	int GetWeaponId() const;

	/// @return equipped shield id
	int GetShieldId() const;

	/// @return equipped armor id
	int GetArmorId() const;

	/// @return equipped helmet id
	int GetHelmetId() const;

	/// @return equipped accessory id
	int GetAccessoryId() const;

	/// @return current level.
	int GetLevel() const;

	/// @return current experience points.
	int GetExp() const;

	////////////////////////////////////////////////////////
	/// Set exp of actor.
	/// @param _exp : exp to set
	////////////////////////////////////////////////////////
	void SetExp(int _exp);

	////////////////////////////////////////////////////////
	/// Set level of actor.
	/// @param _level : level to set
	////////////////////////////////////////////////////////
	void SetLevel(int _level);

	/// @return max hp
	int GetMaxHp();

	/// @return max sp
	int GetMaxSp();

	/// @return learned skills list.
	std::vector<int> GetSkills() const;

	/// @return atk
	int GetBaseAtk() const;

	/// @return def
	int GetBaseDef() const;

	/// @return spi
	int GetBaseSpi() const;

	/// @return agi
	int GetBaseAgi() const;

	void ChangeExp(int exp);

	void ChangeLevel(int level);

private:
	int actor_id;
	std::string name;
	std::string character_name;
	int character_index;
	std::string face_name;
	int face_index;
	std::string title;
	int weapon_id;
	int shield_id;
	int armor_id;
	int helmet_id;
	int accessory_id;
	int level;
	int exp;
	std::vector<int> exp_list;
	std::vector<int> skills;
};

#endif
