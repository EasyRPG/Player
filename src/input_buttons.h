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

#ifndef _INPUT_BUTTONS_H_
#define _INPUT_BUTTONS_H_

// Headers
#include <vector>

/**
 * Input namespace.
 */
namespace Input {
	/** Input buttons list. */
	enum InputButton {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		DECISION,
		CANCEL,
		SHIFT,
		N0,
		N1,
		N2,
		N3,
		N4,
		N5,
		N6,
		N7,
		N8,
		N9,
		PLUS,
		MINUS,
		MULTIPLY,
		DIVIDE,
		PERIOD,
		DEBUG_MENU,
		DEBUG_THROUGH,
		DEBUG_SAVE,
		TOGGLE_FPS,
		TAKE_SCREENSHOT,
		SHOW_LOG,
		RESET,
		PAGE_UP,
		PAGE_DOWN,
		SCROLL_UP,
		SCROLL_DOWN,
		FAST_FORWARD,
		BUTTON_COUNT
	};

	/**
	 * Initializes input buttons to their mappings.
	 */
	void InitButtons();

	/** Buttons list of equivalent keys. */
	extern std::vector<std::vector<int> > buttons;

	/** Direction buttons list of equivalent buttons. */
	extern std::vector<std::vector<int> > dir_buttons;
}

#endif
