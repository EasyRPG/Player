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

#ifndef _GAME_ENEMY_H_
#define _GAME_ENEMY_H_

// Headers
#include "game_battler.h"
#include "rpg_enemy.h"

/**
 * Represents a single enemy in the battle scene
 */
class Game_Enemy : public Game_Battler
{
public:
	Game_Enemy(int enemy_id);

	const std::vector<int16_t>& GetStates() const;
	std::vector<int16_t>& GetStates();

	/**
	 * Gets probability that a state can be inflicted on this actor.
	 *
	 * @param state_id State to test
	 * @return Probability of state infliction
	 */
	int GetStateProbability(int state_id);

	/**
	 * Gets the characters name
	 *
	 * @return Character name
	 */
	const std::string& GetName() const;

	/**
	 * Gets the filename of the enemy sprite
	 *
	 * @return Filename of enemy sprite
	 */
	const std::string& GetSpriteName() const;

	/**
	 * Gets the maximum HP for the current level.
	 *
	 * @return maximum HP.
	 */
	int GetBaseMaxHp() const;

	/**
	 * Gets the maximum SP for the current level.
	 *
	 * @return maximum SP.
	 */
	int GetBaseMaxSp() const;

	/**
	 * Gets the attack for the current level.
	 *
	 * @return attack.
	 */
	int GetBaseAtk() const;

	/**
	 * Gets the defense for the current level.
	 *
	 * @return defense.
	 */
	int GetBaseDef() const;

	/**
	 * Gets the spirit for the current level.
	 *
	 * @return spirit.
	 */
	int GetBaseSpi() const;

	/**
	 * Gets the agility for the current level.
	 *
	 * @return agility.
	 */
	int GetBaseAgi() const;

	/**
	 * Gets enemy X position
	 *
	 * @return enemy X position
	 */
	int GetBattleX() const;
	/**
	 * Gets enemy Y position
	 *
	 * @return enemy Y position
	 */
	int GetBattleY() const;

	/**
	 * Sets enemy X position
	 * 
	 * @param new_x New X position
	 */
	void SetBattleX(int new_x);

	/**
	 * Sets enemy Y position
	 * 
	 * @param new_y New Y position
	 */
	void SetBattleY(int new_y);

	int GetHue() const;

	int GetHp() const;
	void SetHp(int _hp);
	void ChangeHp(int hp);

	int GetSp() const;
	void SetSp(int _sp);

	bool Exists() const;
	void SetHidden(bool _hidden);
	bool IsHidden() const;
	void SetRemoved(bool _removed);
	bool IsRemoved() const;
	void Transform(int new_enemy_id);

	int GetCriticalHitChance() const;
	int GetBattleAnimationId() const;

	int GetExp() const;

	int GetMoney() const;

	/**
	 * Get's the ID of the item the enemy drops when defeated.
	 *
	 * @return Dropped item ID, or 0 if no drop
	 */
	int GetDropId() const;

	/**
	 * Get's the probability that the enemy's item will be
	 * dropped.
	 *
	 * @return Probability of drop as a percent (0-100)
	 */
	int GetDropProbability() const;

	BattlerType GetType() const;

	bool IsActionValid(const RPG::EnemyAction& action);
	const RPG::EnemyAction* ChooseRandomAction();

protected:
	void Setup(int enemy_id);

	int x;
	int y;

	int enemy_id;
	// hidden at battle begin
	bool hidden;
	// removed during battle via escape/self destruct
	bool removed;
	int hp;
	int sp;
	std::vector<int16_t> states;

	RPG::Enemy* enemy;

	// normal attack instance for use after charge
	RPG::EnemyAction normal_atk;
};

#endif

