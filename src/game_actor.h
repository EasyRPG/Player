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

#ifndef _GAME_ACTOR_H_
#define _GAME_ACTOR_H_

// Headers
#include <string>
#include <vector>
#include <stdint.h>
#include "rpg_saveactor.h"
#include "rpg_learning.h"
#include "game_battler.h"

namespace RPG {
	class Skill;
	class BattleCommand;
	class Item;
	class Class;
}

/**
 * Game_Actor class.
 */
class Game_Actor : public Game_Battler {
public:
	/**
	 * Constructor.
	 *
	 * @param actor_id database actor ID.
	 */
	Game_Actor(int actor_id);

	/**
	 * Sets up the game actor
	 * This is automatically called in the constructor.
	 */
	void Setup();

	/**
	 * Initializes the game actor to the database state.
	 * Sets the skills, HP, SP and experience.
	 */
	void Init();

	/**
	 * Used after savegame loading to replace savegame default values with
	 * database ones.
	 */
	void Fixup();

	/**
	 * Applies the effects of an item.
	 * Tests if using that item makes any sense (e.g. for HP healing
	 * items if there are any HP to heal)
	 *
	 * @param item_id ID if item to use
	 * @return true if item affected anything
	 */
	bool UseItem(int item_id) override;

	/**
	 * Checks if the actor is permitted to use the item at all.
	 *
	 * @param ID of item to test
	 * @return true if item is usable
	 */
	virtual bool IsItemUsable(int item_id) const;

	/**
	 * Learns a new skill.
	 *
	 * @param skill_id database skill ID.
	 * @return If skill was learned (fails if already had the skill)
	 */
	bool LearnSkill(int skill_id);

	/**
	 * Unlearns a skill.
	 *
	 * @param skill_id database skill ID.
	 * @return If skill was unlearned (fails if didn't had the skill)
	 */
	bool UnlearnSkill(int skill_id);

	/**
	 * Unlearns all skills.
	 *
	 */
	void UnlearnAllSkills();

	/**
	 * Checks if the actor has the skill learned.
	 *
	 * @param skill_id ID of skill to check.
	 * @return true if skill has been learned.
	 */
	bool IsSkillLearned(int skill_id) const;

	/**
	 * Checks if the actor can use the skill.
	 *
	 * @param skill_id ID of skill to check.
	 * @return true if skill can be used.
	 */
	bool IsSkillUsable(int skill_id) const override;

	/**
	 * Returns the modifier by which skill costs are divided.
	 *
	 * @return modifier
	 */
	int GetSpCostModifier() const;

	/**
	 * Calculates the Skill costs including all modifiers.
	 *
	 * @param skill_id ID of skill to calculate.
	 * @return needed skill cost.
	 */
	int CalculateSkillCost(int skill_id) const override;

	/**
	 * Gets the actor ID.
	 *
	 * @return Actor ID
	 */
	int GetId() const override;

	/**
	 * Calculates the Exp needed for a level up.
	 *
	 * @param level level to calculate exp for.
	 * @return Needed experience.
	 */
	int CalculateExp(int level) const;

	/**
	 * Calculates the Experience curve for every level.
	 */
	void MakeExpList();

	/**
	 * Converts the currect Exp to a string.
	 *
	 * @return Exp-String or ------ if Level is max.
	 */
	std::string GetExpString() const;

	/**
	 * Converts the Exp for the next LV to a string.
	 *
	 * @return Exp-String or ------ if Level is max.
	 */
	std::string GetNextExpString() const;

	/**
	 * Returns how many Exp are minimum for current level.
	 *
	 * @return Exp needed or -1 if invalid.
	 */
	int GetBaseExp() const;

	/**
	 * Returns how many Exp are minimum for a level.
	 *
	 * @param level to return base Exp for.
	 * @return Exp needed or -1 if invalid.
	 */
	int GetBaseExp(int level) const;

	/**
	 * Returns how many Exp are needed for a level up based on the current
	 * level.
	 *
	 * @return Exp needed or -1 if max level.
	 */
	int GetNextExp() const;

	/**
	 * Return how many Exp are needed for a level up.
	 *
	 * @param level to return NextExp for.
	 * @return Exp needed or -1 if max level / invalid.
	 */
	int GetNextExp(int level) const;

	/**
	 * Gets probability that a state can be inflicted on this actor.
	 *
	 * @param state_id State to test
	 * @return Probability of state infliction
	 */
	int GetStateProbability(int state_id) const override;

	/**
	 * Gets attribute protection when the actor is damaged.
	 *
	 * @param attribute_id Attribute to test
	 * @return Attribute resistence
	 */
	int GetAttributeModifier(int attribute_id) const override;

	/**
	 * Gets actor name.
	 *
	 * @return name.
	 */
	const std::string& GetName() const override;

	/**
	 * Gets actor character sprite filename.
	 *
	 * @return character sprite filename.
	 */
	const std::string& GetSpriteName() const override;

	/**
	 * Gets actor character sprite index.
	 *
	 * @return character sprite index.
	 */
	int GetSpriteIndex() const;

	/**
	 * Gets actor face graphic filename.
	 *
	 * @return face graphic filename.
	 */
	std::string GetFaceName() const;

	/**
	 * Gets actor face graphic index.
	 *
	 * @return face graphic index.
	 */
	int GetFaceIndex() const;

	/**
	 * Gets actor title.
	 *
	 * @return title.
	 */
	std::string GetTitle() const;

	/**
	 * Gets actor equipped weapon ID.
	 *
	 * @return equipped weapon ID.
	 */
	int GetWeaponId() const;

	/**
	 * Gets actor equipped shield ID.
	 *
	 * @return equipped shield ID.
	 */
	int GetShieldId() const;

	/**
	 * Gets actor equipped armor ID.
	 *
	 * @return equipped armor ID.
	 */
	int GetArmorId() const;

	/**
	 * Gets actor equipped helmet ID.
	 *
	 * @return equipped helmet ID.
	 */
	int GetHelmetId() const;

	/**
	 * Gets actor equipped accesory ID.
	 *
	 * @return equipped accessory ID.
	 */
	int GetAccessoryId() const;

	/**
	 * Gets actor current level.
	 *
	 * @return current level.
	 */
	int GetLevel() const;

	/**
	 * Gets final level of current actor.
	 *
	 * @return final level
	 */
	int GetMaxLevel() const;

	/**
	* Gets actor current experience points.
	*
	* @return current experience points.
	*/
	int GetExp() const;

	/**
	 * Sets exp of actor.
	 * The value is adjusted to the boundary 0 up 999999.
	 * Other actor attributes are not altered. Use ChangeExp to do a proper 
	 * experience change.
	 *
	 * @param _exp exp to set.
	 */
	void SetExp(int _exp);

	/**
	 * Changes exp of actor and handles level changing based on the new
	 * experience.
	 *
	 * @param exp new exp.
	 * @param level_up_message Whether to show level up message and learned skills.
	 */
	void ChangeExp(int exp, bool level_up_message);

	/**
	 * Changes level of actor and handles experience changes, skill
	 * learning and other attributes based on the new level.
	 *
	 * @param level new level.
	 * @param level_up_message Whether to show level up message and learned skills.
	 */
	void ChangeLevel(int level, bool level_up_message);

	/**
	 * Sets level of actor.
	 * The value is adjusted to the boundary 1 up to max level.
	 * Other actor attributes are not altered. Use ChangeLevel to do a proper 
	 * level change.
	 *
	 * @param _level level to set.
	 */
	void SetLevel(int _level);

	/**
	 * Checks if the actor can equip the item.
	 *
	 * @param item_id ID of item to check.
	 * @return true if it can equip the item.
	 */
	bool IsEquippable(int item_id) const;

	/**
	 * Checks if the actor has a fixed equipped
	 *
	 * @return true if fixed
	 */
	bool IsEquipmentFixed() const;
	
	/**
	 * Checks if the actors defense skill is stronger the usual.
	 * 
	 * @return true if strong defense
	 */
	bool HasStrongDefense() const override;

	/**
	 * Tests if the battler has a weapon that grants preemption.
	 *
	 * @return true if a weapon is having preempt attribute
	 */
	bool HasPreemptiveAttack() const override;
	
	/**
	 * Sets face graphic of actor.
	 * @param file_name file containing new face.
	 * @param index index of face graphic in the file.
	 */
	void SetFace(const std::string& file_name, int index);

	/**
	 * Gets the equipped equipment based on the type.
	 * @param equip_type type of equipment.
	 * @return item if equipped or nullptr if no equipment.
	 */
	const RPG::Item* GetEquipment(int equip_type) const;

	/**
	 * Sets the equipment based on the type.
	 *
	 * @param equip_type type of equipment.
	 * @param new_item_id item to equip.
	 * @return item_id of old item, or 0 if no equipment or -1 if invalid.
	 */
	int SetEquipment(int equip_type, int new_item_id);

	/**
	 * Changes the equipment of the actor.
	 * Unequips the current inventory item and adds it to the inventory.
	 * The new equipment is taken from the inventory (if available),
	 * otherwise a new item is equipped.
	 * If you don't want this behaviour use SetEquipment instead.
	 *
	 * @param equip_type type of equipment.
	 * @param item_id item to equip.
	 */
	void ChangeEquipment(int equip_type, int item_id);

	/**
	 * Returns an array of all equipped item IDs (or 0 for none).
	 *
	 * @return equipped item array
	 */
	const std::vector<int16_t>& GetWholeEquipment() const;

	/**
	 * Unequips the whole equipment and adds it to the inventory.
	 */
	void RemoveWholeEquipment();

	/**
	 * Gets how often the item with the corresponding id is equipped.
	 *
	 * @param item_id database item ID.
	 * @return number of items.
	 */
	int GetItemCount(int item_id);

	/**
	 * Gets learned skills list.
	 *
	 * @return learned skills list.
	 */
	const std::vector<int16_t>& GetSkills() const;

	/**
	 * Gets a random skill

	 * @return random skill
	 */
	const RPG::Skill& GetRandomSkill() const;

	/**
	 * Gets actor states list.
	 *
	 * @return vector containing the IDs of all states the actor has.
	 */
	const std::vector<int16_t>& GetStates() const override;
	std::vector<int16_t>& GetStates() override;

	/**
	 * Adds a State.
	 *
	 * @param state_id ID of state to add.
	 */
	void AddState(int state_id) override;

	/**
	 * Removes a State.
	 *
	 * @param state_id ID of state to remove.
	 */
	void RemoveState(int state_id) override;

	/**
	 * Removes all states which end after battle.
	 */
	void RemoveBattleStates() override;

	/**
	 * Removes all states.
	 */
	void RemoveAllStates() override;

	int GetHp() const override;
	void SetHp(int _hp) override;
	void ChangeHp(int hp) override;

	int GetSp() const override;
	void SetSp(int _sp) override;

	/**
	 * Gets the max HP for the current level.
	 *
	 * @param mod include the modifier bonus.
	 */
	int GetBaseMaxHp(bool mod) const;

	/**
	 * Gets the max SP for the current level.
	 *
	 * @param mod include the modifier bonus.
	 */
	int GetBaseMaxSp(bool mod) const;

	/**
	 * Gets the attack for the current level.
	 *
	 * @param mod include the modifier bonus.
	 * @param equip include the equipment bonuses.
	 */
	int GetBaseAtk(bool mod, bool equip) const;

	/**
	 * Gets the defense for the current level.
	 *
	 * @param mod include the modifier bonus.
	 * @param equip include the equipment bonuses.
	 */
	int GetBaseDef(bool mod, bool equip) const;

	/**
	 * Gets the spirit for the current level.
	 *
	 * @param mod include the modifier bonus.
	 * @param equip include the equipment bonuses.
	 */
	int GetBaseSpi(bool mod, bool equip) const;

	/**
	 * Gets the agility for the current level.
	 *
	 * @param mod include the modifier bonus.
	 * @param equip include the equipment bonuses.
	 */
	int GetBaseAgi(bool mod, bool equip) const;

	/**
	 * Gets the max HP for the current level.
	 * Modifier and equipment bonuses are included.
	 */
	int GetBaseMaxHp() const override;

	/**
	 * Gets the max SP for the current level.
	 * Modifier and equipment bonuses are included.
	 */
	int GetBaseMaxSp() const override;

	/**
	 * Gets the attack for the current level.
	 * Modifier and equipment bonuses are included.
	 */
	int GetBaseAtk() const override;

	/**
	 * Gets the defense for the current level.
	 * Modifier and equipment bonuses are included.
	 */
	int GetBaseDef() const override;

	/**
	 * Gets the spirit for the current level.
	 * Modifier and equipment bonuses are included.
	 */
	int GetBaseSpi() const override;

	/**
	 * Gets the agility for the current level.
	 * Modifier and equipment bonuses are included.
	 */
	int GetBaseAgi() const override;

	/**
	 * Sets the base max HP by adjusting the modifier bonus.
	 * The existing modifier bonus and equipment bonuses
	 * are taken into account.
	 *
	 * @param _maxhp max HP.
	 */
	void SetBaseMaxHp(int _maxhp);

	/**
	 * Sets the base max SP by adjusting the modifier bonus.
	 * The existing modifier bonus and equipment bonuses
	 * are taken into account.
	 *
	 * @param _maxsp max SP.
	 */
	void SetBaseMaxSp(int _maxsp);

	/**
	 * Sets the base attack by adjusting the modifier bonus.
	 * The existing modifier bonus and equipment bonuses
	 * are taken into account.
	 *
	 * @param _atk attack.
	 */
	void SetBaseAtk(int _atk);

	/**
	 * Sets the base defense by adjusting the modifier bonus.
	 * The existing modifier bonus and equipment bonuses
	 * are taken into account.
	 *
	 * @param _def defense.
	 */
	void SetBaseDef(int _def);

	/**
	 * Sets the base spirit by adjusting the modifier bonus.
	 * The existing modifier bonus and equipment bonuses
	 * are taken into account.
	 *
	 * @param _spi spirit.
	 */
	void SetBaseSpi(int _spi);

	/**
	 * Sets the base agility by adjusting the modifier bonus.
	 * The existing modifier bonus and equipment bonuses
	 * are taken into account.
	 *
	 * @param _agi agility.
	 */
	void SetBaseAgi(int _agi);

	/**
	 * Gets if actor has two weapons.
	 *
	 * @return true if actor has two weapons.
	 */
	bool HasTwoWeapons() const;

	/**
	 * Gets if actor does auto battle.
	 *
	 * @return true if actor does two weapons.
	 */
	bool GetAutoBattle() const;

	/**
	 * Gets X position on battlefield
	 *
	 * @return X position in battle scene
	 */
	int GetBattleX() const override;

	/**
	 * Gets Y position on battlefield
	 *
	 * @return Y position in battle scene
	 */
	int GetBattleY() const override;

	/**
	 * Gets name of skill menu item
	 *
	 * @return name of skill menu item
	 */
	const std::string& GetSkillName() const;

	/**
	 * Sets new actor name.
	 *
	 * @param new_name new name.
	 */
	void SetName(const std::string &new_name);

	/**
	 * Sets new actor title.
	 *
	 * @param new_title new title.
	 */
	void SetTitle(const std::string &new_title);

	/**
	 * Sets actor sprite.
	 *
	 * @param file graphic file.
	 * @param index graphic index.
	 * @param transparent transparent flag.
	 */
	void SetSprite(const std::string &file, int index, bool transparent);

	/**
	 * Changes battle commands.
	 *
	 * @param add true => add the command, false => remove the command.
	 * @param id command to add/remove, 0 to remove all commands.
	 */
	void ChangeBattleCommands(bool add, int id);

	/**
	 * Gets Rpg2k3 hero class.
	 *
	 * @return Rpg2k3 hero class.
	 */
	const RPG::Class* GetClass() const;

	/**
	 * Sets new Rpg2k3 hero class.
	 *
	 * @param class_id mew Rpg2k3 hero class.
	 */
	void SetClass(int class_id);

	/**
	 * Gets the actor's class name as a string.
	 * 
	 * @return Rpg2k3 hero class name
	 */
	std::string GetClassName() const;

	/**
	 * Gets battle commands.
	 *
	 * @return all Rpg2k3 battle commands.
	 */
	const std::vector<const RPG::BattleCommand*> GetBattleCommands() const;

	/**
	 * Gets battle row for Rpg2k3 battles.
	 *
	 * @return row for Rpg2k3 battles (-1 front, 1 back).
	 */
	int GetBattleRow() const;

	/**
	 * Sets battle row for Rpg2k3 battles.
	 *
	 * @param battle_row new row for Rpg2k3 battles (-1 front, 1 back).
	 */
	void SetBattleRow(int battle_row);

	int GetBattleAnimationId() const override;

	int GetHitChance() const override;
	float GetCriticalHitChance() const override;

	std::string GetLevelUpMessage(int new_level) const;
	std::string GetLearningMessage(const RPG::Learning& learn) const;

	BattlerType GetType() const override;
private:
	// same reason as for Game_Picture, see comment
	/**
	 * @return Reference to the SaveActor data
	 */
	RPG::SaveActor& GetData() const;

	/**
	 * Removes invalid (wrong type) equipment from the equipment
	 * slots.
	 */
	void RemoveInvalidEquipment();

	int actor_id;
	std::vector<int> exp_list;
};

#endif
