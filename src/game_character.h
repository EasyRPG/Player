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

#ifndef _GAME_CHARACTER_H_
#define _GAME_CHARACTER_H_

#include <string>

////////////////////////////////////////////////////////////
/// Game_Character class.
////////////////////////////////////////////////////////////
class Game_Character {
public:
	Game_Character();
	virtual ~Game_Character();

	virtual bool IsMoving();
	virtual bool IsPassable(int x, int y, int d);

	virtual void MoveTo(int x, int y);

	virtual int GetScreenX();
	virtual int GetScreenY();
	virtual int GetScreenZ();
	virtual int GetScreenZ(int height);

	virtual void Update();
	virtual void UpdateMove();

	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void MoveUp();

	void Lock();

	std::string character_name;
	int character_hue;

	int id;
	int x;
	int y;
	int real_x;
	int real_y;
	int move_speed;

	//////////////
	//looking to:
	//	left -> 4
	//	up   -> 8
	//	down -> 2
	//	right-> 6
	int direction;

	int opacity;
	int blend_type;

	bool move_route_forcing;

private:
	int prelock_direction;
	bool locked;
};

#endif
