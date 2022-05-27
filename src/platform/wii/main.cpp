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

// USBGecko Debugging
bool usbgecko = false;
mutex_t usbgecko_mutex = 0;

// in sdl-wii
extern "C" void OGC_ChangeSquare(int xscale, int yscale, int xshift, int yshift);

static void GekkoResetCallback(u32 /* irq */ , void* /* ctx */) {
	Player::reset_flag = true;
}

extern "C" int main(int argc, char* argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	// dolphin
	bool is_emu = argc == 0;
	if(is_emu) {
		// set arbitrary application path
		args.push_back("/easyrpg-player");
	}

	SYS_SetResetCallback(GekkoResetCallback);

	// Eliminate overscan / add 5% borders
	OGC_ChangeSquare(304, 228, 0, 0);

	// Check if a game directory was provided
	if (std::none_of(args.cbegin(), args.cend(),
		[](const std::string& a) { return a == "--project-path"; })) {

		// Working directory not correctly handled, provide it manually
		char working_dir[256];
		getcwd(working_dir, 255);
		args.push_back("--project-path");
		args.push_back(working_dir);
	}

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	return EXIT_SUCCESS;
}

static ssize_t __usbgecko_write(struct _reent * /* r */, void* /* fd */, const char *ptr, size_t len) {
	uint32_t level;

	if (!ptr || !len || !usbgecko)
		return 0;

	LWP_MutexLock(usbgecko_mutex);
	level = IRQ_Disable();
	usb_sendbuffer(1, ptr, len);
	IRQ_Restore(level);
	LWP_MutexUnlock(usbgecko_mutex);

	return len;
}

const devoptab_t dotab_geckoout = {
	"stdout", 0, NULL, NULL, __usbgecko_write, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL
};

extern const devoptab_t dotab_stdnull;

void Wii::SetConsole() {
	LWP_MutexInit(&usbgecko_mutex, false);
	usbgecko = usb_isgeckoalive(1);

	if (usbgecko) {
		devoptab_list[STD_OUT] = &dotab_geckoout;
		devoptab_list[STD_ERR] = &dotab_geckoout;
	} else {
		devoptab_list[STD_OUT] = &dotab_stdnull;
		devoptab_list[STD_ERR] = &dotab_stdnull;
	}
}

bool Wii::LogMessage(const std::string &message) {
	if (usbgecko) return false;

	std::string m = std::string("[" GAME_TITLE "] ") + message + "\n";

	// HLE in dolphin emulator
	printf("%s", m.c_str());

	// additional usbgecko output not needed
	return true;
}
