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

#if defined(PSP)

// Headers
#include "input_buttons.h"
#include "keys.h"

void Input::InitButtons() {
	buttons.resize(BUTTON_COUNT);

	buttons[DECISION].push_back(Keys::JOY_1); // Circle
	buttons[DECISION].push_back(Keys::JOY_2); // Cross
	buttons[CANCEL].push_back(Keys::JOY_3); // Square

	buttons[TOGGLE_FPS].push_back(Keys::JOY_0); // Triangle

	buttons[N1].push_back(Keys::JOY_4); // Left trigger
	buttons[N2].push_back(Keys::JOY_5); // Right trigger

	buttons[DOWN].push_back(Keys::JOY_6); // Down
	buttons[LEFT].push_back(Keys::JOY_7); // Left
	buttons[UP].push_back(Keys::JOY_8); // Up
	buttons[RIGHT].push_back(Keys::JOY_9); // Right

	dir_buttons.resize(10);
	dir_buttons[2].push_back(DOWN);
	dir_buttons[4].push_back(LEFT);
	dir_buttons[6].push_back(RIGHT);
	dir_buttons[8].push_back(UP);

}

#endif
