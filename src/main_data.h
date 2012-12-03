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
#include "rpg_save.h"
#include "game_screen.h"
#include <string>
#include <boost/scoped_ptr.hpp>

////////////////////////////////////////////////////////////
/// Main Data namespace
////////////////////////////////////////////////////////////
class Game_Player;
class Game_Screen;

namespace Main_Data {
	// Dynamic Game Data
	extern boost::scoped_ptr<Game_Screen> game_screen;
	extern boost::scoped_ptr<Game_Player> game_player;
	extern RPG::Save game_data;
	extern std::string project_path;

	void Cleanup();
}

#endif
