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

#if defined(GEKKO)

// Headers
#include "input_buttons.h"
#include "keys.h"

void Input::InitButtons() {
	buttons.resize(BUTTON_COUNT);

	// Remote
	buttons[TOGGLE_FPS].push_back(Keys::JOY_0); // A
	buttons[SHIFT].push_back(Keys::JOY_1); // B
	buttons[CANCEL].push_back(Keys::JOY_2); // 1
	buttons[DECISION].push_back(Keys::JOY_3); // 2
	buttons[FAST_FORWARD].push_back(Keys::JOY_4); // -
	buttons[PLUS].push_back(Keys::JOY_5); // +
	buttons[CANCEL].push_back(Keys::JOY_6); // Home

	// Nunchuck
	buttons[DECISION].push_back(Keys::JOY_7); // Z
	buttons[CANCEL].push_back(Keys::JOY_8); // C

	// Classic Controller
	buttons[DECISION].push_back(Keys::JOY_9); // A
	buttons[CANCEL].push_back(Keys::JOY_10); // B
	buttons[SHIFT].push_back(Keys::JOY_11); // X
	buttons[N1].push_back(Keys::JOY_12); // Y
	buttons[N3].push_back(Keys::JOY_13); // L
	buttons[N5].push_back(Keys::JOY_14); // R
	buttons[N9].push_back(Keys::JOY_15); // Zl
	buttons[TOGGLE_FPS].push_back(Keys::JOY_16); // Zr
	buttons[FAST_FORWARD].push_back(Keys::JOY_17); // -
	buttons[PLUS].push_back(Keys::JOY_18); // +
	buttons[CANCEL].push_back(Keys::JOY_19); // Home

	buttons[DOWN].push_back(Keys::JOY_HAT_DOWN);
	buttons[LEFT].push_back(Keys::JOY_HAT_LEFT);
	buttons[RIGHT].push_back(Keys::JOY_HAT_RIGHT);
	buttons[UP].push_back(Keys::JOY_HAT_UP);

	buttons[LEFT].push_back(Keys::JOY_AXIS_X_LEFT);
	buttons[RIGHT].push_back(Keys::JOY_AXIS_X_RIGHT);
	buttons[DOWN].push_back(Keys::JOY_AXIS_Y_DOWN);	
	buttons[UP].push_back(Keys::JOY_AXIS_Y_UP);

	dir_buttons.resize(10);
	dir_buttons[2].push_back(DOWN);
	dir_buttons[4].push_back(LEFT);
	dir_buttons[6].push_back(RIGHT);
	dir_buttons[8].push_back(UP);
}

#endif
