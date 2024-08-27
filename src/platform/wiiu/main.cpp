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

#include <cstdlib>
#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>
#include "player.h"
#include "utils.h"
#include "output.h"

#ifdef USE_SDL // SDL might wrap main()
#  include <SDL.h>
#endif

#include <coreinit/debug.h>
#include <coreinit/foreground.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>

using namespace std::chrono_literals;

static bool procUiExited = false;

#ifdef NDEBUG
// stubbed
static void initLogging() {}
static void deinitLogging() {}
#else
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>

static bool moduleLogInit = false;
static bool cafeLogInit = false;
static bool udpLogInit = false;

static void initLogging() {
	if (!(moduleLogInit = WHBLogModuleInit())) {
		cafeLogInit = WHBLogCafeInit();
		udpLogInit  = WHBLogUdpInit();
	}
}

static void deinitLogging() {
	if (moduleLogInit) {
		WHBLogModuleDeinit();
		moduleLogInit = false;
	}
	if (cafeLogInit) {
		WHBLogCafeDeinit();
		cafeLogInit = false;
	}
	if (udpLogInit) {
		WHBLogUdpDeinit();
		udpLogInit = false;
	}
}
#endif

static void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
	std::string m = std::string("[" GAME_TITLE "] ") +
		Output::LogLevelToString(lvl) + ": " + msg;

#ifdef NDEBUG
	OSReport("%s\n", m.c_str());
#else
	WHBLogPrintf("%s\n", m.c_str());
#endif
}

static uint32_t SaveCallback(void*) {
	OSSavesDone_ReadyToRelease();
	return 0;
}

bool WiiU_ProcessProcUI() {
	ProcUIStatus status = ProcUIProcessMessages(TRUE);
	if (status == PROCUI_STATUS_EXITING) {
		procUiExited = true;
		return false;
	} else if (status == PROCUI_STATUS_RELEASE_FOREGROUND) {
		ProcUIDrawDoneRelease();
	}
	return true;
}

void WiiU_Exit() {
	Output::Debug("Shutdown Reason: {}", procUiExited ? "HOME Menu" : "Player Exit");

    if (!procUiExited) {
		// Exit was not through the Home Menu
		// Manually launch the system menu
		SYSLaunchMenu();
		Game_Clock::SleepFor(10ms);
		while (WiiU_ProcessProcUI()) {}
	}

	ProcUIShutdown();

	deinitLogging();
}

/**
 * If the main function ever needs to change, be sure to update the `main()`
 * functions of the other platforms as well.
 */
extern "C" int main(int argc, char* argv[]) {
	std::vector<std::string> args{argv, argv + argc};

	initLogging();
	ProcUIInitEx(SaveCallback, nullptr);

	Output::SetLogCallback(LogCallback);

	const char *default_dir = "fs:/vol/external01/wiiu/apps/easyrpg-player";

	char working_dir[256];
	getcwd(working_dir, 255);

	// Check if wuhb has some files inside or not
	if(::access("fs:/vol/content/RPG_RT.lmt", F_OK) == 0) {
		Output::Debug("Running packaged game from wuhb.");
		std::string cafe_dir = "fs:/vol/content";

		args.push_back("--project-path");
		args.push_back(cafe_dir);

		// Save directory is where the wuhb is located
		args.push_back("--save-path");
		args.push_back(working_dir);
	} else if(::access(default_dir, F_OK) == 0) {
		chdir(default_dir);
	} else {
		// fall back to current working directory
		args.push_back("--project-path");
		args.push_back(working_dir);
	}

	// Setup teardown code
	atexit(WiiU_Exit);

	Player::Init(std::move(args));
	Player::Run();

	return Player::exit_code;
}
