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
#include "input_buttons.h"
#include "keys.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	// Remote
	return {
		{TOGGLE_FPS, Keys::JOY_OTHER_1}, // A
		{SHIFT, Keys::JOY_OTHER_2}, // B
		{CANCEL, Keys::JOY_OTHER_3}, // 1
		{DECISION, Keys::JOY_OTHER_4}, // 2
		{FAST_FORWARD, Keys::JOY_OTHER_5}, // -
		{FAST_FORWARD_PLUS, Keys::JOY_OTHER_6}, // +
		{CANCEL, Keys::JOY_OTHER_7}, // Home

		// Nunchuck
		{DECISION, Keys::JOY_OTHER_8}, // Z
		{CANCEL, Keys::JOY_OTHER_9}, // C

		// Classic Controller
		{DECISION, Keys::JOY_OTHER_10}, // A
		{CANCEL, Keys::JOY_OTHER_11}, // B
		{SHIFT, Keys::JOY_OTHER_12}, // X
		{N1, Keys::JOY_OTHER_13}, // Y
		{N3, Keys::JOY_OTHER_14}, // L
		{N5, Keys::JOY_OTHER_15}, // R
		{N9, Keys::JOY_OTHER_16}, // Zl
		{TOGGLE_FPS, Keys::JOY_OTHER_17}, // Zr
		{FAST_FORWARD, Keys::JOY_OTHER_18}, // -
		{FAST_FORWARD_PLUS, Keys::JOY_OTHER_19}, // +
		{CANCEL, Keys::JOY_OTHER_20}, // Home

		{LEFT, Keys::JOY_STICK_PRIMARY_LEFT},
		{RIGHT, Keys::JOY_STICK_PRIMARY_RIGHT},
		{DOWN, Keys::JOY_STICK_PRIMARY_DOWN},
		{UP, Keys::JOY_STICK_PRIMARY_UP},

		{N1, Keys::JOY_STICK_SECONDARY_DOWN_LEFT},
		{N2, Keys::JOY_STICK_SECONDARY_DOWN},
		{N3, Keys::JOY_STICK_SECONDARY_DOWN_RIGHT},
		{N4, Keys::JOY_STICK_SECONDARY_LEFT},
		{N6, Keys::JOY_STICK_SECONDARY_RIGHT},
		{N7, Keys::JOY_STICK_SECONDARY_UP_LEFT},
		{N8, Keys::JOY_STICK_SECONDARY_UP},
		{N9, Keys::JOY_STICK_SECONDARY_UP_RIGHT}
	};
}

Input::DirectionMappingArray Input::GetDefaultDirectionMappings() {
	return {
		{ Direction::DOWN, DOWN },
		{ Direction::LEFT, LEFT },
		{ Direction::RIGHT, RIGHT },
		{ Direction::UP, UP },
	};
}

Input::KeyNamesArray Input::GetInputKeyNames() {
	return {};
}
