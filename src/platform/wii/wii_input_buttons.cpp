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

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	// Remote
	return {
		{TOGGLE_FPS, Keys::JOY_0}, // A
		{SHIFT, Keys::JOY_1}, // B
		{CANCEL, Keys::JOY_2}, // 1
		{DECISION, Keys::JOY_3}, // 2
		{FAST_FORWARD, Keys::JOY_4}, // -
		{FAST_FORWARD_PLUS, Keys::JOY_5}, // +
		{CANCEL, Keys::JOY_6}, // Home

		// Nunchuck
		{DECISION, Keys::JOY_7}, // Z
		{CANCEL, Keys::JOY_8}, // C

		// Classic Controller
		{DECISION, Keys::JOY_9}, // A
		{CANCEL, Keys::JOY_10}, // B
		{SHIFT, Keys::JOY_11}, // X
		{N1, Keys::JOY_12}, // Y
		{N3, Keys::JOY_13}, // L
		{N5, Keys::JOY_14}, // R
		{N9, Keys::JOY_15}, // Zl
		{TOGGLE_FPS, Keys::JOY_16}, // Zr
		{FAST_FORWARD, Keys::JOY_17}, // -
		{FAST_FORWARD_PLUS, Keys::JOY_18}, // +
		{CANCEL, Keys::JOY_19}, // Home

		{DOWN, Keys::JOY_HAT_DOWN},
		{LEFT, Keys::JOY_HAT_LEFT},
		{RIGHT, Keys::JOY_HAT_RIGHT},
		{UP, Keys::JOY_HAT_UP},

		{LEFT, Keys::JOY_AXIS_X_LEFT},
		{RIGHT, Keys::JOY_AXIS_X_RIGHT},
		{DOWN, Keys::JOY_AXIS_Y_DOWN},
		{UP, Keys::JOY_AXIS_Y_UP},
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

#endif
