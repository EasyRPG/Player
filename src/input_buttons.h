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
#include <cstdint>
#include <initializer_list>
#include <algorithm>
#include <ostream>

#include <lcf/enum_tags.h>
#include "flat_map.h"
#include "keys.h"

struct Game_ConfigInput;

#if USE_SDL==1
#include "platform/sdl/axis.h"
#endif

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
		DEBUG_ABORT_EVENT,
		SETTINGS_MENU,
		TOGGLE_FPS,
		TAKE_SCREENSHOT,
		SHOW_LOG,
		RESET,
		PAGE_UP,
		PAGE_DOWN,
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		SCROLL_UP,
		SCROLL_DOWN,
		FAST_FORWARD_A,
		FAST_FORWARD_B,
		TOGGLE_FULLSCREEN,
		TOGGLE_ZOOM,
		BUTTON_COUNT
	};

	constexpr auto kInputButtonNames = lcf::makeEnumTags<InputButton>(
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
		"DEBUG_ABORT_EVENT",
		"SETTINGS_MENU",
		"TOGGLE_FPS",
		"TAKE_SCREENSHOT",
		"SHOW_LOG",
		"RESET",
		"PAGE_UP",
		"PAGE_DOWN",
		"MOUSE_LEFT",
		"MOUSE_RIGHT",
		"MOUSE_MIDDLE",
		"SCROLL_UP",
		"SCROLL_DOWN",
		"FAST_FORWARD_A",
		"FAST_FORWARD_B",
		"TOGGLE_FULLSCREEN",
		"TOGGLE_ZOOM",
		"BUTTON_COUNT");

	constexpr auto kInputButtonHelp = lcf::makeEnumTags<InputButton>(
		"Up Direction",
		"Down Direction",
		"Left Direction",
		"Right Direction",
		"Decision (Enter) Key",
		"Cancel (ESC) Key",
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
		"(Test Play) Aborts current active event",
		"Open this settings menu",
		"Toggle the FPS display",
		"Take a screenshot",
		"Show the console log on the screen",
		"Reset to the title screen",
		"Move up one page in menus",
		"Move down one page in menus",
		"Left mouse key",
		"Right mouse key",
		"Middle mouse key",
		"Scroll up key",
		"Scroll down key",
		"Run the game at x{} speed",
		"Run the game at x{} speed",
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
			case FAST_FORWARD_A:
			case FAST_FORWARD_B:
				return true;
			default:
				return false;
		}
	}

	/**
	 * Protected buttons are buttons where unmapping them makes the Player unusable.
	 * @return true when the button is protected
	 */
	constexpr bool IsProtectedButton(InputButton b) {
		switch (b) {
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
			case DECISION:
			case CANCEL:
			case SETTINGS_MENU: // Not really critical but needs a way to enter it
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

		static constexpr auto kInputDirectionNames = lcf::makeEnumTags<InputDirection>(
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
		os << "{ " << kInputButtonNames.tag(bm.first) << ", " << Keys::kInputKeyNames.tag(bm.second) << " }";
		return os;
	}

	using DirectionMappingArray = FlatUniqueMultiMap<Direction::InputDirection, InputButton>;

	/** A mapping for a single direction to a button */
	using DirectionMapping = DirectionMappingArray::pair_type;

	inline std::ostream& operator<<(std::ostream& os, DirectionMapping dm) {
		os << "{ " << Direction::kInputDirectionNames.tag(dm.first) << ", " << kInputButtonNames.tag(dm.second) << " }";
		return os;
	}

	/** Provides platform-specific, human readable name for an input key */
	using KeyNamesArray = std::vector<std::pair<Keys::InputKey, std::string>>;

	/** Returns default button mappings */
	ButtonMappingArray GetDefaultButtonMappings();

	/** Returns platform-specific, human readable name for an input key */
	KeyNamesArray GetInputKeyNames();

	/** Used to declare which config options are available */
	void GetSupportedConfig(Game_ConfigInput& cfg);

#if USE_SDL==1
	SdlAxis GetSdlAxis();
#endif
}

#endif
