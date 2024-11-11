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
#include "output.h"

// For sdl-wii based port main is wrapped
#ifdef USE_SDL
#  define main SDL_main
#endif

namespace {
	// Debugging
	bool has_usbgecko = false;
	bool is_emu = false;
}

// in sdl-wii
extern "C" void OGC_ChangeSquare(int xscale, int yscale, int xshift, int yshift);

static void GekkoResetCallback(u32 /* irq */ , void* /* ctx */) {
	Player::reset_flag = true;
}

static void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
	std::string prefix = Output::LogLevelToString(lvl);

	if (is_emu) {
		std::string m = std::string("[" GAME_TITLE "] ") + prefix + ": " + msg;
		// maximum size is 256
		if(m.size() > 254) {
			m = m.substr(0, 251) + "...";
		}

		// Write to OSReport uart in dolphin emulator
		SYS_Report("%s\n", m.c_str());
	}

	if(has_usbgecko) {
		printf("%s: %s\n", prefix.c_str(), msg.c_str());
	}
}

extern "C" int main(int argc, char* argv[]) {
	// save cmdline
	std::vector<std::string> args(argv, argv + argc);

	// dolphin support
	is_emu = argc == 0;

	// Enable USBGecko output
	has_usbgecko = usb_isgeckoalive(CARD_SLOTB);
	if(has_usbgecko) {
		CON_EnableGecko(CARD_SLOTB, true);
	}
	Output::SetLogCallback(LogCallback);

	if(is_emu) {
		Output::Debug("Dolphin Emulator detected.");
		// set arbitrary application path
		args.push_back("/easyrpg-player");
	}

	SYS_SetResetCallback(GekkoResetCallback);

	// Eliminate overscan / add 5% borders
	OGC_ChangeSquare(304, 228, 0, 0);

	// Working directory not correctly handled, provide it manually
	bool want_cwd = true;
	if(is_emu || argv[0][0] == '/') {
		want_cwd = false;
	}
	// Check if a game directory was provided
	if (std::any_of(args.cbegin(), args.cend(),
		[](const std::string& a) { return a == "--project-path"; })) {
		want_cwd = false;
	}

	if (want_cwd) {
		char working_dir[256];
		getcwd(working_dir, 255);
		args.push_back("--project-path");
		args.push_back(working_dir);
	} else {
		Output::Debug("Changing to default directory.");
		chdir("/apps/easyrpg");
	}

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	// Close
	return Player::exit_code;
}
