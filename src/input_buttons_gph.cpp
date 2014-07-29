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

#if defined(GPH)

// Headers
#include "input_buttons.h"
#include "keys.h"

void Input::InitButtons() {
	buttons.resize(BUTTON_COUNT);

#if defined(USE_CAANOO)
	buttons[DEBUG_MENU].push_back(Keys::JOY_4);    // L
	buttons[DEBUG_THROUGH].push_back(Keys::JOY_5); // R
	buttons[N1].push_back(Keys::JOY_3);            // Y
	buttons[CANCEL].push_back(Keys::JOY_2);        // B
	buttons[N2].push_back(Keys::JOY_1);            // X
	buttons[DECISION].push_back(Keys::JOY_0);      // A
	buttons[CANCEL].push_back(Keys::JOY_9);        // HELP1
	buttons[DECISION].push_back(Keys::JOY_8);      // HELP2
#elif defined(USE_GP2XWIZ)
	buttons[DEBUG_MENU].push_back(Keys::JOY_10);    // L
	buttons[DEBUG_THROUGH].push_back(Keys::JOY_11); // R
	buttons[N1].push_back(Keys::JOY_15);            // Y
	buttons[CANCEL].push_back(Keys::JOY_13);        // B
	buttons[N2].push_back(Keys::JOY_14);            // X
	buttons[DECISION].push_back(Keys::JOY_12);      // A
	buttons[CANCEL].push_back(Keys::JOY_8);         // SELECT
	buttons[DECISION].push_back(Keys::JOY_9);       // MENU
#endif

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
