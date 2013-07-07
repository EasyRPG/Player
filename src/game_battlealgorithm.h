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
#include <boost/optional/optional.hpp>

class Game_Actor;
class Game_Battler;
namespace RPG {
	class Animation;
	class Item;
	class State;
	class Skill;
}

/**
 * Contains algorithms to handle the different battle attacks, skills and items.
 * The algorithms are only for single targeting, when a party is affected it
 * must be called for each party member.
 *
 * The action is only simulated and the results can be applied after the
 * simulation by calling Apply.
 */
namespace Game_BattleAlgorithm {

class AlgorithmBase {
public:
	const Game_Battler* GetSource() const;

	const Game_Battler* GetTarget() const;

	bool TargetNext();

	/**
	 * Gets how many Hp were gained/lost.
	 *
	 * @return Hp change or empty element when algorithm didn't affect Hp
	 */
	boost::optional<int> GetAffectedHp() const;

	/**
	 * Gets how many Sp were gained/lost.
	 *
	 * @return Sp change or empty element when algorithm didn't affect Sp
	 */
	boost::optional<int> GetAffectedSp() const;

	/**
	 * Gets how many Attack was gained/lost.
	 *
	 * @return Attack change or empty element when algorithm didn't affect Attack
	 */
	boost::optional<int> GetAffectedAttack() const;

	/**
	 * Gets how many Defense was gained/lost.
	 *
	 * @return Defense change or empty element when algorithm didn't affect Defense
	 */
	boost::optional<int> GetAffectedDefense() const;

	/**
	 * Gets how many Spirit was gained/lost.
	 *
	 * @return Defense change or empty element when algorithm didn't affect Spirit
	 */
	boost::optional<int> GetAffectedSpirit() const;

	/**
	 * Gets how many Agility was gained/lost.
	 *
	 * @return Defense change or empty element when algorithm didn't affect Agility
	 */
	boost::optional<int> GetAffectedAgility() const;

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
	bool GetSuccess() const;

	/**
	 * See GetDeathMessage for further explanations
	 *
	 * @return True when the caller died because his hp reached 0 (false when a condition caused death)
	 */
	bool GetKilledByAttack() const;

	/**
	 * Gets if the last action was a critical hit.
	 */
	bool GetCriticalHit() const;

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
	 * Gets the message that is displayed when the action is invoked.
	 * Usually of style "[Name] uses/casts [Weapon/Item/Skill]".
	 *
	 * @return message
	 */
	virtual std::string GetStartMessage() const = 0;

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
	 *
	 * @param out filled with all conditions in text form
	 */
	virtual void GetResultMessages(std::vector<std::string>& out) const;

protected:
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
};

class Skill : public AlgorithmBase {
public:
	Skill(Game_Battler* source, Game_Battler* target, RPG::Skill& skill);
	Skill(Game_Battler* source, Game_Party_Base* target, RPG::Skill& skill);

	bool Execute();
	void Apply();

	std::string GetStartMessage() const;
	void GetResultMessages(std::vector<std::string>& out) const;

private:
	RPG::Skill& skill;
};

class Item : public AlgorithmBase {
public:
	Item(Game_Battler* source, Game_Battler* target, RPG::Item& item);
	Item(Game_Battler* source, Game_Party_Base* target, RPG::Item& item);

	bool Execute();
	void Apply();

	std::string GetStartMessage() const;
	void GetResultMessages(std::vector<std::string>& out) const;

private:
	RPG::Item& item;
};

}



#endif
