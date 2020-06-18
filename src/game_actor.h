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

#ifndef EP_GAME_ACTOR_H
#define EP_GAME_ACTOR_H

// Headers
#include <string>
#include <vector>
#include <cstdint>
#include <lcf/rpg/saveactor.h>
#include <lcf/rpg/learning.h>
#include "game_battler.h"

namespace lcf {
namespace rpg {
	class Actor;
	class Skill;
	class BattleCommand;
	class Item;
	class Class;
} // namespace rpg
} // namespace lcf

class PendingMessage;

/**
 * Game_Actor class.
 */
class Game_Actor final : public Game_Battler {
public:
	using RowType = lcf::rpg::SaveActor::RowType;
	/**
	 * Constructor.
	 *
	 * @param actor_id database actor ID.
	 */
	Game_Actor(int actor_id);

	int MaxHpValue() const override;

	int MaxStatBattleValue() const override;

	int MaxStatBaseValue() const override;

	virtual PermanentStates GetPermanentStates() const override;

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
	bool UseItem(int item_id, const Game_Battler* source) override;

	/**
	 * Checks if the actor is permitted to use the item at all.
	 *
	 * @param item_id of item to test
	 * @return true if item is usable
	 */
	virtual bool IsItemUsable(int item_id) const;

	/**
	 * Learns a new skill.
	 *
	 * @param skill_id database skill ID.
	 * @return If skill was learned (fails if already had the skill)
	 * @param pm If non-null, will push the learned skill message if learned.
	 */
	bool LearnSkill(int skill_id, PendingMessage* pm);

	/**
	 * Learn all the skills from min_level to max_level
	 *
	 * @param min_level the minimum level to determine which skills to learn.
	 * @param max_level the minimum level to determine which skills to learn.
	 * @param pm If non-null, will push the learned skill messages if learned.
	 * @return number of skills learned
	 */
	int LearnLevelSkills(int min_level, int max_level, PendingMessage* pm);

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
	 * Calculates the Skill costs including all modifiers.
	 *
	 * @param skill_id ID of skill to calculate.
	 * @return needed skill cost.
	 */
	int CalculateSkillCost(int skill_id) const override;

	/**
	 * @return sp cost for attacking with weapon.
	 */
	int CalculateWeaponSpCost() const;

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
	 * Gets the transparency level of the actor sprite
	 */
	int GetSpriteTransparency() const;

	/**
	 * Gets actor face graphic filename.
	 *
	 * @return face graphic filename.
	 */
	const std::string& GetFaceName() const;

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
	const std::string& GetTitle() const;

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
	 * @return actor's weapon if equipped and type == lcf::rpg::Item::Type_weapon
	 */
	const lcf::rpg::Item* GetWeapon() const;

	/**
	 * @return actor's 2nd weapon if equipped and type == lcf::rpg::Item::Type_weapon
	 */
	const lcf::rpg::Item* Get2ndWeapon() const;

	/**
	 * @return actor's shield if equipped and type == lcf::rpg::Item::Type_shield
	 */
	const lcf::rpg::Item* GetShield() const;

	/**
	 * @return actor's armor if equipped and type == lcf::rpg::Item::Type_armor
	 */
	const lcf::rpg::Item* GetArmor() const;

	/**
	 * @return actor's helmet if equipped and type == lcf::rpg::Item::Type_helmet
	 */
	const lcf::rpg::Item* GetHelmet() const;

	/**
	 * @return actor's accessory if equipped and type == lcf::rpg::Item::Type_accessory
	 */
	const lcf::rpg::Item* GetAccessory() const;

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
	 * @param pm If non-null, will push the level up message and learned skills.
	 */
	void ChangeExp(int exp, PendingMessage* pm);

	/**
	 * Changes level of actor and handles experience changes, skill
	 * learning and other attributes based on the new level.
	 *
	 * @param level new level.
	 * @param pm If non-null, will push the level up message and learned skills.
	 */
	void ChangeLevel(int level, PendingMessage* pm);

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
	const lcf::rpg::Item* GetEquipment(int equip_type) const;

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
	 * Checks if the actor has a specific item equipped.
	 *
	 * @param equip_id item id to check
	 * @return Whether it has the item equipped
	 */
	bool IsEquipped(int equip_id) const;

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
	const lcf::rpg::Skill* GetRandomSkill() const;

	/**
	 * Gets actor states list.
	 *
	 * @return vector containing the IDs of all states the actor has.
	 */
	const std::vector<int16_t>& GetStates() const override;
	std::vector<int16_t>& GetStates() override;

	void FullHeal();

	int GetHp() const override;
	void SetHp(int _hp) override;

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
	const lcf::rpg::Class* GetClass() const;

	/** Describes how skills change when we change class */
	enum ClassChangeSkillMode {
		/** Don't change skills */
		eSkillNoChange,
		/** Reset skills based on level of new class */
		eSkillReset,
		/** Add all skills for new class */
		eSkillAdd,
	};

	/** Describes how parameters change before we change class */
	enum ClassChangeParamMode {
		/** Don't change parameters */
		eParamNoChange,
		/** Halve all parameters */
		eParamHalf,
		/** Reset parameters based on level 1 of new class */
		eParamResetLevel1,
		/** Reset parameters based on level of new class */
		eParamReset,
	};

	/**
	 * Sets new Rpg2k3 hero class.
	 *
	 * @param class_id new Rpg2k3 hero class.
	 * @param level change hero level to level
	 * @param skill the skill change behavior
	 * @param param the parameter change behavior
	 * @param pm If non-null, will push the level up message and learned skills.
	 */
	void ChangeClass(int class_id,
			int new_level,
			ClassChangeSkillMode skill,
			ClassChangeParamMode param,
			PendingMessage* pm
			);

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
	const std::vector<const lcf::rpg::BattleCommand*> GetBattleCommands() const;

	/**
	 * Gets battle row for Rpg2k3 battles.
	 *
	 * @return row for Rpg2k3 battles
	 */
	RowType GetBattleRow() const;

	/**
	 * Sets battle row for Rpg2k3 battles.
	 *
	 * @param battle_row new row for Rpg2k3 battles
	 */
	void SetBattleRow(RowType battle_row);

	/**
	 * Tests if the battler has a weapon that grants preemption.
	 *
	 * @return true if a weapon is having preempt attribute
	 */
	bool HasPreemptiveAttack() const override;

	/**
	 * Tests if the battler has a weapon that grants dual attack.
	 *
	 * @return true if a weapon is having dual attack attribute
	 */
	bool HasDualAttack() const;

	/**
	 * Tests if the battler has a weapon that grants attack all
	 *
	 * @return true if a weapon is having attack all attribute
	 */
	bool HasAttackAll() const;

	/**
	 * @return If the actor has weapon that ignores evasion
	 */
	bool AttackIgnoresEvasion() const;

	/**
	 * @return If the actor has equipment that protects against terrain damage.
	 */
	bool PreventsTerrainDamage() const;

	/**
	 * @return If the actor has an equipment that protects against critical hits.
	 */
	bool PreventsCritical() const;

	/**
	 * @return If the actor has an equipment that with physical evasion up.
	 */
	bool HasPhysicalEvasionUp() const;

	/**
	 * @return If the actor has an equipment with half sp cost.
	 */
	bool HasHalfSpCost() const;

	int GetBattleAnimationId() const override;

	int GetHitChance() const override;
	float GetCriticalHitChance() const override;

	std::string GetLevelUpMessage(int new_level) const;
	std::string GetLearningMessage(const lcf::rpg::Skill& skill) const;

	BattlerType GetType() const override;

	/**
	 * @return true if the actor is controllable in battle.
	 */
	int IsControllable() const;

	/** 
	 * Reset all equipment inflicted states
	 *
	 * @param allow_battle_states allow battle states to be added.
	 */
	void ResetEquipmentStates(bool allow_battle_states);

	bool IsInParty() const override;

private:
	void AdjustEquipmentStates(const lcf::rpg::Item* item, bool add, bool allow_battle_states);

	/**
	 * @return Reference to the Actor data of the LDB
	 */
	const lcf::rpg::Actor& GetActor() const;

	// same reason as for Game_Picture, see comment
	/**
	 * @return Reference to the SaveActor data
	 */
	lcf::rpg::SaveActor& GetData() const;

	/**
	 * Removes invalid data from the actor.
	 */
	void RemoveInvalidData();

	int actor_id;
	std::vector<int> exp_list;
};

inline Game_Battler::BattlerType Game_Actor::GetType() const {
	return Game_Battler::Type_Ally;
}

inline void Game_Actor::SetName(const std::string &new_name) {
	GetData().name = new_name;
}


inline const std::string& Game_Actor::GetName() const {
	return GetData().name;
}

inline void Game_Actor::SetTitle(const std::string &new_title) {
	GetData().title = new_title;
}

inline const std::string& Game_Actor::GetTitle() const {
	return GetData().title;
}

inline const std::string& Game_Actor::GetSpriteName() const {
	return GetData().sprite_name;
}

inline int Game_Actor::GetSpriteIndex() const {
	return GetData().sprite_id;
}

inline int Game_Actor::GetSpriteTransparency() const {
	return GetData().transparency;
}

inline const std::string& Game_Actor::GetFaceName() const {
	return GetData().face_name;
}

inline int Game_Actor::GetFaceIndex() const {
	return GetData().face_id;
}

inline int Game_Actor::GetLevel() const {
	return GetData().level;
}

inline int Game_Actor::GetExp() const {
	return GetData().exp;
}

inline int Game_Actor::GetHp() const {
	return GetData().current_hp;
}

inline int Game_Actor::GetSp() const {
	return GetData().current_sp;
}

inline bool Game_Actor::HasTwoWeapons() const {
	return GetData().two_weapon;
}

inline bool Game_Actor::GetAutoBattle() const {
	return GetData().auto_battle;
}

inline bool Game_Actor::HasStrongDefense() const {
	return GetData().super_guard;
}

inline const std::vector<int16_t>& Game_Actor::GetSkills() const {
	return GetData().skills;
}

inline const std::vector<int16_t>& Game_Actor::GetStates() const {
	return GetData().status;
}

inline std::vector<int16_t>& Game_Actor::GetStates() {
	return GetData().status;
}

inline const std::vector<int16_t>& Game_Actor::GetWholeEquipment() const {
	return GetData().equipped;
}

inline int Game_Actor::GetId() const {
	return actor_id;
}



#endif
