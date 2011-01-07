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

#ifndef _MAIN_DATA_H_
#define _MAIN_DATA_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "data.h"
#include "game_screen.h"

////////////////////////////////////////////////////////////
/// Main Data namespace
////////////////////////////////////////////////////////////
class Game_Player;
class Game_Screen;
class Game_Troop;

namespace Main_Data {
	// Dynamic Game Data
	extern Game_Screen* game_screen;
	extern Game_Player* game_player;
	extern Game_Troop* game_troop;
	extern Game_Player* game_player;
	
	void Cleanup();
}

#endif
