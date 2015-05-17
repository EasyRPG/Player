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

#ifndef _EASYRPG_GAME_BATTLEALGORITHM_H_
#define _EASYRPG_GAME_BATTLEALGORITHM_H_

#include <string>
#include <vector>

class Game_Actor;
class Game_Battler;
class Game_Party_Base;
namespace RPG {
	class Animation;
	class Item;
	class State;
	class Skill;
	class Sound;
}

/**
 * Contains algorithms to handle the different battle attacks, skills and items.
 * The algorithms support single targets and party targets.
 * For party targets the caller is responsible for retargeting using TargetNext.
 *
 * The action is simulated using Execute and the results can be applied after the
 * simulation by calling Apply.
 */
namespace Game_BattleAlgorithm {

class AlgorithmBase {
public:
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
	 * Changes the target reference to the next target.
	 *
	 * @return true if there was a next target available
	 */
	bool TargetNext();

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
	 * Gets the Battle Animation that is assigned to the Algorithm
	 *
	 * @return Battle Animation or NULL if no animation is assigned
	 */
	const RPG::Animation* GetAnimation() const;

	/**
	 * Returns a list of all inflicted/removed conditions.
	 *
	 * @return List of all inflicted/removed conditions
	 */
	const std::vector<RPG::State>& GetAffectedConditions() const;

	/**
	 * Returns whether the action hit the target.
	 * This function returns the same as the last Execute-call.
	 *
	 * @return if the action hit the target
	 */
	bool IsSuccess() const;

	/**
	 * See GetDeathMessage for further explanations
	 *
	 * @return True when the caller died because his hp reached 0 (false when a condition caused death)
	 */
	bool IsKilledByAttack() const;

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
	virtual bool IsTargetValid();

	/**
	 * Gets the message that is displayed when the action is invoked.
	 * Usually of style "[Name] uses/casts [Weapon/Item/Skill]".
	 *
	 * @return message
	 */
	virtual std::string GetStartMessage() const = 0;

	/**
	 * Gets animation state id of the source character.
	 *
	 * @return animation state
	 */
	virtual int GetSourceAnimationState() const;

	/**
	 * Gets the sound effect that is played when the action is starting.
	 *
	 * @return start se
	 */
	virtual const RPG::Sound* GetStartSe() const;

	/**
	 * Gets the sound effect that is played then the action took place.
	 *
	 * @return result se
	 */
	virtual const RPG::Sound* GetResultSe() const;

	/**
	 * Gets the sound effect that is played when the target died.
	 *
	 * @return death se
	 */
	virtual const RPG::Sound* GetDeathSe() const;

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
	 * Returns all inflicted/healed conditions.
	 * This must be called before calling Execute, otherwise the conditions
	 * are reported incorrectly.
	 *
	 * @param out filled with all conditions in text form
	 */
	virtual void GetResultMessages(std::vector<std::string>& out) const;

protected:
	AlgorithmBase(Game_Battler* source);
	AlgorithmBase(Game_Battler* source, Game_Battler* target);
	AlgorithmBase(Game_Battler* source, Game_Party_Base* target);

	void Reset();

	Game_Battler* source;
	std::vector<Game_Battler*> targets;
	std::vector<Game_Battler*>::iterator current_target;

	int hp;
	int sp;
	int attack;
	int defense;
	int spirit;
	int agility;
	int switch_id;

	bool first_attack;
	bool healing;
	bool success;
	bool killed_by_attack_damage;
	bool critical_hit;

	RPG::Animation* animation;

	std::vector<RPG::State> conditions;
};

class Normal : public AlgorithmBase {
public:
	Normal(Game_Battler* source, Game_Battler* target);
	Normal(Game_Battler* source, Game_Party_Base* target);

	bool Execute();
	void Apply();

	std::string GetStartMessage() const;
	int GetSourceAnimationState() const;
	const RPG::Sound* GetStartSe() const;
};

class Skill : public AlgorithmBase {
public:
	Skill(Game_Battler* source, Game_Battler* target, const RPG::Skill& skill, const RPG::Item* item = NULL);
	Skill(Game_Battler* source, Game_Party_Base* target, const RPG::Skill& skill, const RPG::Item* item = NULL);
	Skill(Game_Battler* source, const RPG::Skill& skill, const RPG::Item* item = NULL);

	bool IsTargetValid();
	bool Execute();
	void Apply();

	std::string GetStartMessage() const;
	int GetSourceAnimationState() const;
	const RPG::Sound* GetStartSe() const;
	void GetResultMessages(std::vector<std::string>& out) const;

private:
	const RPG::Skill& skill;
	const RPG::Item* item;
};

class Item : public AlgorithmBase {
public:
	Item(Game_Battler* source, Game_Battler* target, const RPG::Item& item);
	Item(Game_Battler* source, Game_Party_Base* target, const RPG::Item& item);
	Item(Game_Battler* source, const RPG::Item& item);

	bool IsTargetValid();
	bool Execute();
	void Apply();

	std::string GetStartMessage() const;
	int GetSourceAnimationState() const;
	const RPG::Sound* GetStartSe() const;
	void GetResultMessages(std::vector<std::string>& out) const;

private:
	const RPG::Item& item;
};

class NormalDual : public AlgorithmBase {
public:
	NormalDual(Game_Battler* source, Game_Battler* target);

	std::string GetStartMessage() const;
	const RPG::Sound* GetStartSe() const;
	bool Execute();
};

class Defend : public AlgorithmBase {
public:
	Defend(Game_Battler* source);

	std::string GetStartMessage() const;
	int GetSourceAnimationState() const;
	bool Execute();
	void Apply();
};

class Observe : public AlgorithmBase {
public:
	Observe(Game_Battler* source);

	std::string GetStartMessage() const;
	bool Execute();
};

class Charge : public AlgorithmBase {
public:
	Charge(Game_Battler* source);

	std::string GetStartMessage() const;
	bool Execute();
	void Apply();
};

class SelfDestruct : public AlgorithmBase {
public:
	SelfDestruct(Game_Battler* source, Game_Party_Base* target);

	std::string GetStartMessage() const;
	int GetSourceAnimationState() const;
	const RPG::Sound* GetStartSe() const;
	bool Execute();
	void Apply();
};

class Escape : public AlgorithmBase {
public:
	Escape(Game_Battler* source);

	std::string GetStartMessage() const;
	int GetSourceAnimationState() const;
	const RPG::Sound* GetStartSe() const;
	bool Execute();
	void Apply();

	void GetResultMessages(std::vector<std::string>& out) const;
};

class Transform : public AlgorithmBase {
public:
	Transform(Game_Battler* source, int new_monster_id);

	std::string GetStartMessage() const;
	bool Execute();
	void Apply();

private:
	int new_monster_id;
};

}



#endif
