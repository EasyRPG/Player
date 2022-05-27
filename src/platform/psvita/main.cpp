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

#include <cstdio>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/appmgr.h>
#include "player.h"
#include "filefinder.h"

int _newlib_heap_size_user = 330 * 1024 * 1024;

int main(int argc, char* argv[]) {
	std::string psp2_dir = "ux0:/data/easyrpg-player";

	// Check if app is invoked with an externalized game path
	char boot_params[1024];
	sceAppMgrGetAppParam(boot_params);
	if (strstr(boot_params,"psgm:play") && strstr(boot_params, "&project-path=")) {
		psp2_dir = std::string(strstr(boot_params, "&project-path=") + 14);
	} else {
		// Check if app0 filesystem contains the title id reference file
		FILE* f = fopen("app0:/titleid.txt", "r");
		if (f != NULL) {
			char title_id[10];
			char save_dir[256];
			psp2_dir = "app0:/";

			fread(title_id, 1, 9, f);
			title_id[9] = 0;
			fclose(f);

			// Creating saves dir if it doesn't exist
			sprintf(save_dir, "ux0:/data/%s", title_id);
			sceIoMkdir(save_dir, 0777);
			auto savefs = FileFinder::Root().Create(save_dir);
			if (savefs)
				FileFinder::SetSaveFilesystem(savefs);
		}
	}

	auto gamefs = FileFinder::Root().Create(psp2_dir);
	if (gamefs)
		FileFinder::SetGameFilesystem(gamefs);

	// Run Player
	Player::Init(argc, argv);
	Player::Run();

	// Close
	sceKernelExitProcess(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
