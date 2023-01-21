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
#include "output.h"
#include "player.h"

/**
 * If the main function ever needs to change, be sure to update the `main()`
 * functions of the other platforms as well.
 */
extern "C" int main(int argc, char* argv[]) {
	// Yield on start to ensure async operations (e.g. "mounting" of filesystems) can finish
	// 10ms appears to work already but to be on the safe side better use 100ms
	emscripten_sleep(100);

	std::vector<std::string> args;
	args.assign(argv, argv + argc);

	Output::IgnorePause(true);

	Player::Init(std::move(args));
	Player::Run();

	// Yield on shutdown to ensure async operations (e.g. IDBFS saving) can finish
	emscripten_sleep(100);

	return EXIT_SUCCESS;
}
