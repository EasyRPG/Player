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

#ifdef __ANDROID__
	#include <jni.h>
	#include <SDL_system.h>
#endif

#ifdef GEKKO
	#include <unistd.h>
#endif

// Global variables.

Game_Variables_Class Game_Variables(Main_Data::game_data.system.variables);
Game_Switches_Class Game_Switches(Main_Data::game_data.system.switches);

std::string project_path;
std::string save_path;

namespace Main_Data {
	// Dynamic Game Data
	boost::scoped_ptr<Game_Screen> game_screen;
	boost::scoped_ptr<Game_Player> game_player;
	boost::scoped_ptr<Game_Party> game_party;
	boost::scoped_ptr<Game_EnemyParty> game_enemyparty;

	RPG::Save game_data;
}

void Main_Data::Init() {
	if (project_path.empty()) {
		project_path =
			getenv("RPG_TEST_GAME_PATH") ? getenv("RPG_TEST_GAME_PATH") :
			getenv("RPG_GAME_PATH") ? getenv("RPG_GAME_PATH") :
			"";

		if (project_path.empty()) {
#ifdef GEKKO
			// Working directory not correctly handled under Wii
			char gekko_dir[256];
			getcwd(gekko_dir, 255);
			project_path = std::string(gekko_dir);
#else
			project_path = ".";
#endif
		}

	}
}

void Main_Data::Cleanup() {
	Game_Map::Quit();
	Game_Actors::Dispose();

	game_screen.reset();
	game_player.reset();
	game_party.reset();
	game_enemyparty.reset();

	game_data = RPG::Save();
}

const std::string& Main_Data::GetProjectPath() {	
	return project_path;
}

void Main_Data::SetProjectPath(const std::string& path) {
	project_path = path;
}
	
const std::string& Main_Data::GetSavePath() {
	if (save_path.empty()) {
		return GetProjectPath();
	}
	
	return save_path;
}

void Main_Data::SetSavePath(const std::string& path) {
	save_path = path;
}
