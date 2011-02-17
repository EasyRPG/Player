/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GAME_ENEMY_H_
#define _GAME_ENEMY_H_

#include "game_battler.h"
class Game_Enemy :
	public Game_Battler
{
public:
	Game_Enemy(int enemy_id);

	const std::vector<int16_t>& GetStates() const;
	std::vector<int16_t>& GetStates();

	////////////////////////////////////////////////////////
	/// Gets the maximum hp for the current level
	/// @return max hp
	////////////////////////////////////////////////////////
	int GetBaseMaxHp() const;

	////////////////////////////////////////////////////////
	/// Gets the maximum sp for the current level
	/// @return max sp
	////////////////////////////////////////////////////////
	int GetBaseMaxSp() const;

	/// @return atk
	int GetBaseAtk() const;

	/// @return def
	int GetBaseDef() const;

	/// @return spi
	int GetBaseSpi() const;

	/// @return agi
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

