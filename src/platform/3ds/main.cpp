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
#include <unistd.h>

// 8 MB required for booting and need extra linear memory for the sound
// effect cache
u32 __ctru_linear_heap_size = 12*1024*1024;

static u32 old_time_limit;

int main(int argc, char* argv[]) {
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

			// Creating dirs if they don't exist
			FS_Archive archive;
			FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, {PATH_EMPTY, 1, (u8*)""});
			FS_Path filePath = fsMakePath(PATH_ASCII, "/easyrpg-player");
			FSUSER_CreateDirectory(archive,filePath, FS_ATTRIBUTE_DIRECTORY);
			FS_Path filePath2 = fsMakePath(PATH_ASCII, &tmp_path[5]);
			FSUSER_CreateDirectory(archive, filePath2, FS_ATTRIBUTE_DIRECTORY);
			FSUSER_CloseArchive(archive);
		}

		auto savefs = FileFinder::Root().Create(tmp_path);
		if (savefs)
			FileFinder::SetSaveFilesystem(savefs);
	} else if(is_cia) {
		// No RomFS -> load games from hardcoded path
		ctr_dir = tmp_path;
	}
	auto gamefs = FileFinder::Root().Create(ctr_dir);
	if (gamefs)
		FileFinder::SetGameFilesystem(gamefs);

	// Run Player
	Player::Init(argc, argv);
	Player::Run();

	romfsExit();
	gfxExit();

	if (old_time_limit != UINT32_MAX) {
		APT_SetAppCpuTimeLimit(old_time_limit);
	}

	return EXIT_SUCCESS;
}
