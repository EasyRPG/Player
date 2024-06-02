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
#include <whb/proc.h>
#include <sysapp/launch.h>

using namespace std::chrono_literals;

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

/**
 * If the main function ever needs to change, be sure to update the `main()`
 * functions of the other platforms as well.
 */
extern "C" int main(int argc, char* argv[]) {
	std::vector<std::string> args{argv, argv + argc};

	initLogging();
	WHBProcInit();

	Output::SetLogCallback(LogCallback);

	const char *default_dir = "fs:/vol/external01/wiiu/apps/easyrpg-player";

	// Check if wuhb has some files inside or not
	if(::access("fs:/vol/content/RPG_RT.lmt", F_OK) == 0) {
		Output::Debug("Running packaged game from wuhb.");
		std::string cafe_dir = "fs:/vol/content";

		args.push_back("--project-path");
		args.push_back(cafe_dir);

		//args.push_back("--save-path");
		//args.push_back(cafe_dir);
	} else if(::access(default_dir, F_OK) == 0) {
		chdir(default_dir);
	} else {
		// fall back to current working directory
		char working_dir[256];
		getcwd(working_dir, 255);
		args.push_back("--project-path");
		args.push_back(working_dir);
	}

	Player::Init(std::move(args));
	Player::Run();

	// FIXME: somehow the wiiu sdl2 port does not clean up procui
	// without launching the menu manually
#if 1
	SYSLaunchMenu();
	while(WHBProcIsRunning()) {
		Output::Debug("Waiting for shutdown...");
		Game_Clock::SleepFor(1s);
	}
#else
	WHBProcShutdown();
#endif

	deinitLogging();

	return EXIT_SUCCESS;
}
