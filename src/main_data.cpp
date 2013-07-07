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

// Headers
#include <boost/scoped_ptr.hpp>
#include <cstdlib>
#include "main_data.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "game_player.h"
#include "game_screen.h"
#include "game_map.h"
#include "game_variables.h"
#include "game_switches.h"
#include "font.h"

// Global variables.

Game_Variables_Class Game_Variables(Main_Data::game_data.system.variables);
Game_Switches_Class Game_Switches(Main_Data::game_data.system.switches);

namespace Main_Data {
	// Dynamic Game Data
	boost::scoped_ptr<Game_Screen> game_screen;
	boost::scoped_ptr<Game_Player> game_player;
	boost::scoped_ptr<Game_Party> game_party;
	boost::scoped_ptr<Game_EnemyParty> game_enemyparty;

	RPG::Save game_data;
	std::string project_path =
		getenv("RPG_TEST_GAME_PATH")? getenv("RPG_TEST_GAME_PATH"):
		getenv("RPG_GAME_PATH")? getenv("RPG_GAME_PATH"):
		".";
}

void Main_Data::Cleanup() {
	Game_Map::Quit();
	Game_Actors::Dispose();
	Font::Dispose();

	game_screen.reset();
	game_player.reset();
	game_party.reset();
	game_enemyparty.reset();
}
