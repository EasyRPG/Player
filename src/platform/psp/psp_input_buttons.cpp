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

#if defined(PSP)

// Headers
#include "input_buttons.h"
#include "keys.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	return {
		{CANCEL, Keys::JOY_A}, // Cross
		{DECISION, Keys::JOY_B}, // Circle
		{FAST_FORWARD, Keys::JOY_X}, // Square
		{SHIFT, Keys::JOY_Y}, // Triangle

		{N1, Keys::JOY_SHOULDER_LEFT}, // Left trigger
		{N2, Keys::JOY_SHOULDER_RIGHT}, // Right trigger

		{DOWN, Keys::JOY_DPAD_DOWN}, // Down
		{LEFT, Keys::JOY_DPAD_LEFT}, // Left
		{UP, Keys::JOY_DPAD_UP}, // Up
		{RIGHT, Keys::JOY_DPAD_RIGHT}, // Right
	}
}

Input::KeyNamesArray Input::GetInputKeyNames() {
	return {};
}

void Input::GetSupportedConfig(Game_ConfigInput& cfg) {
	cfg.gamepad_swap_ab_and_xy.SetOptionVisible(true);
	cfg.gamepad_swap_dpad_with_buttons.SetOptionVisible(true);
}

#endif
