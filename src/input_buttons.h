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

#include "enum_tags.h"
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

	constexpr auto kButtonNames = makeEnumTags<InputButton>(
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

	constexpr auto kButtonHelp = makeEnumTags<InputButton>(
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

		static constexpr auto kNames = makeEnumTags<InputDirection>(
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

	/** A mapping for a single button to a key */
	struct ButtonMapping {
		using domain_type = InputButton;
		using range_type = Keys::InputKey;

		InputButton button = BUTTON_COUNT;
		Keys::InputKey key = Keys::NONE;
	};

	inline bool operator==(ButtonMapping l, ButtonMapping r) {
		return l.button == r.button && l.key == r.key;
	}

	inline bool operator!=(ButtonMapping l, ButtonMapping r) {
		return !(l == r);
	}

	inline bool operator<(ButtonMapping l, ButtonMapping r) {
		return l.button < r.button || (l.button == r.button && l.key < r.key);
	}

	inline bool operator>(ButtonMapping l, ButtonMapping r) {
		return l.button > r.button || (l.button == r.button && l.key > r.key);
	}

	inline bool operator<=(ButtonMapping l, ButtonMapping r) {
		return !(l > r);
	}

	inline bool operator>=(ButtonMapping l, ButtonMapping r) {
		return !(l < r);
	}

	inline std::ostream& operator<<(std::ostream& os, ButtonMapping bm) {
		os << "{ " << kButtonNames.tag(bm.button) << ", " << Keys::kNames.tag(bm.key) << " }";
		return os;
	}

	/** A mapping for a single direction to a button */
	struct DirectionMapping {
		using domain_type = Direction::InputDirection;
		using range_type = InputButton;

		Direction::InputDirection direction = Direction::NONE;
		InputButton button = BUTTON_COUNT;
	};

	inline bool operator==(DirectionMapping l, DirectionMapping r) {
		return l.direction == r.direction && l.button == r.button;
	}

	inline bool operator!=(DirectionMapping l, DirectionMapping r) {
		return !(l == r);
	}

	inline bool operator<(DirectionMapping l, DirectionMapping r) {
		return l.direction < r.direction || (l.direction == r.direction && l.button < r.button);
	}

	inline bool operator>(DirectionMapping l, DirectionMapping r) {
		return l.direction > r.direction || (l.direction == r.direction && l.button > r.button);
	}

	inline bool operator<=(DirectionMapping l, DirectionMapping r) {
		return !(l > r);
	}

	inline bool operator>=(DirectionMapping l, DirectionMapping r) {
		return !(l < r);
	}

	inline std::ostream& operator<<(std::ostream& os, DirectionMapping bm) {
		os << "{ " << Direction::kNames.tag(bm.direction) << ", " << kButtonNames.tag(bm.button) << " }";
		return os;
	}

	/** Container which contains all button mappings. The mappings are stored in sorted order. */
	template <typename T>
	class InputMappingArray {
		public:
			using value_type = T;
			using container_type = std::vector<value_type>;
			using domain_type = typename value_type::domain_type;
			using range_type = typename value_type::range_type;

			using iterator = typename container_type::iterator;
			using const_iterator = typename container_type::const_iterator;

			InputMappingArray() = default;
			InputMappingArray(std::initializer_list<T> ilist);

			/**
			 * Check whether the input mapping is in the array.
			 * @param im the mapping to check.
			 * @return true if im is in the set.
			 */
			bool Has(value_type im) const;

			/**
			 * Adds the input mapping if it doesn't exist.
			 * @param im the mapping to add
			 * @return true if was added.
			 */
			bool Add(value_type im);

			/**
			 * Removes the input mapping if it exists.
			 * @param im the mapping to remove
			 * @return true if im was removed.
			 */
			bool Remove(value_type im);

			/**
			 * Removes all mappings for the given domain type.
			 * @param d the domain to remove
			 * @return the number of mappings removed.
			 */
			int RemoveAll(domain_type d);

			/**
			 * Replace all mappings for the given domain type with range type objects.
			 * @param d the domain to remove
			 * @param r the range to replace with.
			 */
			void ReplaceAll(domain_type d, const std::vector<range_type>& r);

			/** Iterator to beginning */
			iterator begin() { return mappings.begin(); }
			/** Iterator to end */
			iterator end() { return mappings.end(); }

			/** Iterator to beginning */
			const_iterator begin() const { return mappings.begin(); }
			/** Iterator to end */
			const_iterator end() const { return mappings.end(); }

			/** @return the number of button mappings */
			size_t size() const { return mappings.size(); }
		private:
			container_type mappings;
	};

	using ButtonMappingArray = InputMappingArray<ButtonMapping>;
	using DirectionMappingArray = InputMappingArray<DirectionMapping>;

	/** Returns default button mappings */
	ButtonMappingArray GetDefaultButtonMappings();

	/** Returns default direction mappings */
	DirectionMappingArray GetDefaultDirectionMappings();
}

#endif
