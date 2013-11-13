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

#if defined(OPENDINGUX)

// Headers
#include "input_buttons.h"
#include "keys.h"

void Input::InitButtons() {
	buttons.resize(BUTTON_COUNT);
	
	buttons[UP].push_back(Keys::UP);
	buttons[DOWN].push_back(Keys::DOWN);
	buttons[LEFT].push_back(Keys::LEFT);
	buttons[RIGHT].push_back(Keys::RIGHT);
	buttons[DECISION].push_back(Keys::RETURN); //START
	buttons[DECISION].push_back(Keys::LCTRL); //A
	buttons[CANCEL].push_back(Keys::ESCAPE); //SELECT
	buttons[CANCEL].push_back(Keys::LALT); //B
	buttons[N1].push_back(Keys::LSHIFT);//Y
	buttons[N2].push_back(Keys::SPACE);//X

	buttons[DEBUG_MENU].push_back(Keys::TAB);//L
	buttons[DEBUG_THROUGH].push_back(Keys::BACKSPACE);//R

	dir_buttons.resize(10);
	dir_buttons[2].push_back(DOWN);
	dir_buttons[4].push_back(LEFT);
	dir_buttons[6].push_back(RIGHT);
	dir_buttons[8].push_back(UP);
}

#endif
