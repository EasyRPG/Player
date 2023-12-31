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

#include "player.h"
#include <gccore.h>
#include <ogcsys.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <algorithm>
#include "baseui.h"
#include <sys/iosupport.h>
#include <wiiuse/wpad.h>
#include "main.h"

// Currently for sdl-wii based port is wrapped
#ifdef USE_SDL
#  define main SDL_main
#endif

namespace {
	// USBGecko Debugging
	bool usbgecko = false;

	bool is_emu = false;
}

// in sdl-wii
extern "C" void OGC_ChangeSquare(int xscale, int yscale, int xshift, int yshift);

static void GekkoResetCallback(u32 /* irq */ , void* /* ctx */) {
	Player::reset_flag = true;
}

extern "C" int main(int argc, char* argv[]) {
	// Enable USBGecko output
	CON_EnableGecko(CARD_SLOTB, true);
	usbgecko = usb_isgeckoalive(CARD_SLOTB);

	// cmdline
	std::vector<std::string> args(argv, argv + argc);

	// dolphin
	is_emu = argc == 0;
	if(is_emu) {
		// set arbitrary application path
		args.push_back("/easyrpg-player");
	}

	SYS_SetResetCallback(GekkoResetCallback);

	// Eliminate overscan / add 5% borders
	OGC_ChangeSquare(304, 228, 0, 0);

	if (is_emu || strchr(argv[0], '/') == 0) {
		Output::Debug("USBGecko/Dolphin mode, changing dir to default.");
		chdir("/apps/easyrpg");
	} else {
		// Check if a game directory was provided
		if (std::none_of(args.cbegin(), args.cend(),
			[](const std::string& a) { return a == "--project-path"; })) {

			// Working directory not correctly handled, provide it manually
			char working_dir[256];
			getcwd(working_dir, 255);
			args.push_back("--project-path");
			args.push_back(working_dir);
		}
	}

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	return EXIT_SUCCESS;
}

bool Wii::LogMessage(const std::string &message) {
	if (is_emu) {
		std::string m = std::string("[" GAME_TITLE "] ") + message + "\n";

		// Write to OSReport uart in dolphin emulator
		SYS_Report("%s", m.c_str());

		// additional usbgecko output not needed
		return true;
	}

	// let Output class write to USBGecko or eat message if not present
	return !usbgecko;
}
