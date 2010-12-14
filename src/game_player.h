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

#ifndef _GAME_PLAYER_H_
#define _GAME_PLAYER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "game_character.h"
#include <vector>

////////////////////////////////////////////////////////////
/// Game Player class
////////////////////////////////////////////////////////////
class Game_Player : public Game_Character {
public:
	Game_Player();
	~Game_Player();

	bool IsPassable(int x, int y, int d) const;
	bool IsTeleporting() const;
	void ReserveTeleport(int map_id, int x, int y);
	void PerformTeleport();
	void Center(int x, int y);
	void MoveTo(int x, int y);
	void Update();

	void Refresh();

	bool CheckEventTriggerTouch(int x, int y);

private:
	bool teleporting;
	int new_map_id, new_x, new_y;

	void UpdateScroll(int last_real_x, int last_real_y);
	void UpdateNonMoving(bool last_moving);
	bool CheckTouchEvent();
	bool CheckActionEvent();
	bool CheckEventTriggerHere(const std::vector<int>& triggers);
	bool CheckEventTriggerThere(const std::vector<int>& triggers);
};

#endif
