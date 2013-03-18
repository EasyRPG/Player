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

// Headers
#include "player.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"

#include <cstdlib>
#include <SDL.h>

extern "C" int main(int argc, char* argv[]) {
	Player::Init(argc, argv);
	Graphics::Init();
	Input::Init();

	Player::Run();

	return EXIT_SUCCESS;
}
