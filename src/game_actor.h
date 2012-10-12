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
#include "rpg_save.h"
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
	/// Sets up the game actor
	/// This is automatically called in the constructor.
	////////////////////////////////////////////////////////
	void Setup();

	////////////////////////////////////////////////////////
	/// Initializes the game actor to the database state
	/// Sets the skills, HP, SP and experience
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Initializes the game actor to the database state
	/// and overwrites them with savedata values.
	////////////////////////////////////////////////////////
	void Init(const RPG::SaveActor& save_data);

	////////////////////////////////////////////////////////
	/// Learn a new skill.
	/// @param skill_id : database skill id
	////////////////////////////////////////////////////////
	void LearnSkill(int skill_id);

	////////////////////////////////////////////////////////
	/// Unlearn a skill.
	/// @param skill_id : database skill id
	////////////////////////////////////////////////////////
	void UnlearnSkill(int skill_id);

	////////////////////////////////////////////////////////
	/// Checks if the actor has the skill learned
	/// @param skill_id : id of skill to check
	/// @return true if skill has been learned
	////////////////////////////////////////////////////////
	bool IsSkillLearned(int skill_id);

	////////////////////////////////////////////////////////
	/// Checks if the actor can use the skill
	/// @param skill_id : id of skill to check
	/// @return true if skill can be used
	////////////////////////////////////////////////////////
	bool IsSkillUsable(int skill_id);

	////////////////////////////////////////////////////////
	/// Gets the actor ID.
	/// @return Actor ID
	////////////////////////////////////////////////////////
	int GetId() const;

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
	/// Change exp of actor.
	/// @param exp : 
	////////////////////////////////////////////////////////
	void ChangeExp(int exp);

	////////////////////////////////////////////////////////
	/// Change level of actor.
	/// @param level : new level 
	////////////////////////////////////////////////////////
	void ChangeLevel(int level);

	////////////////////////////////////////////////////////
	/// Set level of actor.
	/// @param _level : level to set
	////////////////////////////////////////////////////////
	void SetLevel(int _level);

	////////////////////////////////////////////////////////
	/// Checks if the actor can equip the item
	/// @param item_id : id of item to check
	/// @return true if it can equip the item
	////////////////////////////////////////////////////////
	bool IsEquippable(int item_id);

	////////////////////////////////////////////////////////
	/// Set face graphic of actor.
	/// @param file_name : file containing new face
	/// @param index : index of face graphic in the file
	////////////////////////////////////////////////////////
	void SetFace(const std::string& file_name, int index);

	////////////////////////////////////////////////////////
	/// Gets the equipped equipment based on the type
	/// @param equip_type : Type of equipment
	/// @return item_id or 0 if no equipment or -1 if invalid
	////////////////////////////////////////////////////////
	int GetEquipment(int equip_type) const;

	////////////////////////////////////////////////////////
	/// Sets the equipment based on the type
	/// @param equip_type : Type of equipment
	/// @param new_item_id : Item to equip
	/// @return item_id of old item, or 0 if no equipment or -1 if invalid
	////////////////////////////////////////////////////////
	int SetEquipment(int equip_type, int new_item_id);

	////////////////////////////////////////////////////////
	/// Changes the equipment of the actor.
	/// Removes one instance of that item from the Inventory and adds the old
	/// one of the actor to it.
	/// If you don't want this use SetEquipment instead.
	/// @param equip_type : Type of equipment
	/// @param item_id : Item to equip
	////////////////////////////////////////////////////////
	void ChangeEquipment(int equip_type, int item_id);

	/// @return learned skills list.
	const std::vector<int16_t>& GetSkills() const;

	/// @return Vector containing the ids of all states the actor has
	const std::vector<int16_t>& GetStates() const;
	std::vector<int16_t>& GetStates();

	int GetHp() const;
	void SetHp(int _hp);

	int GetSp() const;
	void SetSp(int _sp);

	////////////////////////////////////////////////////////
	/// Gets the stats for the current level
	/// @param mod : include the modifier bonus
	/// @param equip : include the equipment bonuses
	////////////////////////////////////////////////////////

	int GetBaseMaxHp(bool mod) const;
	int GetBaseMaxSp(bool mod) const;
	int GetBaseAtk(bool mod, bool equip) const;
	int GetBaseDef(bool mod, bool equip) const;
	int GetBaseSpi(bool mod, bool equip) const;
	int GetBaseAgi(bool mod, bool equip) const;

	////////////////////////////////////////////////////////
	/// Gets the stats for the current level
	/// modifier and equipment bonuses are included
	////////////////////////////////////////////////////////

	int GetBaseMaxHp() const;
	int GetBaseMaxSp() const;
	int GetBaseAtk() const;
	int GetBaseDef() const;
	int GetBaseSpi() const;
	int GetBaseAgi() const;

	////////////////////////////////////////////////////////
	/// Sets the base stats by adjusting the modifier bonus
	/// the existing modifier bonus and equipment bonuses
	/// are taken into account
	////////////////////////////////////////////////////////

	void SetBaseMaxHp(int _maxhp);
	void SetBaseMaxSp(int _maxsp);
	void SetBaseAtk(int _atk);
	void SetBaseDef(int _def);
	void SetBaseSpi(int _spi);
	void SetBaseAgi(int _agi);

	/// @return true if actor has two weapons
	bool GetTwoSwordsStyle() const;

	/// @param new_name : new name
	void SetName(const std::string &new_name);

	/// @param new_title : new title
	void SetTitle(const std::string &new_title);

	/// @param file : graphic file
	/// @param index : graphic index
	/// @param transparent : transparent flag
	void SetSprite(const std::string &file, int index, bool transparent);

	/// @param add : true => add the command, false => remove the command
	/// @param id  : command to add/remove, 0 to remove all commands
	void ChangeBattleCommands(bool add, int id);

	int GetClass() const;
	void SetClass(int class_id);

	const std::vector<uint32_t>& GetBattleCommands();

private:
	RPG::SaveActor& data;

	std::vector<int> exp_list;
};

#endif
