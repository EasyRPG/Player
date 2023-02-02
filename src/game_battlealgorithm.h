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

#ifndef EP_GAME_BATTLEALGORITHM_H
#define EP_GAME_BATTLEALGORITHM_H

#include <cstdint>
#include <string>
#include <vector>
#include <bitset>
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/state.h>
#include "string_view.h"
#include "game_battler.h"

class Game_Battler;
class Game_Party_Base;

/**
 * Contains algorithms to handle the different battle attacks, skills and items.
 * The algorithms support single targets and party targets.
 * For party targets the caller is responsible for retargeting using TargetNext.
 *
 * The action is simulated using Execute and the results can be applied after the
 * simulation by calling Apply.
 */
namespace Game_BattleAlgorithm {

enum class Type {
	None,
	Normal,
	Skill,
	Item,
	Defend,
	Observe,
	Charge,
	SelfDestruct,
	Escape,
	Transform,
	DoNothing,
};

struct StateEffect {
	enum Effect : int16_t {
		None,
		Inflicted,
		AlreadyInflicted,
		Healed,
		HealedByAttack
	};
	int16_t state_id = 0;
	Effect effect = None;

	StateEffect() = default;
	StateEffect(int state_id, Effect effect)
		: state_id(state_id), effect(effect) {}
};

struct AttributeEffect {
	int16_t attr_id = 0;
	int16_t shift = 0;

	AttributeEffect() = default;
	AttributeEffect(int id, int shift)
		: attr_id(id), shift(shift) {}
};

class AlgorithmBase {
public:
	virtual ~AlgorithmBase() {}

	/** @return the source of the battle action. */
	Game_Battler* GetSource() const;

	/** @return current target battler */
	Game_Battler* GetTarget() const;

	/** @return If the action was reflected, returns the target which triggered the reflect */
	Game_Battler* GetReflectTarget() const;

	/** @return true if this algorithm targets a party */
	Game_Party_Base* GetOriginalPartyTarget() const;

	/** @return the original targets of the action before reflect or other modifications */
	Span<Game_Battler* const> GetOriginalTargets() const;

	/** @return If the action originally had a single target, return that target. Otherwise return nullptr */
	Game_Battler* GetOriginalSingleTarget() const;

	/** @return the current repetition of the algorithm */
	int GetCurrentRepeat() const;

	/** @return the total number of times this algo will repeat */
	int GetTotalRepetitions() const;

	/** Initializes targetting and performs any initial actions such as sp cost reduction for the user. */
	void Start();

	/**
	 * If IsReflected(target) true on any target, will reflect the action back on the source or the source's party.
	 *
	 * @return true if this algo was reflected.
	 */
	bool ReflectTargets();

	/**
	 * Add a new target to the algo.
	 *
	 * @param target the target to add
	 * @param set_current Whether or not to reset the current target to this one
	 */
	void AddTarget(Game_Battler* target, bool set_current);

	/**
	 * Add all party targets to the algo.
	 *
	 * @param targets the party to add
	 * @param set_current Whether or not to reset the current target to the first member of the party
	 */
	void AddTargets(Game_Party_Base* party, bool set_current);

	/**
	 * Changes the target reference to the next target.
	 * When reaches the last target, will return false and reset back to first target.
	 *
	 * @return true if there was a next target available.
	 */
	bool TargetNext();

	/**
	 * Performs the next repeated action.
	 *
	 * @param require_valid_target only repeat if current target is valid.
	 * @return true if the action should be repeated. Once false is returned, the repetition resets.
	 */
	bool RepeatNext(bool require_valid_target);

	/**
	 * Defines switches that will be switched on after the action is finished.
	 * Multiple calls to this function will add additional switches to the list.
	 * 
	 * @param switch_id Switch to turn on
	 */
	void SetSwitchEnable(int switch_id);

	/**
	 * Defines switches that will be switched off after the action is finished.
	 * Multiple calls to this function will add additional switches to the list.
	 *
	 * @param switch_id Switch to turn off
	 */
	void SetSwitchDisable(int switch_id);

	/** @return activated switch id or 0 when algorithm didn't affect a switch */
	int GetAffectedSwitch() const;

	/** @return true if this action affects hp. */
	bool IsAffectHp() const;

	/** @return true if this action absorbs hp. */
	bool IsAbsorbHp() const;

	/** @return true if this action affects sp. */
	bool IsAffectSp() const;

	/** @return true if this action absorbs sp. */
	bool IsAbsorbSp() const;

	/** @return true if this action affects atk. */
	bool IsAffectAtk() const;

	/** @return true if this action absorbs atk. */
	bool IsAbsorbAtk() const;

	/** @return true if this action affects def. */
	bool IsAffectDef() const;

	/** @return true if this action absorbs def. */
	bool IsAbsorbDef() const;

	/** @return true if this action affects spi. */
	bool IsAffectSpi() const;

	/** @return true if this action absorbs spi. */
	bool IsAbsorbSpi() const;

	/** @return true if this action affects agi. */
	bool IsAffectAgi() const;

	/** @return true if this action affects agi. */
	bool IsAbsorbAgi() const;

	/** @return signed value of how much hp is to be gained or lost */
	int GetAffectedHp() const;

	/** @return signed value of how much sp is to be gained or lost */
	int GetAffectedSp() const;

	/** @return signed value of how much attack is to be gained or lost */
	int GetAffectedAtk() const;

	/** @return signed value of how much defense is to be gained or lost */
	int GetAffectedDef() const;

	/** @return signed value of how much spirit is to be gained or lost */
	int GetAffectedSpi() const;

	/** @return signed value of how much agility is to be gained or lost */
	int GetAffectedAgi() const;

	/** @return all states changes caused by this action in order. */
	const std::vector<StateEffect>& GetStateEffects() const;

	/** @return all attributes which are shifited by this action. */
	const std::vector<AttributeEffect>& GetShiftedAttributes() const;

	/**
	 * Returns whether the action hit the target.
	 * This function returns the same as the last Execute-call.
	 *
	 * @return if the action hit the target
	 */
	bool IsSuccess() const;

	/** @return Whether action was positive (e.g. healing) instead of damage. */
	bool IsPositive() const;

	/** @return Whether target will be revived from death */
	bool IsRevived() const;

	/** @return if the last action was a critical hit.  */
	bool IsCriticalHit() const;

	/**
	 * Gets the Battle Animation that is assigned to the Algorithm
	 *
	 * @param i Which animation to fetch, starting from 0.
	 * @return Battle Animation id or 0 if no animation is assigned
	 */
	virtual int GetAnimationId(int i) const;

	/**
	 * Plays the battle animation on all valid targets starting from the current target to the last target.
	 * Takes care of single and multi-target animations.
	 *
	 * @param anim_id the ID of the animation to play.
	 * @param sound_only Only play sounds
	 * @param cutoff If >= 0 maximum number of frames to play
	 * @param invert Flips the animation
	 * @return the number of frames the animation will play
	 */
	int PlayAnimation(int anim_id, bool sound_only = false, int cutoff = -1, bool invert = false);

	/**
	 * Executes the algorithm. Must be called before using the other functions.
	 * This function only simulates the Algorithm, call Apply to add the
	 * changes of the last Execute call to the target.
	 *
	 * @return true if the action was successful, false if failed/dodged
	 */
	bool Execute();

	/** Apply custom effects */
	virtual void ApplyCustomEffect();

	/**
	 * Apply switch enabled by action
	 * @return the switch id enabled, or 0 if none.
	 */
	int ApplySwitchEffect();

	/**
	 * Apply hp damage or healing.
	 * @note Hp healing is not applied if the action revivies
	 * @return the amount of hp changed.
	 */
	int ApplyHpEffect();

	/**
	 * Apply sp increase or decrease.
	 * @return the amount of sp changed.
	 */
	int ApplySpEffect();

	/**
	 * Apply atk increase or decrease.
	 * @return the amount of atk changed.
	 */
	int ApplyAtkEffect();

	/**
	 * Apply def increase or decrease.
	 * @return the amount of def changed.
	 */
	int ApplyDefEffect();

	/**
	 * Apply spi increase or decrease.
	 * @return the amount of spi changed.
	 */
	int ApplySpiEffect();

	/**
	 * Apply agi increase or decrease.
	 * @return the amount of agi changed.
	 */
	int ApplyAgiEffect();

	/**
	 * Apply the given state effect.
	 * @return true if state was successfully added or removed.
	 */
	bool ApplyStateEffect(StateEffect se);

	/** Apply all state effects */
	void ApplyStateEffects();

	/**
	 * Apply the given attribute effect
	 * @return the amount the attribute was shifted.
	 */
	int ApplyAttributeShiftEffect(AttributeEffect ae);

	/** Apply all attribute effects */
	void ApplyAttributeShiftEffects();

	/** Apply all effects in order */
	void ApplyAll();

	/** Apply switches set on the action externally (e.g. enemy actions) */
	void ProcessPostActionSwitches();

	/**
	 * Tests if it makes sense to apply an action on the target.
	 * E.g. when it is dead.
	 *
	 * @param target the target to check
	 * @return true if valid, in case of false another target should be selected.
	 */
	virtual bool IsTargetValid(const Game_Battler& target) const;

	/** @return whether the current target is valid */
	bool IsCurrentTargetValid() const;

	/**
	 * Gets the first line message that is displayed when the action is invoked.
	 * Usually of style "[Name] uses/casts [Weapon/Item/Skill]".
	 *
	 * @param line which line of the message to fetch
	 * @return message
	 */
	virtual std::string GetStartMessage(int line) const;

	/** @return the pose the source should be in when performing the action */
	virtual int GetSourcePose() const;

	/** @return the CBA movement to use when performing the action */
	virtual int GetCBAMovement() const;

	/** @return the CBA afterimage setting to use when performing the action */
	virtual int GetCBAAfterimage() const;

	/** @return true if it is still possible to perform this action now.  */
	virtual bool ActionIsPossible() const;

	/** @return the weapon animation data for this action (if applicable) */
	virtual const lcf::rpg::BattlerAnimationItemSkill* GetWeaponAnimationData() const;

	/** @return the weapon data for this action (if applicable) */
	virtual const lcf::rpg::Item* GetWeaponData() const;

	/**
	 * Gets the sound effect that is played when the action is starting.
	 *
	 * @return start se
	 */
	virtual const lcf::rpg::Sound* GetStartSe() const;

	/**
	 * Gets the sound effect that is played then the action fails. 
	 *
	 * @return result se
	 */
	virtual const lcf::rpg::Sound* GetFailureSe() const;

	/**
	 * Returns the message used when the action fails.
	 *
	 * @return failure message
	 */
	virtual std::string GetFailureMessage() const;

	/**
	 * Returns whether the attack would be reflected if used upon the target.
	 *
	 * @param the target to check
	 * @return true when reflected
	 */
	virtual bool IsReflected(const Game_Battler& target) const;

	/**
	 * Returns the algorithm type of this object.
	 */
	Type GetType() const;

	/**
	 * Set number of times to repeat the same action on a target
	 */
	void SetRepeat(int repeat);

	/** 
	 * Apply a combo number of hits to repeat the action.
	 *
	 * @param hits the number of combo hits
	 */
	virtual void ApplyComboHitsMultiplier(int hits);

	/**
	 * Set the affected switch id 
	 * @param s the switch id
	 */
	int SetAffectedSwitch(int s);

	/**
	 * Set the affected hp
	 * @param hp the signed hp gain/loss value
	 */
	int SetAffectedHp(int hp);

	/**
	 * Set whether the effect will absorb hp from the target
	 * @param a whether to absorb or not.
	 */
	bool SetIsAbsorbHp(bool a);

	/**
	 * Set the affected sp
	 * @param hp the signed sp gain/loss value
	 */
	int SetAffectedSp(int sp);

	/**
	 * Set whether the effect will absorb sp from the target
	 * @param a whether to absorb or not.
	 */
	bool SetIsAbsorbSp(bool a);

	/**
	 * Set the affected atk
	 * @param hp the signed atk gain/loss value
	 */
	int SetAffectedAtk(int hp);

	/**
	 * Set whether the effect will absorb atk from the target
	 * @param a whether to absorb or not.
	 */
	bool SetIsAbsorbAtk(bool a);

	/**
	 * Set the affected def
	 * @param hp the signed def gain/loss value
	 */
	int SetAffectedDef(int hp);

	/**
	 * Set whether the effect will absorb def from the target
	 * @param a whether to absorb or not.
	 */
	bool SetIsAbsorbDef(bool a);

	/**
	 * Set the affected spi
	 * @param hp the signed spi gain/loss value
	 */
	int SetAffectedSpi(int hp);

	/**
	 * Set whether the effect will absorb spi from the target
	 * @param a whether to absorb or not.
	 */
	bool SetIsAbsorbSpi(bool a);

	/**
	 * Set the affected agi
	 * @param hp the signed agi gain/loss value
	 */
	int SetAffectedAgi(int hp);

	/**
	 * Set whether the effect will absorb agi from the target
	 * @param a whether to absorb or not.
	 */
	bool SetIsAbsorbAgi(bool a);

	/**
	 * Add a state effect
	 * @param se the state effect to add
	 */
	void AddAffectedState(StateEffect se);

	/**
	 * Add an attribute shift effect
	 * @param se the attribute shift effect to add
	 */
	void AddAffectedAttribute(AttributeEffect ae);

	/**
	 * Set if the original intention was positive (healing)
	 * @param p if positive or not
	 */
	bool SetIsPositive(bool p);

	/**
	 * Set if the effect was a critical hit
	 * @param c if critical hit or not
	 */
	bool SetIsCriticalHit(bool c);

	/** Set if the algo was a success  */
	bool SetIsSuccess();

	/**
	 * Set the algo to successful if the condition is true. If it's false, no change.
	 * @param x condition to check
	 */
	bool SetIsSuccessIf(bool x);

	/** Set if the algo failed  */
	bool SetIsFailure();

protected:
	AlgorithmBase(Type t, Game_Battler* source, Game_Battler* target);
	AlgorithmBase(Type t, Game_Battler* source, std::vector<Game_Battler*> targets);
	AlgorithmBase(Type t, Game_Battler* source, Game_Party_Base* target);
	virtual bool vStart();
	virtual bool vExecute();

	void Reset();

	/**
	 * Implements logic of TargetNext but ignores reflect.
	 * Used by const-functions that restore the old state afterwards.
	 * So technically this function is non-const but due to the help of the
	 * other functions the behaviour to the callee is const...
	 *
	 * @return true if there was a next target available
	 */
	bool TargetNextInternal();

	void BattlePhysicalStateHeal(int physical_rate, std::vector<int16_t>& target_states, const PermanentStates& ps);

private:
	Type type = Type::None;
	Game_Battler* source = nullptr;
	std::vector<Game_Battler*> targets;
	std::vector<Game_Battler*>::iterator current_target;
	Game_Party_Base* party_target = nullptr;
	Game_Battler* reflect_target = nullptr;

	int hp = 0;
	int sp = 0;
	int attack = 0;
	int defense = 0;
	int spirit = 0;
	int agility = 0;
	int switch_id = 0;

	enum Flag {
		eSuccess,
		ePositive,
		eCriticalHit,
		eRevived,
		eAffectHp,
		eAbsorbHp,
		eAffectSp,
		eAbsorbSp,
		eAffectAtk,
		eAbsorbAtk,
		eAffectDef,
		eAbsorbDef,
		eAffectSpi,
		eAbsorbSpi,
		eAffectAgi,
		eAbsorbAgi,
	};
	std::bitset<64> flags = {};
	int num_original_targets = 0;
	int cur_repeat = 0;
	int repeat = 1;

	std::vector<StateEffect> states;
	std::vector<AttributeEffect> attributes;
	std::vector<int> switch_on;
	std::vector<int> switch_off;

	bool SetFlag(Flag f, bool value);
	bool GetFlag(Flag f) const;
};

// Special algorithm for handling non-moving because of states
class None : public AlgorithmBase {
public:
	None(Game_Battler* source);
};


class Normal : public AlgorithmBase {
public:
	enum Style {
		/** 2k style, single combined attack with both weapons */
		Style_Combined,
		/** 2k3 style, multiple attacks, one per weapon */
		Style_MultiHit,
	};

	static Style GetDefaultStyle();

	Normal(Game_Battler* source, Game_Battler* target, int hits_multiplier = 1, Style style = GetDefaultStyle());
	Normal(Game_Battler* source, Game_Party_Base* target, int hits_multiplier = 1, Style style = GetDefaultStyle());

	bool vExecute() override;
	bool vStart() override;

	int GetAnimationId(int i) const override;
	std::string GetStartMessage(int line) const override;
	int GetSourcePose() const override;
	int GetCBAMovement() const override;
	int GetCBAAfterimage() const override;
	const lcf::rpg::BattlerAnimationItemSkill* GetWeaponAnimationData() const override;
	const lcf::rpg::Item* GetWeaponData() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	Game_Battler::Weapon GetWeapon() const;
	void ApplyComboHitsMultiplier(int hits) override;

	// Emulates an RPG_RT bug where whenver an actor attacks an enemy, the hit rate and damage
	// is adjusted as if the enemy were in the front row.
	void SetTreatEnemiesAsIfInFrontRow(bool v);

	// Return true if this is a charged attack.
	bool IsChargedAttack() const;
private:
	void Init(Style style);
	int hits_multiplier = 1;
	int weapon_style = -1;
	bool charged_attack = false;
	bool treat_enemies_asif_in_front_row = false;
};

class Skill : public AlgorithmBase {
public:
	Skill(Game_Battler* source, Game_Battler* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);
	Skill(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);
	Skill(Game_Battler* source, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);

	bool IsTargetValid(const Game_Battler&) const override;
	bool vExecute() override;
	bool vStart() override;

	int GetAnimationId(int i) const override;
	std::string GetStartMessage(int line) const override;
	int GetSourcePose() const override;
	int GetCBAMovement() const override;
	int GetCBAAfterimage() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	const lcf::rpg::Sound* GetFailureSe() const override;
	std::string GetFailureMessage() const override;
	bool IsReflected(const Game_Battler&) const override;
	bool ActionIsPossible() const override;

	const lcf::rpg::Skill& GetSkill() const;
	const lcf::rpg::Item* GetItem() const;

	// Emulates an RPG_RT bug where whenver an actor attacks an enemy, the hit rate and damage
	// is adjusted as if the enemy were in the front row.
	void SetTreatEnemiesAsIfInFrontRow(bool v);

private:
	void Init();
	std::string GetFirstStartMessage() const;
	std::string GetSecondStartMessage() const;
	const lcf::rpg::Skill& skill;
	const lcf::rpg::Item* item;
	bool treat_enemies_asif_in_front_row = false;
};

class Item : public AlgorithmBase {
public:
	Item(Game_Battler* source, Game_Battler* target, const lcf::rpg::Item& item);
	Item(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Item& item);
	Item(Game_Battler* source, const lcf::rpg::Item& item);

	bool IsTargetValid(const Game_Battler&) const override;
	bool vExecute() override;
	bool vStart() override;

	std::string GetStartMessage(int line) const override;
	int GetSourcePose() const override;
	int GetCBAMovement() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	bool ActionIsPossible() const override;

private:
	std::string GetFirstStartMessage() const;
	std::string GetSecondStartMessage() const;
	const lcf::rpg::Item& item;
};

class Defend : public AlgorithmBase {
public:
	Defend(Game_Battler* source);

	std::string GetStartMessage(int line) const override;
	int GetSourcePose() const override;
};

class Observe : public AlgorithmBase {
public:
	Observe(Game_Battler* source);

	std::string GetStartMessage(int line) const override;
};

class Charge : public AlgorithmBase {
public:
	Charge(Game_Battler* source);

	std::string GetStartMessage(int line) const override;
	void ApplyCustomEffect() override;
};

class SelfDestruct : public AlgorithmBase {
public:
	SelfDestruct(Game_Battler* source, Game_Party_Base* target);

	std::string GetStartMessage(int line) const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	bool vExecute() override;
	void ApplyCustomEffect() override;
private:
	bool animate = true;
};

class Escape : public AlgorithmBase {
public:
	Escape(Game_Battler* source);

	std::string GetStartMessage(int line) const override;
	int GetSourcePose() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	void ApplyCustomEffect() override;
};

class Transform : public AlgorithmBase {
public:
	Transform(Game_Battler* source, int new_monster_id);

	std::string GetStartMessage(int line) const override;
	void ApplyCustomEffect() override;

private:
	int new_monster_id;
};

// EnemyAi "Do Nothing" action. Handled slightly differently than None.
class DoNothing : public AlgorithmBase {
public:
	DoNothing(Game_Battler* source);
};

inline Type AlgorithmBase::GetType() const {
	return type;
}

inline const std::vector<StateEffect>& AlgorithmBase::GetStateEffects() const {
	return states;
}

inline Game_Party_Base* AlgorithmBase::GetOriginalPartyTarget() const {
	return party_target;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedHp() const {
	return hp;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSp() const {
	return sp;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAtk() const {
	return attack;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedDef() const {
	return defense;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSpi() const {
	return spirit;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAgi() const {
	return agility;
}

inline const std::vector<Game_BattleAlgorithm::AttributeEffect>& Game_BattleAlgorithm::AlgorithmBase::GetShiftedAttributes() const {
	return attributes;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSwitch() const {
	return switch_id;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsPositive() const {
	return GetFlag(ePositive);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsRevived() const {
	return GetFlag(eRevived);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::ActionIsPossible() const {
	return true;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAnimationId(int) const {
	return 0;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectHp() const {
	return GetFlag(eAffectHp);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorbHp() const {
	return GetFlag(eAbsorbHp);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectSp() const {
	return GetFlag(eAffectSp);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorbSp() const {
	return GetFlag(eAbsorbSp);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectAtk() const {
	return GetFlag(eAffectAtk);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorbAtk() const {
	return GetFlag(eAbsorbAtk);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectDef() const {
	return GetFlag(eAffectDef);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorbDef() const {
	return GetFlag(eAbsorbDef);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectSpi() const {
	return GetFlag(eAffectSpi);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorbSpi() const {
	return GetFlag(eAbsorbSpi);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectAgi() const {
	return GetFlag(eAffectAgi);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorbAgi() const {
	return GetFlag(eAbsorbAgi);
}


inline Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetReflectTarget() const {
	return reflect_target;
}

inline Span<Game_Battler* const> Game_BattleAlgorithm::AlgorithmBase::GetOriginalTargets() const {
	assert(num_original_targets <= static_cast<int>(targets.size()));
	return Span<Game_Battler* const>(targets.data(), num_original_targets);
}

inline Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetOriginalSingleTarget() const {
	assert(num_original_targets <= static_cast<int>(targets.size()));
	return (GetOriginalPartyTarget() == nullptr && num_original_targets == 1) ? targets.front() : nullptr;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedSwitch(int s) {
	return this->switch_id = s;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedHp(int hp) {
	SetFlag(eAffectHp, true);
	return this->hp = hp;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedSp(int sp) {
	SetFlag(eAffectSp, true);
	return this->sp = sp;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedAtk(int atk) {
	SetFlag(eAffectAtk, true);
	return this->attack = atk;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedDef(int def) {
	SetFlag(eAffectDef, true);
	return this->defense = def;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedSpi(int spi) {
	SetFlag(eAffectSpi, true);
	return this->spirit = spi;
}

inline int Game_BattleAlgorithm::AlgorithmBase::SetAffectedAgi(int agi) {
	SetFlag(eAffectAgi, true);
	return this->agility = agi;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsPositive(bool p) {
	return SetFlag(ePositive, p);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsCriticalHit(bool c) {
	return SetFlag(eCriticalHit, c);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsSuccess() {
	return SetFlag(eSuccess, true);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsSuccessIf(bool x) {
	return SetFlag(eSuccess, GetFlag(eSuccess) | x);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsFailure() {
	return SetFlag(eSuccess, false);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsSuccess() const {
	return GetFlag(eSuccess);
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsCriticalHit() const {
	return GetFlag(eCriticalHit);
}

inline Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetSource() const {
	return source;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetCurrentRepeat() const {
	return cur_repeat;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetTotalRepetitions() const {
	return repeat;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetFlag(Flag f, bool value) {
	flags.set(uint64_t(f), value);
	return value;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::GetFlag(Flag f) const {
	return flags.test(uint64_t(f));
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsAbsorbHp(bool a) {
	SetFlag(eAbsorbHp, a);
	return a;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsAbsorbSp(bool a) {
	SetFlag(eAbsorbSp, a);
	return a;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsAbsorbAtk(bool a) {
	SetFlag(eAbsorbAtk, a);
	return a;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsAbsorbDef(bool a) {
	SetFlag(eAbsorbDef, a);
	return a;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsAbsorbSpi(bool a) {
	SetFlag(eAbsorbSpi, a);
	return a;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::SetIsAbsorbAgi(bool a) {
	SetFlag(eAbsorbAgi, a);
	return a;
}

inline void Game_BattleAlgorithm::Normal::SetTreatEnemiesAsIfInFrontRow(bool v) {
	treat_enemies_asif_in_front_row = v;
}

inline bool Game_BattleAlgorithm::Normal::IsChargedAttack() const {
	return charged_attack;
}

inline const lcf::rpg::Skill& Game_BattleAlgorithm::Skill::GetSkill() const {
	return skill;
}

inline const lcf::rpg::Item* Game_BattleAlgorithm::Skill::GetItem() const {
	return item;
}

inline void Game_BattleAlgorithm::Skill::SetTreatEnemiesAsIfInFrontRow(bool v) {
	treat_enemies_asif_in_front_row = v;
}

} //namespace Game_BattleAlgorithm

#endif
