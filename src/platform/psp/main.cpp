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

#include <pspkernel.h>
#include <pspthreadman.h>
#include <string>
#include <vector>
#include "player.h"

PSP_MODULE_INFO("EasyRPG Player", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
// Leave 1 MB of the user memory for the kernel
PSP_HEAP_SIZE_KB(-1024);

namespace {
	// Used when argv[0] does not contain the location of the EBOOT
	constexpr const char* DEFAULT_PROJECT_PATH = "ms0:/PSP/GAME/EasyRPG";

	// The HOME button ends the program
	int ExitCallback(int /* arg1 */, int /* arg2 */, void* /* common */) {
		sceKernelExitGame();
		return 0;
	}

	int CallbackThread(SceSize /* args */, void* /* argp */) {
		int cbid = sceKernelCreateCallback("Exit Callback", ExitCallback, nullptr);
		sceKernelRegisterExitCallback(cbid);
		sceKernelSleepThreadCB();
		return 0;
	}

	void SetupCallbacks() {
		int thid = sceKernelCreateThread("Callback Thread", CallbackThread, 0x11, 0xFA0, 0, nullptr);
		if (thid >= 0) {
			sceKernelStartThread(thid, 0, nullptr);
		}
	}

	std::string GetProjectPath(const char* exe_path) {
		// The EBOOT lives in the game folder, so the game data next to it is found
		if (exe_path == nullptr) {
			return DEFAULT_PROJECT_PATH;
		}
		std::string exe = exe_path;
		auto pos = exe.find_last_of('/');
		if (pos == std::string::npos) {
			return DEFAULT_PROJECT_PATH;
		}
		return exe.substr(0, pos);
	}
}

int main(int argc, char* argv[]) {
	SetupCallbacks();

	std::vector<std::string> args(argv, argv + argc);

	args.push_back("--project-path");
	args.push_back(GetProjectPath(argc > 0 ? argv[0] : nullptr));

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	return Player::exit_code;
}
