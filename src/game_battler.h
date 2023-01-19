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

#ifndef EP_GAME_BATTLER_H
#define EP_GAME_BATTLER_H

// Headers
#include <cstdint>
#include <string>
#include <vector>
#include <limits>
#include <lcf/rpg/state.h>
#include <lcf/dbbitarray.h>
#include "system.h"
#include "state.h"
#include "color.h"
#include "flash.h"
#include "utils.h"
#include "point.h"
#include "string_view.h"
#include "sprite_battler.h"
#include "sprite_weapon.h"

class Game_Party_Base;
class Sprite_Battler;

namespace Game_BattleAlgorithm {
	class AlgorithmBase;
}

typedef std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> BattleAlgorithmRef;

/**
 * Game_Battler class.
 */
class Game_Battler {
public:
	/** Weapon mode used for various accessors, used to specify which weapons to include in calculation */
	enum Weapon {
		/** Use all weapons combined */
		WeaponAll = -1,
		/** Use no weapons */
		WeaponNone = 0,
		/** Use only primary weapon */
		WeaponPrimary = 1,
		/** Use only secondary weapon */
		WeaponSecondary = 2,
	};

	/**
	 * Constructor.
	 */
	Game_Battler();

	Game_Battler(const Game_Battler&) = delete;
	Game_Battler& operator=(const Game_Battler&) = delete;
	Game_Battler(Game_Battler&&) noexcept = default;
	Game_Battler& operator=(Game_Battler&&) noexcept = default;

	virtual ~Game_Battler() = default;

	virtual int MaxHpValue() const = 0;

	virtual int MaxSpValue() const = 0;

	virtual int MaxStatBattleValue() const = 0;

	virtual int MaxStatBaseValue() const = 0;

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
	std::vector<int16_t> GetInflictedStates() const;

	/** @return permenant states that cannot be removed */
	virtual PermanentStates GetPermanentStates() const;

	/**
	 * @return true battler evades all physical attacks.
	 */
	bool EvadesAllPhysicalAttacks() const;

	/**
	 * Apply effects of Conditions to Battler
	 *
	 * @return Damage taken to Battler from conditions
	 */
	int ApplyConditions();

	/**
	 * Gets battler states.
	 * This returns the raw state list with not inflected states set to 0 and
	 * inflected states set to at least 1 (this maps to the turn count).
	 *
	 * @return vector containing state list
	 */
	virtual const std::vector<int16_t>& GetStates() const = 0;
	virtual std::vector<int16_t>& GetStates() = 0;

	/**
	 * Checks all states and returns the first restriction that different to
	 * normal or normal if that is the only restriction.
	 *
	 * @return First non-normal restriction or normal if not restricted
	 */
	lcf::rpg::State::Restriction GetSignificantRestriction() const;

	/**
	 * Gets the Battler ID.
	 *
	 * @return Battler ID
	 */
	virtual int GetId() const = 0;

	/**
	 * Tests if the battler has a "No Action" condition like sleep.
	 *
	 * @return can act
	 */
	bool CanAct() const;

	/**
	 * Tests if the battler has a "No Action" condition which does not auto recover.
	 * If all actors have these conditions, the battle is lost.
	 *
	 * @return true if actor can act or if they cannot act but the state is auto recoverable
	 */
	bool CanActOrRecoverable() const;

	/**
	 * Gets current battler state with highest priority.
	 *
	 * @return the highest priority state affecting the battler.
	 *         Returns nullptr if no states.
	 */
	const lcf::rpg::State* GetSignificantState() const;

	/**
	 * Gets the state probability by rate (A-E).
	 *
	 * @param state_id State to test
	 * @param rate State rate to get
	 * @return state rate (probability)
	 */
	int GetStateRate(int state_id, int rate) const;

	/**
	 * Gets the base attribute rate when actor is damaged, without battle attribute shifts.
	 * 
	 * @param attribute_id Attribute to query
	 * @return Attribute rate
	 */
	virtual int GetBaseAttributeRate(int attribute_id) const = 0;

	/**
	 * Gets the attribute rate when actor is damaged.
	 * 
	 * @param attribute_id Attribute to query
	 * @return Attribute rate
	 */
	int GetAttributeRate(int attribute_id) const;

	/**
	 * Applies a modifier (buff/debuff) to an attribute rate.
	 * GetAttributeModifier will use this shift in the rate lookup.
	 * A shift of +1 changed a C to D and a -1 a C to B.
	 * The maximum shift value is +-1.
	 * Calling this function again applies the new shift to the previous shifts.
	 * The shift is cleared after the battle ended.
	 *
	 * @param attribute_id Attribute to modify
	 * @param shift Shift to apply.
	 * @return the amount shifted.
	 */
	int ShiftAttributeRate(int attribute_id, int shift);

	/**
	 * Checks if we can shift an attribute.
	 *
	 * @return The amount we are able to shift by.
	 */
	int CanShiftAttributeRate(int attribute_id, int shift) const;

	/**
	 * Gets the current modifier (buff/debuff) to an attribute rate.
	 * The shift is cleared after the battle ended.
	 *
	 * @param attribute_id Attribute modified
	 * @return shift Shift applied.
	 */
	int GetAttributeRateShift(int attribute_id) const;

	/**
	 * Checks if the battler is immune to attribute downshifts.
	 *
	 * @return if the battler is immune to attribute downshifts.
	 */
	virtual bool IsImmuneToAttributeDownshifts() const = 0;

	/**
	 * Gets probability that a state can be inflicted on this actor.
	 *
	 * @param state_id State to test
	 * @return Probability of state infliction
	 */
	virtual int GetStateProbability(int state_id) const = 0;

	/**
	 * Gets the characters name
	 *
	 * @return Character name
	 */
	virtual StringView GetName() const = 0;

	/**
	 * Gets the filename of the character sprite
	 *
	 * @return Filename of character sprite
	 */
	virtual StringView GetSpriteName() const = 0;

	/**
	 * Gets battler HP.
	 *
	 * @return current HP.
	 */
	virtual int GetHp() const = 0;

	/**
	 * Sets the current battler HP.
	 *
	 * @param hp the new hp value to try to set
	 * @return the actual hp set
	 */
	virtual int SetHp(int hp) = 0;

	/**
	 * Increases/Decreases hp.
	 * Also handles death condition.
	 *
	 * @param hp relative hp change
	 * @param lethal whether this change can kill the battler or not. If not, minimum 1 hp is allowed.
	 * @return how much hp was actually changed.
	 */
	int ChangeHp(int hp, bool lethal);

	/**
	 * Gets the battler max HP.
	 *
	 * @return current max HP.
	 */
	virtual int GetMaxHp() const;

	/**
	 * Gets if the battler has full hp.
	 *
	 * @return if battler is healty.
	 */
	virtual bool HasFullHp() const;

	/**
	 * Gets battler SP.
	 *
	 * @return current SP.
	 */
	virtual int GetSp() const = 0;

	/**
	 * Sets the current battler SP.
	 * @param sp the new sp value to try to set
	 * @return the actual sp set
	 */
	virtual int SetSp(int sp) = 0;

	/**
	 * Increases/Decreases sp.
	 *
	 * @param sp relative sp change
	 * @return how much hp was actually changed.
	 */
	int ChangeSp(int sp);

	/**
	 * Gets the battler max SP.
	 *
	 * @return current max SP.
	 */
	virtual int GetMaxSp() const;

	/**
	 * Gets if the battler has full sp.
	 *
	 * @return if battler is full of magic.
	 */
	virtual bool HasFullSp() const;

	/**
	 * Calculates a value after considering the inflicted states
	 * which influence attack.
	 *
	 * @return the value after considering the inflicted states.
	 */
	int CalcValueAfterAtkStates(int value) const;

	/**
	 * Calculates a value after considering the inflicted states
	 * which influence defense.
	 *
	 * @return the value after considering the inflicted states.
	 */
	int CalcValueAfterDefStates(int value) const;

	/**
	 * Calculates a value after considering the inflicted states
	 * which influence spirit.
	 *
	 * @return the value after considering the inflicted states.
	 */
	int CalcValueAfterSpiStates(int value) const;

	/**
	 * Calculates a value after considering the inflicted states
	 * which influence agility.
	 *
	 * @return the value after considering the inflicted states.
	 */
	int CalcValueAfterAgiStates(int value) const;

	/**
	 * Gets the current battler attack.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return current attack.
	 */
	int GetAtk(Weapon weapon = Game_Battler::WeaponAll) const;

	/**
	 * Gets the current battler defense.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return current defense.
	 */
	int GetDef(Weapon weapon = Game_Battler::WeaponAll) const;

	/**
	 * Gets the current battler spirit.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return current spirit.
	 */
	int GetSpi(Weapon weapon = Game_Battler::WeaponAll) const;

	/**
	 * Gets the current battler agility.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return current agility.
	 */
	int GetAgi(Weapon weapon = Game_Battler::WeaponAll) const;

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
	 * @param weapon Which weapons to include in calculating result.
	 * @return attack.
	 */
	virtual int GetBaseAtk(Weapon weapon = WeaponAll) const = 0;

	/**
	 * Gets the defense for the current level.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return defense.
	 */
	virtual int GetBaseDef(Weapon weapon = WeaponAll) const = 0;

	/**
	 * Gets the spirit for the current level.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return spirit.
	 */
	virtual int GetBaseSpi(Weapon weapon = WeaponAll) const = 0;

	/**
	 * Gets the agility for the current level.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return agility.
	 */
	virtual int GetBaseAgi(Weapon weapon = WeaponAll) const = 0;

	/** @return whether the battler is facing the opposite it's normal direction */
	bool IsDirectionFlipped() const;

	/**
	 * Set whether the battler is facing the opposite it's normal direction
	 *
	 * @param flip to flip or not
	 */
	void SetDirectionFlipped(bool flip);

	void SetHidden(bool hidden);
	bool IsHidden() const;
	virtual bool IsImmortal() const;

	/** @return true if this battler is in it's party */
	virtual bool IsInParty() const = 0;

	bool Exists() const;
	bool IsDead() const;

	/**
	 * Kills the battler
	 */
	void Kill();

	/**
	 * Checks if the actor can use the skill.
	 *
	 * @param skill_id ID of skill to check.
	 * @return true if skill can be used.
	 */
	virtual bool IsSkillUsable(int skill_id) const;

	/**
	 * Applies the effects of an item.
	 * Tests if using that item makes any sense (e.g. for HP healing
	 * items if there are any HP to heal)
	 *
	 * @param item_id ID if item to use
	 * @return true if item affected anything
	 */
	virtual bool UseItem(int item_id, const Game_Battler* source);

	/**
	 * Applies the effects of a skill.
	 * Tests if using that skill makes any sense (e.g. for HP healing
	 * skills if there are any HP to heal)
	 * Does not reduce the MP, use Game_Party->UseSkill for this.
	 *
	 * @param skill_id ID of skill to use
	 * @param source battler who threw the skill
	 * @return true if skill affected anything
	 */
	virtual bool UseSkill(int skill_id, const Game_Battler* source);

	/**
	 * Calculates the Skill costs including all modifiers.
	 *
	 * @param skill_id ID of skill to calculate.
	 * @return needed skill cost.
	 */
	virtual int CalculateSkillCost(int skill_id) const;

	/**
	 * Calculates the Skill hp costs including all modifiers.
	 *
	 * @param skill_id ID of skill to calculate.
	 * @return needed skill hp cost.
	 */
	virtual int CalculateSkillHpCost(int skill_id) const;

	/**
	 * Calculates the Sp cost for attacking with a weapon.
	 *
	 * @param weapon which weapons to include in calculating result.
	 * @return sp cost for attacking with weapon.
	 */
	virtual int CalculateWeaponSpCost(Weapon weapon = WeaponAll) const;

	/**
	 * Sets the battler attack modifier.
	 *
	 * @param modifier attack modifier
	 */
	void SetAtkModifier(int modifier);

	/**
	 * Sets the battler defense modifier.
	 *
	 * @param modifier defense modifier.
	 */
	void SetDefModifier(int modifier);

	/**
	 * Sets the battler spirity modifier.
	 *
	 * @param modifier spirity modifier.
	 */
	void SetSpiModifier(int modifier);

	/**
	 * Sets the battler agility modifier.
	 *
	 * @param modifier agility modifier.
	 */
	void SetAgiModifier(int modifier);

	/**
	 * Increases/Decreases battler attack modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier was actually changed.
	 */
	int ChangeAtkModifier(int modifier);

	/**
	 * Increases/Decreases battler defense modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier was actually changed.
	 */
	int ChangeDefModifier(int modifier);

	/**
	 * Increases/Decreases battler spirit modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier was actually changed.
	 */
	int ChangeSpiModifier(int modifier);

	/**
	 * Increases/Decreases battler agility modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier was actually changed.
	 */
	int ChangeAgiModifier(int modifier);

	/**
	 * Check if we can increases or decreases battler attack modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier will change.
	 */
	int CanChangeAtkModifier(int modifier) const;

	/**
	 * Check if we can increases or decreases battler defense modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier will change.
	 */
	int CanChangeDefModifier(int modifier) const;

	/**
	 * Check if we can increases or decreases battler spirit modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier will change.
	 */
	int CanChangeSpiModifier(int modifier) const;

	/**
	 * Check if we can increases or decreases battler agility modifier.
	 *
	 * @param modifier relative modifier change
	 * @return how much the modifier will change.
	 */
	int CanChangeAgiModifier(int modifier) const;

	/**
	 * Add a State.
	 *
	 * @param state_id ID of state to add.
	 * @param allow_battle_states allow adding of battle only states
	 *
	 * @return true if the state was added
	 */
	bool AddState(int state_id, bool allow_battle_states);

	/**
	 * Removes a State.
	 *
	 * @param state_id ID of state to remove.
	 * @param always_remove_battle_states remove battle states even if permanent
	 *
	 * @return true if the state was removed
	 */
	bool RemoveState(int state_id, bool always_remove_battle_states);

	/**
	 * Removes all states which end after battle.
	 */
	void RemoveBattleStates();

	/**
	 * Removes all states.
	 */
	void RemoveAllStates();

	/**
	 * Tests if the battler has a state that provides reflect.
	 * Attack skills targeted at this battler will be reflected to the source.
	 *
	 * @return Reflect is enabled.
	 */
	bool HasReflectState() const;

	/* @return current (x,y) position in the battle scene */
	Point GetBattlePosition() const;

	/**
	 *  Set (x,y) position in the battle scene.
	 *
	 *  @param pos new position to set
	 */
	void SetBattlePosition(Point pos);

	/** @return original (x,y) position from the database */
	virtual Point GetOriginalPosition() const  = 0;

	/** @return Adjusted X position on the screen.  */
	int GetDisplayX() const;

	/** @return Adjusted Y position on the screen.  */
	int GetDisplayY() const;

	virtual int GetBattleAnimationId() const = 0;

	/**
	 * Gets the chance to hit for a normal attack.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return hit rate. [0-100]
	 */
	virtual int GetHitChance(Weapon weapon = WeaponAll) const = 0;

	/**
	 * Gets the chance to critical hit for a normal attack.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return critical hit rate [0.0f-1.0f]
	 */
	virtual float GetCriticalHitChance(Weapon weapon = WeaponAll) const = 0;

	/**
	 * @return If battler is charged (next attack double damages)
	 */
	bool IsCharged() const;

	/**
	 * Sets charge state (next attack double damages)
	 *
	 * @param charge new charge state
	 */
	void SetCharged(bool charge);

	/**
	 * @return If battler is defending (next turn, defense is doubled)
	 */
	bool IsDefending() const;

	/**
	 * Set whether the battler is defending
	 *
	 * @param val whether battler is defending.
	 */
	void SetIsDefending(bool val);

	/**
	 * @return If battler has strong defense (defense is tripled when defending)
	 */
	virtual bool HasStrongDefense() const;

	/**
	 * Tests if the battler has a weapon that grants preemption.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return true if a weapon is having preempt attribute
	 */
	virtual bool HasPreemptiveAttack(Weapon weapon = WeaponAll) const;

	/**
	 * Returns the number of times the battler will attack.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return the number of attacks.
	 */
	virtual int GetNumberOfAttacks(Weapon weapon = WeaponAll) const;

	/**
	 * Tests if the battler has a weapon that grants attack all
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return true if a weapon is having attack all attribute
	 */
	virtual bool HasAttackAll(Weapon weapon = WeaponAll) const = 0;

	/**
	 * Tests if the battler has a weapon which ignores evasion.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return If the actor has weapon that ignores evasion
	 */
	virtual bool AttackIgnoresEvasion(Weapon weapon = WeaponAll) const = 0;

	/**
	 * @return If the battler is protected against critical hits.
	 */
	virtual bool PreventsCritical() const = 0;

	/**
	 * @return If the battler has an increased physical evasion rate.
	 */
	virtual bool HasPhysicalEvasionUp() const = 0;


	enum BattlerType {
		Type_Ally,
		Type_Enemy
	};

	virtual BattlerType GetType() const = 0;

	/**
	 * Convenience function to access the party based on the type of this
	 * battler. This function does not ensure that the battler is in the
	 * party.
	 * @return Party this member probably belongs to.
	 */
	Game_Party_Base& GetParty() const;

	/**
	 * Gets the maximal atb gauge value.
	 * When GetAtbGauge() >= this, the battler can act.
	 * Used by RPG2k3 battle system.
 	 */
	static constexpr int GetMaxAtbGauge();

	/**
	 * Gets the current value of the atb gauge.
	 * Used by RPG2k3 battle system.
	 *
	 * @return atb gauge value
	 */
	int GetAtbGauge() const;

	/**
	 * Sets the gauge to the specified value.
	 * Used by RPG2k3 battle system.
	 *
	 * @param value the new value to set
	 */
	void SetAtbGauge(int value);

	/**
	 * Increments the gauge by specified amount.
	 * Used by RPG2k3 battle system.
	 *
	 * @param value the value to add
	 */
	void IncrementAtbGauge(int value);

	/**
	 * Tests if the battler is ready for an action.
	 * Used by RPG2k3 battle system.
	 *
	 * @return If gauge is full
	 */
	bool IsAtbGaugeFull() const;

	/**
	 * @return Offset of flying enemies
	 */
	virtual int GetFlyingOffset() const;

	/**
	 * Updates the Battler
	 */
	virtual void UpdateBattle();

	/**
	 * Gets the current BattleAlgorithm (action to execute in battle)
	 *
	 * @return Current algorithm or NULL if none
	 */
	const BattleAlgorithmRef GetBattleAlgorithm() const;

	/**
	 * Assigns a new battle algorithm (action to execute in battle) to the
	 * battler.
	 *
	 * @param battle_algorithm New algorithm to assign
	 */
	void SetBattleAlgorithm(const BattleAlgorithmRef battle_algorithm);

	/** @return the battle sprite for the battler */
	Sprite_Battler* GetBattleSprite() const;

	/**
	 * Sets the battle sprite
	 * @param s the new sprite
	 */
	void SetBattleSprite(std::unique_ptr<Sprite_Battler> s);

	/** @return the weapon sprite for the battler */
	Sprite_Weapon* GetWeaponSprite() const;

	/**
	 * Sets the weapon sprite
	 * @param w the new sprite
	 */
	void SetWeaponSprite(std::unique_ptr<Sprite_Weapon> w);

	/**
	 * @return Current turn in battle
	 */
	int GetBattleTurn() const;

	/**
	 * Increases the turn counter of the actor and heals states that reached
	 * the required numbers of turns.
	 *
	 * @return Healed states
	 */
	void NextBattleTurn();

	/**
	 * Increases the internal turn counter of states and heals states that
	 * reached the required numbers of turns.
	 *
	 * @return Healed states
	 */
	std::vector<int16_t> BattleStateHeal();

	void SetBattleOrderAgi(int val);
	int GetBattleOrderAgi();

	void SetLastBattleAction(int battle_action);

	int GetLastBattleAction() const;

	/**
	 * Setup the combo command
	 *
	 * @param which battle command to enable combos
	 * @param times how many times to repeat
	 */
	void SetBattleCombo(int command_id, int times);

	/** @return combo command id */
	int GetBattleComboCommand() const;

	/** @return combo command number of times */
	int GetBattleComboTimes() const;

	/**
	 * Initializes battle related data to there default values.
	 */
	virtual void ResetBattle();

	/**
	 * @return Effective physical hit rate modifier from inflicted states.
	 */
	int GetHitChanceModifierFromStates() const;

	/**
	 * Animate a shake for the sprite
	 *
	 * @param strength strength of the shake
	 * @param speed speed of the shake
	 * @param frames how many frames to shake
	 */
	void ShakeOnce(int strength, int speed, int frames);

	/**
	 * Begins a flash.
	 *
	 * @param r red color
	 * @param g blue color
	 * @param b green color
	 * @param power power of the flash
	 * @param frames Duration of the flash in frames
	 */
	void Flash(int r, int g, int b, int power, int frames);

	/** @return current flash color */
	Color GetFlashColor() const;

	/** @return battle frame counter */
	int GetBattleFrameCounter() const;

	/** @return inflicted states as state objects ordered by priority */
	const std::vector<lcf::rpg::State*> GetInflictedStatesOrderedByPriority() const;

protected:
	/** Gauge for RPG2k3 Battle */
	int gauge = 0;

	/** Battle action for next turn */
	BattleAlgorithmRef battle_algorithm;
	int atk_modifier = 0;
	int def_modifier = 0;
	int spi_modifier = 0;
	int agi_modifier = 0;
	int battle_turn = 0;
	int frame_counter = 0;
	int last_battle_action = -1;
	int battle_combo_command_id = -1;
	int battle_combo_times = 1;
	Point position = { 0, 0 };
	bool defending = false;
	bool charged = false;
	bool hidden = false;
	bool direction_flipped = false;

	std::unique_ptr<Sprite_Battler> battle_sprite;
	std::unique_ptr<Sprite_Weapon> weapon_sprite;
	std::vector<int> attribute_shift;

	int battle_order = 0;

	struct ShakeData {
		int32_t position = 0;
		int32_t time_left = 0;
		int32_t strength = 0;
		int32_t speed = 0;
	};
	ShakeData shake;

	struct FlashData {
		int32_t red = 0;
		int32_t green = 0;
		int32_t blue = 0;
		int32_t time_left = 0;
		double current_level = 0.0;
	};
	FlashData flash;
};

inline Color Game_Battler::GetFlashColor() const {
	return Flash::MakeColor(flash.red, flash.green, flash.blue, flash.current_level);
}

inline void Game_Battler::Kill() {
	ChangeHp(-GetHp(), true);
}

inline bool Game_Battler::IsDead() const {
	// RPG_RT compatibility requires checking the death state and not hp.
	return GetHp() == 0;
}

inline bool Game_Battler::Exists() const {
	return !IsHidden() && !IsDead() && IsInParty();
}

inline void Game_Battler::SetAtkModifier(int modifier) {
	atk_modifier = modifier;
}

inline void Game_Battler::SetDefModifier(int modifier) {
	def_modifier = modifier;
}

inline void Game_Battler::SetSpiModifier(int modifier) {
	spi_modifier = modifier;
}

inline void Game_Battler::SetAgiModifier(int modifier) {
	agi_modifier = modifier;
}

inline bool Game_Battler::IsCharged() const {
	return charged;
}

inline void Game_Battler::SetCharged(bool charge) {
	charged = charge;
}

inline bool Game_Battler::IsDefending() const {
	return defending;
}

inline void Game_Battler::SetIsDefending(bool val) {
	defending = val;
}

inline bool Game_Battler::HasStrongDefense() const {
	return false;
}

inline bool Game_Battler::HasPreemptiveAttack(Weapon) const {
	return false;
}

inline int Game_Battler::GetNumberOfAttacks(Weapon) const {
	return 1;
}

inline bool Game_Battler::HasAttackAll(Weapon) const {
	return false;
}

inline void Game_Battler::SetHidden(bool _hidden) {
	hidden = _hidden;
}

inline bool Game_Battler::IsHidden() const {
	return hidden;
}

inline bool Game_Battler::IsImmortal() const {
	return false;
}

constexpr int Game_Battler::GetMaxAtbGauge() {
	return 300000;
}

inline void Game_Battler::SetAtbGauge(int value) {
	gauge = Utils::Clamp(value, 0, GetMaxAtbGauge());
}

inline int Game_Battler::GetAtbGauge() const {
	return gauge;
}

inline void Game_Battler::IncrementAtbGauge(int amount) {
	SetAtbGauge(GetAtbGauge() + amount);
}

inline bool Game_Battler::IsAtbGaugeFull() const {
	return gauge >= GetMaxAtbGauge();
}

inline int Game_Battler::GetFlyingOffset() const {
	return 0;
}

inline const BattleAlgorithmRef Game_Battler::GetBattleAlgorithm() const {
	return battle_algorithm;
}

inline void Game_Battler::SetBattleAlgorithm(BattleAlgorithmRef battle_algorithm) {
	this->battle_algorithm = battle_algorithm;
}

inline Sprite_Battler* Game_Battler::GetBattleSprite() const {
	return battle_sprite.get();
}

inline void Game_Battler::SetBattleSprite(std::unique_ptr<Sprite_Battler> s) {
	battle_sprite = std::move(s);
}

inline Sprite_Weapon* Game_Battler::GetWeaponSprite() const {
	return weapon_sprite.get();
}

inline void Game_Battler::SetWeaponSprite(std::unique_ptr<Sprite_Weapon> w) {
	weapon_sprite = std::move(w);
}

inline void Game_Battler::NextBattleTurn() {
	++battle_turn;
}

inline int Game_Battler::GetBattleTurn() const {
	return battle_turn;
}

inline void Game_Battler::SetLastBattleAction(int battle_action) {
	last_battle_action = battle_action;
}

inline int Game_Battler::GetLastBattleAction() const {
	return last_battle_action;
}

inline void Game_Battler::SetBattleCombo(int command_id, int times) {
	battle_combo_command_id = command_id;
	battle_combo_times = times;
}

inline int Game_Battler::GetBattleComboCommand() const {
	return battle_combo_command_id;
}

inline int Game_Battler::GetBattleComboTimes() const {
	return std::max(1, battle_combo_times);
}

inline void Game_Battler::SetBattleOrderAgi(int val) {
	battle_order = val;
}

inline int Game_Battler::GetBattleOrderAgi() {
	return battle_order;
}

inline Point Game_Battler::GetBattlePosition() const {
	return position;
}

inline void Game_Battler::SetBattlePosition(Point pos) {
	position = pos;
}

inline bool Game_Battler::IsDirectionFlipped() const {
	return direction_flipped;
}

inline void Game_Battler::SetDirectionFlipped(bool flip) {
	direction_flipped = flip;
}

inline int Game_Battler::GetBattleFrameCounter() const {
	return frame_counter;
}

inline int Game_Battler::CalculateWeaponSpCost(Weapon) const {
	return 0;
}

#endif
