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

class Game_Enemy :
	public Game_Battler
{
public:
	Game_Enemy(int enemy_id);

	const std::vector<int16_t>& GetStates() const;
	std::vector<int16_t>& GetStates();

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

	int GetHp() const;
	void SetHp(int _hp);
	int GetSp() const;
	void SetSp(int _sp);

	void SetHidden(bool _hidden);
	bool IsHidden() const;
	void Transform(int new_enemy_id);

protected:
	void Setup(int enemy_id);

	int enemy_id;
	bool hidden;
	int hp;
	int sp;
	std::vector<int16_t> states;
};

#endif

