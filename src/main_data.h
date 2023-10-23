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

#ifndef EP_MAIN_DATA_H
#define EP_MAIN_DATA_H

// Headers
#include <lcf/data.h>
#include <string>
#include <memory>

/**
 * Main lcf::Data namespace.
 */
class Game_System;
class Game_Player;
class Game_Screen;
class Game_Pictures;
class Game_Windows;
class Game_Actors;
class Game_Party;
class Game_EnemyParty;
class Game_Switches;
class Game_Variables;
class Game_Strings;
class Game_Targets;
class Game_Quit;
class Game_Ineluki;
class FileFinder_RTP;

namespace Main_Data {
	// Dynamic Game lcf::Data
	extern std::unique_ptr<Game_System> game_system;
	extern std::unique_ptr<Game_Switches> game_switches;
	extern std::unique_ptr<Game_Variables> game_variables;
	extern std::unique_ptr<Game_Strings> game_strings;
	extern std::unique_ptr<Game_Screen> game_screen;
	extern std::unique_ptr<Game_Pictures> game_pictures;
	extern std::unique_ptr<Game_Windows> game_windows;
	extern std::unique_ptr<Game_Player> game_player;
	extern std::unique_ptr<Game_Actors> game_actors;
	extern std::unique_ptr<Game_Party> game_party;
	extern std::unique_ptr<Game_EnemyParty> game_enemyparty;
	extern std::unique_ptr<Game_Targets> game_targets;
	extern std::unique_ptr<Game_Quit> game_quit;
	extern std::unique_ptr<Game_Ineluki> game_ineluki;
	extern std::unique_ptr<Game_Switches> game_switches_global; // Used by Global Save command
	extern std::unique_ptr<Game_Variables> game_variables_global;


	extern std::unique_ptr<FileFinder_RTP> filefinder_rtp;

	void Init();
	void Cleanup();

	const std::string& GetDefaultProjectPath();
}

#endif
