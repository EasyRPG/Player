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

#include <switch.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "player.h"

static int nxlinkSocket = -1;

int main(int argc, char* argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	appletLockExit();

	// yuzu/nso
	bool is_nro = envHasArgv();
	if(!is_nro) {
		// set arbitrary application path
		args.push_back("none:/easyrpg-player");
	}

	// Debug log over network
	if(is_nro && R_SUCCEEDED(socketInitializeDefault())) {
		nxlinkSocket = nxlinkStdio();
		if (nxlinkSocket < 0)
			socketExit();
	}

	romfsInit();

	// Check if romfs has some files inside or not
	if(::access("romfs:/RPG_RT.lmt", F_OK) == 0) {
		args.push_back("--project-path");
		args.push_back("romfs:/");

		// Use cwd as savepath, assuming each packaged game has an own directory
		// This will not work if somebody ever packages a game as nso
		if (is_nro) {
			char working_dir[256];
			getcwd(working_dir, 255);
			args.push_back("--save-path");
			args.push_back(working_dir);
		}
	}

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	romfsExit();

	// Close debug log
	if (nxlinkSocket >= 0) {
		close(nxlinkSocket);
		socketExit();
		nxlinkSocket = -1;
	}

	// HOS will close us immediately afterwards, if requested by home menu.
	// So no further cleanup possible.
	appletUnlockExit();
	return EXIT_SUCCESS;
}
