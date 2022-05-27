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
#include "libretro.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	// Keyboard not mapped because libretro expects that all input goes
	// through Retropad
	return {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
		{UP, Keys::JOY_DPAD_UP},
		{DOWN, Keys::JOY_DPAD_DOWN},
		{LEFT, Keys::JOY_DPAD_LEFT},
		{RIGHT, Keys::JOY_DPAD_RIGHT},
		{DECISION, Keys::JOY_A},
		{CANCEL, Keys::JOY_B},
		{CANCEL, Keys::JOY_X},
		{SHIFT, Keys::JOY_Y},
		{N0, Keys::JOY_STICK_PRIMARY},
		{N5, Keys::JOY_STICK_SECONDARY},
		{DEBUG_ABORT_EVENT, Keys::JOY_SHOULDER_LEFT},
		{DEBUG_SAVE, Keys::JOY_SHOULDER_RIGHT},
		{SETTINGS_MENU, Keys::JOY_START},
		{RESET, Keys::JOY_BACK},
#endif

#if defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS)
		{UP, Keys::JOY_STICK_PRIMARY_UP},
		{DOWN, Keys::JOY_STICK_PRIMARY_DOWN},
		{LEFT, Keys::JOY_STICK_PRIMARY_LEFT},
		{RIGHT, Keys::JOY_STICK_PRIMARY_RIGHT},
		{N1, Keys::JOY_STICK_SECONDARY_DOWN_LEFT},
		{N2, Keys::JOY_STICK_SECONDARY_DOWN},
		{N3, Keys::JOY_STICK_SECONDARY_DOWN_RIGHT},
		{N4, Keys::JOY_STICK_SECONDARY_LEFT},
		{N6, Keys::JOY_STICK_SECONDARY_RIGHT},
		{N7, Keys::JOY_STICK_SECONDARY_UP_LEFT},
		{N8, Keys::JOY_STICK_SECONDARY_UP},
		{N9, Keys::JOY_STICK_SECONDARY_UP_RIGHT},
		{FAST_FORWARD, Keys::JOY_TRIGGER_RIGHT_PARTIAL},
		{FAST_FORWARD_PLUS, Keys::JOY_TRIGGER_RIGHT_FULL},
		{DEBUG_THROUGH, Keys::JOY_TRIGGER_LEFT_PARTIAL},
		{DEBUG_MENU, Keys::JOY_TRIGGER_LEFT_FULL},
#endif
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
	return {
		{Keys::JOY_DPAD_UP, "D-Pad Up"},
		{Keys::JOY_DPAD_DOWN, "D-Pad Down"},
		{Keys::JOY_DPAD_LEFT, "D-Pad Left"},
		{Keys::JOY_DPAD_RIGHT, "D-Pad Up"},
		{Keys::JOY_A, "A"},
		{Keys::JOY_B, "B"},
		{Keys::JOY_X, "X"},
		{Keys::JOY_Y, "Y"},
		{Keys::JOY_SHOULDER_LEFT, "L"},
		{Keys::JOY_SHOULDER_RIGHT, "R"},
		{Keys::JOY_TRIGGER_LEFT_FULL, "L2"},
		{Keys::JOY_TRIGGER_RIGHT_FULL, "R2"},
		{Keys::JOY_STICK_PRIMARY, "L3"},
		{Keys::JOY_STICK_SECONDARY, "R3"},
		{Keys::JOY_BACK, "Select"},
		{Keys::JOY_START, "Start"}
	};
}
