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
#include "player.h"

#ifdef __ANDROID__
	#include <jni.h>
	#include <SDL_system.h>
#endif

#ifdef GEKKO
	#include <unistd.h>
#endif

#ifdef _3DS
	#include <3ds.h>
	#include "output.h"
	#include <stdio.h>
#endif

#ifdef PSP2
	#include <stdio.h>
	#include <psp2/io/stat.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
	#include <SDL.h>
	#include <unistd.h>
#endif

// Global variables.

Game_Variables_Class Game_Variables;
Game_Switches_Class Game_Switches;

std::string project_path;
std::string save_path;

namespace Main_Data {
	// Dynamic Game Data
	std::unique_ptr<Game_Screen> game_screen;
	std::unique_ptr<Game_Player> game_player;
	std::unique_ptr<Game_Party> game_party;
	std::unique_ptr<Game_EnemyParty> game_enemyparty;

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
#elif defined(PSP2)
			// Check if app0 filesystem contains the title id reference file
			FILE* f = fopen("app0:/titleid.txt","r");
			if (f == NULL) project_path = "ux0:/data/easyrpg-player";
			else{
				char titleID[10];
				char psp2_dir[256];
				fread(titleID, 1, 9, f);
				titleID[9] = 0;
				sprintf(psp2_dir, "ux0:/data/%s",titleID);
				fclose(f);
				project_path = "app0:";
				save_path = psp2_dir;

				// Creating saves dir if it doesn't exist
				sceIoMkdir(psp2_dir, 0777);

			}
#elif defined(_3DS)
#   ifndef CITRA3DS_COMPATIBLE
			// Check if romFs has some files inside or not
			FILE* testfile = fopen("romfs:/RPG_RT.lmt","r");
			if (testfile != NULL){
				Output::Debug("Detected a project on romFs filesystem...");
				fclose(testfile);
				project_path = "romfs:";
				save_path = ".";

				if (!Player::is_3dsx) {
					// Create savepath for CIA - unique for any ID

					// Generating save_path
					u64 titleID;
					APT_GetProgramID(&titleID);
					char mainDir[64];
					sprintf(mainDir,"sdmc:/easyrpg-player/%016llX",titleID);

					// Creating dirs if they don't exist
					FS_Archive archive;
					FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, {PATH_EMPTY, 1, (u8*)""});
					FS_Path filePath=fsMakePath(PATH_ASCII, "/easyrpg-player");
					FSUSER_CreateDirectory(archive,filePath, FS_ATTRIBUTE_DIRECTORY);
					FS_Path filePath2=fsMakePath(PATH_ASCII, &mainDir[5]);
					FSUSER_CreateDirectory(archive,filePath2, FS_ATTRIBUTE_DIRECTORY);
					FSUSER_CloseArchive(archive);

					save_path = mainDir;
				}
			} else if (!Player::is_3dsx) {
				// No RomFS -> load games from hardcoded path
				project_path = "sdmc:/3ds/easyrpg-player";
			} else {
				project_path = ".";
			}
#   else
			project_path = ".";
#   endif
#elif defined(__APPLE__) && defined(__MACH__)
#  if SDL_MAJOR_VERSION>1
			char* home = getenv("HOME");
			char current_dir[255] = { 0 };
			getcwd(current_dir, sizeof(current_dir));
			if (!strcmp(current_dir, home)) {
				// Apple Finder does not set the working directory
				// It points to HOME instead. When it is HOME change it to
				// the application directory instead

				// FIXME: Uses SDL API
				char* data_dir = SDL_GetBasePath();
				project_path = data_dir;

				free(data_dir);
			}
#  else
			project_path = ".";
#  endif
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
