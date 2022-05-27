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
#include "player.h"

static int nxlinkSocket = -1;

int main(int argc, char* argv[]) {
	appletLockExit();

	// Debug log over network
	if(R_SUCCEEDED(socketInitializeDefault())) {
		nxlinkSocket = nxlinkStdio();
		if (nxlinkSocket < 0)
			socketExit();
	}

	romfsInit();

	char working_dir[256];
	getcwd(working_dir, 255);
	auto gamefs = FileFinder::Root().Create(working_dir);
	if (gamefs)
		FileFinder::SetGameFilesystem(gamefs);

	// Run Player
	Player::Init(argc, argv);
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
