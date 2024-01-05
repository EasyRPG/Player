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
#include <string>
#include <vector>
#include <emscripten.h>
#include "baseui.h"
#include "output.h"
#include "player.h"

namespace {
	std::vector<std::string> args;
	int counter = 0;
}

static void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
// Allow pretty log output and filtering in browser console
EM_ASM({
  lvl = $0;
  msg = UTF8ToString($1);

  switch (lvl) {
	case 0:
	  console.error(msg);
	  break;
	case 1:
	  console.warn(msg);
	  break;
	case 2:
	  console.info(msg);
	  break;
	case 3:
	  console.debug(msg);
	  break;
	default:
	  console.log(msg);
	  break;
  }
}, static_cast<int>(lvl), msg.c_str());
}

void main_loop() {
	if (counter < 5) {
		++counter;
	}

	if (counter == 5) {
		// Yield on start to ensure async operations (e.g. "mounting" of filesystems) can finish
		Player::Init(std::move(args));
		Player::Run();
		++counter;
	} else if (counter == 6) {
		Player::MainLoop();
		if (!DisplayUi.get()) {
			// Yield on shutdown to ensure async operations (e.g. IDBFS saving) can finish
			counter = -5;
		}
	} else if (counter == -1) {
		emscripten_cancel_main_loop();
	}
}

/**
 * If the main function ever needs to change, be sure to update the `main()`
 * functions of the other platforms as well.
 */
extern "C" int main(int argc, char* argv[]) {
	args.assign(argv, argv + argc);

	Output::IgnorePause(true);
	Output::SetLogCallback(LogCallback);

	emscripten_set_main_loop(main_loop, 0, 0);

	return EXIT_SUCCESS;
}
