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

#ifndef _EASYRPG_GAME_BATTLECOMMAND_H_
#define _EASYRPG_GAME_BATTLECOMMAND_H_

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
 * The action is only simulated, the callee must apply the results.
 */
namespace Game_BattleAlgorithm {

class AlgorithmBase {
public:
	boost::optional<int> GetAffectedHp() const;
	boost::optional<int> GetAffectedSp() const;
	boost::optional<int> GetAffectedAttack() const;
	boost::optional<int> GetAffectedDefense() const;
	boost::optional<int> GetAffectedSpirit() const;
	boost::optional<int> GetAffectedAgility() const;
	const RPG::Animation* GetAnimation() const;

	const std::vector<RPG::State>& GetAffectedConditions() const;

	/**
	 * Executes the algorithm, results can be obtained using the getters.
	 *
	 * @return true if the action was successful, false if failed/dodged
	 */
	virtual bool Execute() = 0;

protected:
	AlgorithmBase(Game_Battler* source, Game_Battler* target);

	Game_Battler* source;
	Game_Battler* target;

	int hp;
	int sp;
	int attack;
	int defense;
	int spirit;
	int agility;

	RPG::Animation* animation;

	std::vector<RPG::State> conditions;
};

class Normal : public AlgorithmBase {
public:
	Normal(Game_Battler* source, Game_Battler* target);

	bool Execute();
};

class Skill : public AlgorithmBase {
public:
	Skill(Game_Battler* source, Game_Battler* target, RPG::Skill& skill);

	bool Execute();

private:
	RPG::Skill& skill;
};

class Item : public AlgorithmBase {
public:
	Item(Game_Battler* source, Game_Battler* target, RPG::Item& item);

	bool Execute();

private:
	RPG::Item& item;
};

}



#endif
