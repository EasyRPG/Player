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
#include <malloc.h>
#include <string>
#include <vector>
#include "lcf/scope_guard.h"
#include "output.h"

/* 8 MB required for booting and need extra linear memory for the sound
 * effect cache and frame buffers
 */
u32 __ctru_linear_heap_size = 22*1024*1024;

namespace {
	u32 old_time_limit;
	int n3dslinkSocket = -1;
	u32* n3dsSocBuf = nullptr;
	bool is_emu = false;

	constexpr int SOC_BUFFERSIZE = 0x100000;
}

static void start3DSLink() {
	// only do socket stuff, if sent by 3dslink
	if (!__3dslink_host.s_addr) return;

	n3dsSocBuf = (u32*)memalign(0x1000, SOC_BUFFERSIZE);
	if(n3dsSocBuf == nullptr) {
		printf("failed to allocate SOC buffer\n");
		return;
	}
	auto buff_sg = lcf::makeScopeGuard([&]() {
		free(n3dsSocBuf);
		n3dsSocBuf = nullptr;
	});

	int ret = socInit(n3dsSocBuf, SOC_BUFFERSIZE);
	if (ret != 0) {
		printf("socInit failed: 0x%08X\n", (unsigned int)ret);
		return;
	}
	auto serv_sg = lcf::makeScopeGuard([&]() {
		socExit();
	});

	n3dslinkSocket = link3dsStdio();
	if(n3dslinkSocket < 0) {
		printf("failed to link stdio\n");
		return;
	}

	serv_sg.Dismiss();
	buff_sg.Dismiss();
}

static void stop3DSLink() {
	if (n3dslinkSocket < 0) return;

	close(n3dslinkSocket);
	n3dslinkSocket = -1;
	socExit();
	free(n3dsSocBuf);
}

static void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
	std::string prefix = Output::LogLevelToString(lvl);

	if (is_emu) {
		std::string m = std::string("[" GAME_TITLE "] ") + prefix + ": " + msg + "\n";

		// HLE in citra emulator
		svcOutputDebugString(m.c_str(), m.length());
	}

	// additionally to 3dslink server or bottom console
	bool want_log = n3dslinkSocket >= 0;
#ifdef _DEBUG
	want_log = true;
#endif
	if (want_log) {
		printf("%s: %s\n", prefix.c_str(), msg.c_str());
	}
}

void n3dsExit() {
	romfsExit();
	stop3DSLink();
	gfxExit();

	if (old_time_limit != UINT32_MAX) {
		APT_SetAppCpuTimeLimit(old_time_limit);
	}
}

int main(int argc, char* argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	gfxInitDefault();
#ifdef _DEBUG
	consoleInit(GFX_BOTTOM, nullptr);
#endif
	start3DSLink();
	Output::SetLogCallback(LogCallback);

	APT_GetAppCpuTimeLimit(&old_time_limit);
	APT_SetAppCpuTimeLimit(30);

	// Enable 804 Mhz mode if on N3DS
	bool isN3DS;
	APT_CheckNew3DS(&isN3DS);
	if (isN3DS) {
		osSetSpeedupEnable(true);
	}

	// cia/citra
	is_emu = !envIsHomebrew();
	bool is_cia = argc == 0;
	if(is_emu) {
		Output::Debug("Running inside emulator or as CIA.");

		// set arbitrary application path
		args.push_back("none:/easyrpg-player");
	}
	romfsInit();

	char tmp_path[64] = "sdmc:/3ds/easyrpg-player";
	std::string ctr_dir;

	// Check if romfs has some files inside or not
	if(::access("romfs:/RPG_RT.lmt", F_OK) == 0) {
		Output::Debug("Running packaged game from RomFS.");
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
		Output::Debug("CIA: Using hard-coded path.");
	} else if(args[0].rfind("3dslink:/", 0) == 0) {
		// Check if a game directory was provided, otherwise use default
		if (std::none_of(args.cbegin(), args.cend(),
			[](const std::string& a) { return a == "--project-path"; })) {
			ctr_dir = tmp_path;
			Output::Debug("3dslink: Using hard-coded path.");
		}
	}
	// otherwise uses cwd by default or 3dslink argument
	if (!ctr_dir.empty()) {
		args.push_back("--project-path");
		args.push_back(ctr_dir);
	}

	// Setup teardown code
	atexit(n3dsExit);

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	// Close
	return Player::exit_code;
}
