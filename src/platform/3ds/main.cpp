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

#include <3ds.h>
#include <cstdio>

#include "player.h"
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>

/* 8 MB required for booting and need extra linear memory for the sound
 * effect cache and frame buffers
 */
u32 __ctru_linear_heap_size = 12*1024*1024;
static u32 old_time_limit;

int main(int argc, char* argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	// cia/citra
	if(!envIsHomebrew()) {
		// set arbitrary application path
		args.push_back("none:/easyrpg-player");
	}

	APT_GetAppCpuTimeLimit(&old_time_limit);
	APT_SetAppCpuTimeLimit(30);

	// Enable 804 Mhz mode if on N3DS
	bool isN3DS;
	APT_CheckNew3DS(&isN3DS);
	if (isN3DS) {
		osSetSpeedupEnable(true);
	}

	gfxInitDefault();
#ifdef _DEBUG
	consoleInit(GFX_BOTTOM, nullptr);
#endif
	romfsInit();

	bool is_cia = argc == 0;
	char tmp_path[64] = "sdmc:/3ds/easyrpg-player";
	std::string ctr_dir;

	// Check if romfs has some files inside or not
	if(::access("romfs:/RPG_RT.lmt", F_OK) == 0) {
		ctr_dir = "romfs:/";

		if (is_cia) {
			// CIA savepath is unique for any ID
			u64 title_id;
			APT_GetProgramID(&title_id);
			sprintf(tmp_path, "sdmc:/easyrpg-player/%016llX", title_id);

			// Create dirs if they don't exist
			mkdir("sdmc:/easyrpg-player", 0777);
			mkdir(tmp_path, 0777);
		}

		args.push_back("--save-path");
		args.push_back(tmp_path);
	} else if(is_cia) {
		// No RomFS -> load games from hardcoded path
		ctr_dir = tmp_path;
	}
	// otherwise uses cwd by default or 3dslink argument
	if (!ctr_dir.empty()) {
		args.push_back("--project-path");
		args.push_back(ctr_dir);
	}

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	romfsExit();
	gfxExit();

	if (old_time_limit != UINT32_MAX) {
		APT_SetAppCpuTimeLimit(old_time_limit);
	}

	return EXIT_SUCCESS;
}
