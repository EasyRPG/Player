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

#ifndef EP_INPUT_BUTTONS_H
#define EP_INPUT_BUTTONS_H

// Headers
#include <vector>
#include <array>
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <ostream>

#include <lcf/enum_tags.h>
#include "flat_map.h"
#include "keys.h"

/**
 * Input namespace.
 */
namespace Input {
	/** Input buttons list. */
	enum InputButton : uint8_t {
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
		TOGGLE_FULLSCREEN,
		TOGGLE_ZOOM,
		BUTTON_COUNT
	};

	constexpr auto kButtonNames = lcf::makeEnumTags<InputButton>(
		"UP",
		"DOWN",
		"LEFT",
		"RIGHT",
		"DECISION",
		"CANCEL",
		"SHIFT",
		"N0",
		"N1",
		"N2",
		"N3",
		"N4",
		"N5",
		"N6",
		"N7",
		"N8",
		"N9",
		"PLUS",
		"MINUS",
		"MULTIPLY",
		"DIVIDE",
		"PERIOD",
		"DEBUG_MENU",
		"DEBUG_THROUGH",
		"DEBUG_SAVE",
		"TOGGLE_FPS",
		"TAKE_SCREENSHOT",
		"SHOW_LOG",
		"RESET",
		"PAGE_UP",
		"PAGE_DOWN",
		"SCROLL_UP",
		"SCROLL_DOWN",
		"FAST_FORWARD",
		"TOGGLE_FULLSCREEN",
		"TOGGLE_ZOOM",
		"BUTTON_COUNT");

	constexpr auto kButtonHelp = lcf::makeEnumTags<InputButton>(
		"Up Direction",
		"Down Direction",
		"Left Direction",
		"Right Direction",
		"Decision Key",
		"Cancel Key",
		"Shift Key",
		"Number 0",
		"Number 1",
		"Number 2",
		"Number 3",
		"Number 4",
		"Number 5",
		"Number 6",
		"Number 7",
		"Number 8",
		"Number 9",
		"Plus Key",
		"Minus Key",
		"Multiply Key",
		"Divide Key",
		"Period Key",
		"(Test Play) Open the debug menu",
		"(Test Play) Walk through walls",
		"(Test Play) Open the save menu",
		"Open the settings menu",
		"Toggle the FPS display",
		"Take a screenshot",
		"Show the console log on the screen",
		"Reset to the title screen",
		"Page up key",
		"Page down key",
		"Scroll up key",
		"Scroll down key",
		"Fast forward key",
		"Toggle Fullscreen mode",
		"Toggle Window Zoom level",
		"Total Button Count");

	/**
	 * Return true if the given button is a system button.
	 * System buttons are refreshed on every physical frame
	 * and do not affect the game logic.
	 */
	constexpr bool IsSystemButton(InputButton b) {
		switch (b) {
			case TOGGLE_FPS:
			case TAKE_SCREENSHOT:
			case SHOW_LOG:
			case TOGGLE_ZOOM:
			case FAST_FORWARD:
				return true;
			default:
				return false;
		}
	}

	namespace Direction {
		enum InputDirection : uint8_t {
			NONE = 0,
			DOWNLEFT = 1,
			DOWN = 2,
			DOWNRIGHT = 3,
			LEFT = 4,
			CENTER = 5,
			RIGHT = 6,
			UPLEFT = 7,
			UP = 8,
			UPRIGHT = 9,
			NUM_DIRECTIONS = 10,
		};

		static constexpr auto kNames = lcf::makeEnumTags<InputDirection>(
			"NONE",
			"DOWNLEFT",
			"DOWN",
			"DOWNRIGHT",
			"LEFT",
			"CENTER",
			"RIGHT",
			"UPLEFT",
			"UP",
			"UPRIGHT",
			"NUM_DIRECTIONS");
	};

	using ButtonMappingArray = FlatUniqueMultiMap<InputButton,Keys::InputKey>;

	/** A mapping for a button to a input key */
	using ButtonMapping = ButtonMappingArray::pair_type;

	inline std::ostream& operator<<(std::ostream& os, ButtonMapping bm) {
		os << "{ " << kButtonNames.tag(bm.first) << ", " << Keys::kNames.tag(bm.second) << " }";
		return os;
	}

	using DirectionMappingArray = FlatUniqueMultiMap<Direction::InputDirection, InputButton>;

	/** A mapping for a single direction to a button */
	using DirectionMapping = DirectionMappingArray::pair_type;

	inline std::ostream& operator<<(std::ostream& os, DirectionMapping dm) {
		os << "{ " << Direction::kNames.tag(dm.first) << ", " << kButtonNames.tag(dm.second) << " }";
		return os;
	}

	/** Returns default button mappings */
	ButtonMappingArray GetDefaultButtonMappings();

	/** Returns default direction mappings */
	DirectionMappingArray GetDefaultDirectionMappings();
}

#endif
