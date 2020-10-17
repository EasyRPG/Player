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

#include <string>
#include <vector>
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
	Null,
	Normal,
	Skill,
	Item,
	Defend,
	Observe,
	Charge,
	SelfDestruct,
	Escape,
	Transform,
	NoMove,
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
};

class AlgorithmBase {
public:
	virtual ~AlgorithmBase() {}

	/**
	 * Returns the source of the battle action.
	 *
	 * @return source battler
	 */
	Game_Battler* GetSource() const;

	/**
	 * Returns the current target.
	 *
	 * @return current target battler
	 */
	Game_Battler* GetTarget() const;

	/** @return If the action was reflected, returns the target which triggered the reflect */
	Game_Battler* GetReflectTarget() const;

	/** @return true if this algorithm targets a party */
	bool IsTargetingParty() const;

	/**
	 * Initializes targetting. Must be called after initialising a multi target
	 * skill, otherwise will behave incorrectly.
	 */
	void InitTargets();

	/**
	 * If IsReflectable(), will reflect the action back on the source or the source's party.
	 *
	 * @return true if this algo was reflected.
	 */
	bool ReflectTargets();

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

	/** @return true if this action affects hp. */
	bool IsAffectHp() const;

	/** @return true if this action affects sp. */
	bool IsAffectSp() const;

	/** @return true if this action affects atk. */
	bool IsAffectAtk() const;

	/** @return true if this action affects def. */
	bool IsAffectDef() const;

	/** @return true if this action affects spi. */
	bool IsAffectSpi() const;

	/** @return true if this action affects agi. */
	bool IsAffectAgi() const;

	/** @return signed value of how much hp is to be gained or lost */
	int GetAffectedHp() const;

	/** @return signed value of how much sp is to be gained or lost */
	int GetAffectedSp() const;

	/** @return signed value of how much attack is to be gained or lost */
	int GetAffectedAttack() const;

	/** @return signed value of how much defense is to be gained or lost */
	int GetAffectedDefense() const;

	/** @return signed value of how much spirit is to be gained or lost */
	int GetAffectedSpirit() const;

	/** @return signed value of how much spirit is to be gained or lost */
	int GetAffectedAgility() const;

	/** @return all states changes caused by this action in order. */
	const std::vector<StateEffect>& GetStateEffects() const;

	/** @return all attributes which are shifited by this action. */
	const std::vector<AttributeEffect>& GetShiftedAttributes() const;

	/**
	 * Gets activated switch.
	 *
	 * @return switch id or -1 when algorithm didn't affect a switch
	 */
	int GetAffectedSwitch() const;

	/**
	 * Gets whether the action was positive (e.g. healing) instead of damage.
	 *
	 * @return Whether action was positive
	 */
	bool IsPositive() const;

	/**
	 * Gets whether the action had absorb component.
	 *
	 * @return Whether action was absorb
	 */
	bool IsAbsorb() const;

	/*
	 * @return Whether target will be revived from death
	 */
	bool IsRevived() const;

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
	 * Returns whether the action hit the target.
	 * This function returns the same as the last Execute-call.
	 *
	 * @return if the action hit the target
	 */
	bool IsSuccess() const;

	/** @return if the last action was a critical hit.  */
	bool IsCriticalHit() const;

	/** @return if that is the first target of the action.  */
	bool IsFirstAttack() const;

	/**
	 * Executes the algorithm. Must be called before using the other functions.
	 * This function only simulates the Algorithm, call Apply to add the
	 * changes of the last Execute call to the target.
	 *
	 * @return true if the action was successful, false if failed/dodged
	 */
	virtual bool Execute() = 0;

	/** Apply the first time effect such as sp cost etc.. */
	void ApplyFirstTimeEffect();

	/** Apply hp damage or healing. Hp healing is not applied if the action revivies */
	int ApplyHpEffect();

	/** Apply sp increase or decrease. */
	int ApplySpEffect();

	/** Apply atk increase or decrease. */
	int ApplyAtkEffect();

	/** Apply def increase or decrease. */
	int ApplyDefEffect();

	/** Apply spi increase or decrease. */
	int ApplySpiEffect();

	/** Apply agi increase or decrease. */
	int ApplyAgiEffect();

	/** Apply the given state effect */
	void ApplyStateEffect(StateEffect se);

	/** Apply all state effects */
	void ApplyStateEffects();

	/** Apply the given attribute effect */
	int ApplyAttributeShiftEffect(AttributeEffect ae);

	/** Apply all attribute effects */
	void ApplyAttributeShiftEffects();

	/** Apply all effects in order */
	void ApplyAll();

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
	 * @return message
	 */
	virtual std::string GetStartMessage() const = 0;

	/**
	 * Checks if there is a first line message to display when the action is invoked.
	 *
	 * @return check
	 */
	bool HasStartMessage() const;

	/**
	 * Checks if there is a second line message to display when the action is invoked.
	 *
	 * @return check
	 */
	virtual bool HasSecondStartMessage() const;

	/**
	 * Gets the second line message that is displayed when the action is invoked.
	 * Usually of style "[Name] uses/casts [Weapon/Item/Skill]".
	 *
	 * @return message
	 */
	virtual std::string GetSecondStartMessage() const;

	/**
	 * Gets animation state id of the source character.
	 *
	 * @return animation state
	 */
	virtual int GetSourceAnimationState() const;

	/**
	* @return true if it is still possible to perform this action now.
	*/
	virtual bool ActionIsPossible() const;

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
	 * Gets the sound effect that is played when the target died.
	 *
	 * @return death se
	 */
	virtual const lcf::rpg::Sound* GetDeathSe() const;

	/**
	 * Returns the message used when the action fails.
	 *
	 * @return failure message
	 */
	virtual std::string GetFailureMessage() const;

	/**
	 * This is used to handle a corner case in the RPG2k battle system.
	 * When a battler dies because his hp reached 0 the "[NAME] has fallen"
	 * message is displayed on a new line. When the death is caused by a
	 * condition it is printed on the same line as all other conditions.
	 *
	 * @return death message
	 */
	virtual std::string GetDeathMessage() const;

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
	 * @return The significant restriction of the source when this
	 *      algorithm was created.
	 */
	lcf::rpg::State::Restriction GetSourceRestrictionWhenStarted() const;

	/**
	 * Set number of times to repeat the same action on a target
	 */
	void SetRepeat(int repeat);

	/**
	 * @return the critical hit message
	 */
	std::string GetCriticalHitMessage() const;

	std::string GetUndamagedMessage() const;
	std::string GetHpSpAbsorbedMessage(int value, StringView points) const;
	std::string GetDamagedMessage(int value) const;
	std::string GetHpSpRecoveredMessage(int value, StringView points) const;
	std::string GetParameterChangeMessage(int value, StringView points) const;
	std::string GetStateMessage(StringView message) const;
	std::string GetAttributeShiftMessage(int value, StringView attribute) const;

protected:
	AlgorithmBase(Type t, Game_Battler* source, Game_Battler* target);
	AlgorithmBase(Type t, Game_Battler* source, Game_Party_Base* target);
	virtual void vInitTargets();
	virtual void vApplyFirstTimeEffect();

	std::string GetAttackFailureMessage(StringView points) const;

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

	Type type = Type::Null;
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

private:
	bool first_attack = true;
protected:
	bool affect_hp = false;
	bool affect_sp = false;
	bool affect_atk = false;
	bool affect_def = false;
	bool affect_spi = false;
	bool affect_agi = false;
	bool positive = false;
	bool success = false;
	bool critical_hit = false;
	bool absorb = false;
	bool revived = false;
	bool physical_charged = false;
	lcf::rpg::State::Restriction source_restriction = lcf::rpg::State::Restriction_normal;
	int num_original_targets = 0;
	int cur_repeat = 0;
	int repeat = 1;

	std::vector<StateEffect> states;
	std::vector<AttributeEffect> attributes;
	std::vector<int> switch_on;
	std::vector<int> switch_off;
};

// Special algorithm for battlers which have no action. 
// Similar to NoMove, but treated as no action wheras NoMove
// is an action that does nothing.
class Null : public AlgorithmBase {
public:
	Null(Game_Battler* source);

	std::string GetStartMessage() const override;

	bool Execute() override;
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

	bool Execute() override;
	void vInitTargets() override;
	void vApplyFirstTimeEffect() override;

	int GetAnimationId(int i) const override;
	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	Game_Battler::Weapon GetWeapon() const;
private:
	void Init(int hits, Style style);
	int weapon_style = -1;
};

class Skill : public AlgorithmBase {
public:
	Skill(Game_Battler* source, Game_Battler* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);
	Skill(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);
	Skill(Game_Battler* source, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);

	bool IsTargetValid(const Game_Battler&) const override;
	bool Execute() override;
	void vApplyFirstTimeEffect() override;

	int GetAnimationId(int i) const override;
	std::string GetStartMessage() const override;
	bool HasSecondStartMessage() const override;
	std::string GetSecondStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	const lcf::rpg::Sound* GetFailureSe() const override;
	std::string GetFailureMessage() const override;
	bool IsReflected(const Game_Battler&) const override;
	bool ActionIsPossible() const override;

private:
	void Init();
	const lcf::rpg::Skill& skill;
	const lcf::rpg::Item* item;
};

class Item : public AlgorithmBase {
public:
	Item(Game_Battler* source, Game_Battler* target, const lcf::rpg::Item& item);
	Item(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Item& item);
	Item(Game_Battler* source, const lcf::rpg::Item& item);

	bool IsTargetValid(const Game_Battler&) const override;
	bool Execute() override;
	void vApplyFirstTimeEffect() override;

	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	bool ActionIsPossible() const override;

private:
	const lcf::rpg::Item& item;
};

class Defend : public AlgorithmBase {
public:
	Defend(Game_Battler* source);

	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	bool Execute() override;
};

class Observe : public AlgorithmBase {
public:
	Observe(Game_Battler* source);

	std::string GetStartMessage() const override;
	bool Execute() override;
};

class Charge : public AlgorithmBase {
public:
	Charge(Game_Battler* source);

	std::string GetStartMessage() const override;
	bool Execute() override;
	void vApplyFirstTimeEffect() override;
};

class SelfDestruct : public AlgorithmBase {
public:
	SelfDestruct(Game_Battler* source, Game_Party_Base* target);

	std::string GetStartMessage() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	bool Execute() override;
	void vApplyFirstTimeEffect() override;
};

class Escape : public AlgorithmBase {
public:
	Escape(Game_Battler* source);

	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	bool Execute() override;
	void vApplyFirstTimeEffect() override;
};

class Transform : public AlgorithmBase {
public:
	Transform(Game_Battler* source, int new_monster_id);

	std::string GetStartMessage() const override;
	bool Execute() override;
	void vApplyFirstTimeEffect() override;

private:
	int new_monster_id;
};

// Special algorithm for handling non-moving because of states
class NoMove : public AlgorithmBase {
public:
	NoMove(Game_Battler* source);

	std::string GetStartMessage() const override;

	bool Execute() override;
};


inline Type AlgorithmBase::GetType() const {
	return type;
}

inline bool AlgorithmBase::HasStartMessage() const {
	return !GetStartMessage().empty();
}

inline const std::vector<StateEffect>& AlgorithmBase::GetStateEffects() const {
	return states;
}

inline bool AlgorithmBase::IsTargetingParty() const {
	return party_target != nullptr;
}

inline bool AlgorithmBase::IsFirstAttack() const {
	return first_attack;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedHp() const {
	return hp;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSp() const {
	return sp;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAttack() const {
	return attack;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedDefense() const {
	return defense;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSpirit() const {
	return spirit;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAgility() const {
	return agility;
}

inline const std::vector<Game_BattleAlgorithm::AttributeEffect>& Game_BattleAlgorithm::AlgorithmBase::GetShiftedAttributes() const {
	return attributes;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSwitch() const {
	return switch_id;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsPositive() const {
	return positive;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorb() const {
	return absorb;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsRevived() const {
	return revived;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::ActionIsPossible() const {
	return true;
}

inline int Game_BattleAlgorithm::AlgorithmBase::GetAnimationId(int) const {
	return 0;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectHp() const {
	return affect_hp;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectSp() const {
	return affect_sp;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectAtk() const {
	return affect_atk;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectDef() const {
	return affect_def;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectSpi() const {
	return affect_spi;
}

inline bool Game_BattleAlgorithm::AlgorithmBase::IsAffectAgi() const {
	return affect_agi;
}

inline Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetReflectTarget() const {
	return reflect_target;
}

} //namespace Game_BattleAlgorithm

#endif
