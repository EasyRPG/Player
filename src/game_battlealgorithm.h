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
#include <lcf/rpg/state.h>

class Game_Battler;
class Game_Party_Base;
namespace lcf {
namespace rpg {
	class Animation;
	class Item;
	class State;
	class Skill;
	class Sound;
} // namespace rpg
} // namespace lcf

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

	/**
	 * Allows changing of the target.
	 * Purpose is to allow attacking someone else if the old target is
	 * already dead (don't call this when attacking a party).
	 * This function doesn't adjust the algorithm if new and old target types
	 * mismatch (ally instead of enemy e.g.).
	 *
	 * @param target new target
	 */
	void SetTarget(Game_Battler* target);

	/**
	 * Initializes targetting. Must be called after initialising a multi target
	 * skill, otherwise will behave incorrectly.
	 */
	void TargetFirst();

	/**
	 * Changes the target reference to the next target.
	 *
	 * @return true if there was a next target available
	 */
	bool TargetNext();

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

	/**
	 * Gets how many Hp were gained/lost.
	 *
	 * @return Hp change or -1 when algorithm didn't affect Hp
	 */
	int GetAffectedHp() const;

	/**
	 * Gets how many Sp were gained/lost.
	 *
	 * @return Sp change or -1 when algorithm didn't affect Sp
	 */
	int GetAffectedSp() const;

	/**
	 * Gets how many Attack was gained/lost.
	 *
	 * @return Attack change or -1 when algorithm didn't affect Attack
	 */
	int GetAffectedAttack() const;

	/**
	 * Gets how many Defense was gained/lost.
	 *
	 * @return Defense change or -1 when algorithm didn't affect Defense
	 */
	int GetAffectedDefense() const;

	/**
	 * Gets how many Spirit was gained/lost.
	 *
	 * @return Spirit change or -1 when algorithm didn't affect Spirit
	 */
	int GetAffectedSpirit() const;

	/**
	 * Gets how many Agility was gained/lost.
	 *
	 * @return Agility change or -1 when algorithm didn't affect Agility
	 */
	int GetAffectedAgility() const;

	/**
	 * Gets all states changes caused by this action in order.
	 *
	 * @return state effects
	 */
	const std::vector<StateEffect>& GetStateEffects() const;

	/**
	 * Gets which attributes were shifited.
	 *
	 * @return shifted attributes
	 */
	const std::vector<int16_t>& GetShiftedAttributes() const;

	/**
	 * Gets activated switch.
	 *
	 * @return switch id or -1 when algorithm didn't affect a switch
	 */
	int GetAffectedSwitch() const;

	/**
	 * Gets whether the action was positive (e.g. healing) instead of demage.
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
	 * @return Battle Animation or NULL if no animation is assigned
	 */
	const lcf::rpg::Animation* GetAnimation() const;
	const lcf::rpg::Animation* GetSecondAnimation() const;

	/**
	 * Checks if the animation has already played once
	 *
	 * @return Whether the animation played once
	 */
	bool HasAnimationPlayed() const;
	bool HasSecondAnimationPlayed() const;

	/**
	 * Plays the battle animation on the targets.
	 * Takes care of single- and multi-target animations.
	 * Must be called before calling TargetNext, otherwise the result will
	 * be incorrect.
	 *
	 * @param on_source Renders the animation on the source instead of the
	 *                  targets (required for reflect)
	 */
	void PlayAnimation(bool on_source = false);
	void PlaySecondAnimation(bool on_source = false);

	void PlaySoundAnimation(bool on_source = false, int cutoff = -1);

	/**
	 * Returns whether the action hit the target.
	 * This function returns the same as the last Execute-call.
	 *
	 * @return if the action hit the target
	 */
	bool IsSuccess() const;

	/**
	 * @return true when this action inflicts death on the target.
	 */
	bool IsLethal() const;

	/**
	 * @return true when the target was killed by damage.
	 */
	bool IsKilledByDamage() const;

	/**
	 * Gets if the last action was a critical hit.
	 */
	bool IsCriticalHit() const;

	/**
	 * Gets if that is the first target of the action.
	 */
	bool IsFirstAttack() const;

	/**
	 * Executes the algorithm. Must be called before using the other functions.
	 * This function only simulates the Algorithm, call Apply to add the
	 * changes of the last Execute call to the target.
	 *
	 * @return true if the action was successful, false if failed/dodged
	 */
	virtual bool Execute() = 0;

	/**
	 * Applies the results (last Execute-call) of the algorithm to the target.
	 */
	virtual void Apply();

	/**
	 * Tests if it makes sense to apply an action on the target.
	 * E.g. when it is dead.
	 *
	 * @return true if valid, in case of false another target should be selected.
	 */
	virtual bool IsTargetValid() const;

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
	 * Gets the sound effect that is played then the action took place.
	 *
	 * @return result se
	 */
	virtual const lcf::rpg::Sound* GetResultSe() const;

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
	 * Returns the physical rate of the attack.
	 *
	 * @return physical rate
	 */
	virtual int GetPhysicalDamageRate() const;

	/**
	 * Returns whether the attack is reflected to the source.
	 * This is automatically handled by the battle algorithm class and
	 * GetTarget will return the source instead.
	 * The only exception is PlayAnimation which must be controlled through
	 * an extra argument because a reflected skill renders both animations:
	 * First time on target, then second time on source.
	 *
	 * @return true when reflected
	 */
	virtual bool IsReflected() const;

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
	std::string GetHpSpAbsorbedMessage(int value, const std::string& points) const;
	std::string GetDamagedMessage() const;
	std::string GetHpSpRecoveredMessage(int value, const std::string& points) const;
	std::string GetParameterChangeMessage(bool is_positive, int value, const std::string& points) const;
	std::string GetStateMessage(const std::string& message) const;
	std::string GetAttributeShiftMessage(const std::string& attribute) const;

protected:
	AlgorithmBase(Type t, Game_Battler* source);
	AlgorithmBase(Type t, Game_Battler* source, Game_Battler* target);
	AlgorithmBase(Type t, Game_Battler* source, Game_Party_Base* target);

	std::string GetAttackFailureMessage(const std::string& points) const;

	void ApplyActionSwitches();

	void Reset();

	/**
	 * Implements logic of TargetNext but ignores reflect.
	 * Used by const-functions that restore the old state afterwards.
	 * So technically this function is non-const but due to the help of the
	 * other functions the behaviour to the callee is const...
	 *
	 * @return true if there was a next target available
	 */
	bool TargetNextInternal() const;

	Type type = Type::Null;
	Game_Battler* source = nullptr;
	std::vector<Game_Battler*> targets;
	mutable std::vector<Game_Battler*>::iterator current_target;
	Game_Party_Base* party_target = nullptr;

	bool no_target;

	int hp;
	int sp;
	int attack;
	int defense;
	int spirit;
	int agility;
	int switch_id;

	mutable bool first_attack;
	bool healing;
	bool success;
	bool lethal = false;
	bool killed_by_dmg = false;
	bool critical_hit;
	bool absorb;
	bool revived = false;
	bool physical_charged = false;
	mutable int reflect;
	lcf::rpg::State::Restriction source_restriction = lcf::rpg::State::Restriction_normal;
	int cur_repeat = 0;
	int repeat = 1;

	lcf::rpg::Animation* animation = nullptr;
	lcf::rpg::Animation* animation2 = nullptr;
	bool has_animation_played = false;
	bool has_animation2_played = false;

	std::vector<StateEffect> states;
	std::vector<int16_t> shift_attributes;
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
	void Apply() override;
};

class Normal : public AlgorithmBase {
public:
	Normal(Game_Battler* source, Game_Battler* target);
	Normal(Game_Battler* source, Game_Party_Base* target);

	bool Execute() override;
	void Apply() override;

	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	int GetPhysicalDamageRate() const override;
private:
	void Init();
};

class Skill : public AlgorithmBase {
public:
	Skill(Game_Battler* source, Game_Battler* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);
	Skill(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);
	Skill(Game_Battler* source, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item = NULL);

	bool IsTargetValid() const override;
	bool Execute() override;
	void Apply() override;

	std::string GetStartMessage() const override;
	bool HasSecondStartMessage() const override;
	std::string GetSecondStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	const lcf::rpg::Sound* GetFailureSe() const override;
	const lcf::rpg::Sound* GetResultSe() const override;
	std::string GetFailureMessage() const override;
	int GetPhysicalDamageRate() const override;
	bool IsReflected() const override;
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

	bool IsTargetValid() const override;
	bool Execute() override;
	void Apply() override;

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
	void Apply() override;
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
	void Apply() override;
};

class SelfDestruct : public AlgorithmBase {
public:
	SelfDestruct(Game_Battler* source, Game_Party_Base* target);

	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	int GetPhysicalDamageRate() const override;
	bool Execute() override;
	void Apply() override;
};

class Escape : public AlgorithmBase {
public:
	Escape(Game_Battler* source, bool always_succeed = false);

	std::string GetStartMessage() const override;
	int GetSourceAnimationState() const override;
	const lcf::rpg::Sound* GetStartSe() const override;
	bool Execute() override;
	void Apply() override;
private:
	bool always_succeed = false;
};

class Transform : public AlgorithmBase {
public:
	Transform(Game_Battler* source, int new_monster_id);

	std::string GetStartMessage() const override;
	bool Execute() override;
	void Apply() override;

private:
	int new_monster_id;
};

// Special algorithm for handling non-moving because of states
class NoMove : public AlgorithmBase {
public:
	NoMove(Game_Battler* source);

	std::string GetStartMessage() const override;

	bool Execute() override;
	void Apply() override;
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

} //namespace Game_BattleAlgorithm

#endif
