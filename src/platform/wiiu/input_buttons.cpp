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
#include "game_config.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
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
		{N0, Keys::JOY_LSTICK},
		{N5, Keys::JOY_RSTICK},
		{DEBUG_ABORT_EVENT, Keys::JOY_SHOULDER_LEFT},
		{TOGGLE_FPS, Keys::JOY_SHOULDER_RIGHT},
		{SETTINGS_MENU, Keys::JOY_START},
		{RESET, Keys::JOY_BACK},
#endif

#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
		{UP, Keys::JOY_LSTICK_UP},
		{DOWN, Keys::JOY_LSTICK_DOWN},
		{LEFT, Keys::JOY_LSTICK_LEFT},
		{RIGHT, Keys::JOY_LSTICK_RIGHT},
		{N1, Keys::JOY_RSTICK_DOWN_LEFT},
		{N2, Keys::JOY_RSTICK_DOWN},
		{N3, Keys::JOY_RSTICK_DOWN_RIGHT},
		{N4, Keys::JOY_RSTICK_LEFT},
		{N6, Keys::JOY_RSTICK_RIGHT},
		{N7, Keys::JOY_RSTICK_UP_LEFT},
		{N8, Keys::JOY_RSTICK_UP},
		{N9, Keys::JOY_RSTICK_UP_RIGHT},
		{FAST_FORWARD_A, Keys::JOY_RTRIGGER_FULL},
		{DEBUG_MENU, Keys::JOY_LTRIGGER_FULL},
#endif

#if defined(USE_TOUCH) && defined(SUPPORT_TOUCH)
		{MOUSE_LEFT, Keys::ONE_FINGER},
		{MOUSE_RIGHT, Keys::TWO_FINGERS},
		{MOUSE_MIDDLE, Keys::THREE_FINGERS},
#endif
	};
}

Input::KeyNamesArray Input::GetInputKeyNames() {
	// FIXME: Wiimote mapping is broken, consider removing
	return {
		{Keys::JOY_A, "A"}, // 2 (Wiimote)
		{Keys::JOY_B, "B"}, // 1 (Wiimote)
		{Keys::JOY_X, "X"}, // (Pad/CC/Pro) / B (Wiimote) / Z (Wiimote+Nunchuck)
		{Keys::JOY_Y, "Y"}, // (Pad/CC/Pro) / A (Wiimote) / C (Wiimote+Nunchuck)
		{Keys::JOY_BACK, "-"},
		{Keys::JOY_START, "+"},
		{Keys::JOY_GUIDE, "Home"}, // FIXME: not mapped at all, maybe use custom trigger
		{Keys::JOY_SHOULDER_LEFT, "L"},
		{Keys::JOY_SHOULDER_RIGHT, "R"},
		{Keys::JOY_LTRIGGER_FULL, "ZL"},
		{Keys::JOY_RTRIGGER_FULL, "ZR"},

		{Keys::JOY_DPAD_UP, "D-Pad Up"},
		{Keys::JOY_DPAD_DOWN, "D-Pad Down"},
		{Keys::JOY_DPAD_LEFT, "D-Pad Left"},
		{Keys::JOY_DPAD_RIGHT, "D-Pad Up"},

		{Keys::JOY_LSTICK, "Left Stick Press"},
		{Keys::JOY_LSTICK_UP, "Left Stick Up"},
		{Keys::JOY_LSTICK_DOWN, "Left Stick Down"},
		{Keys::JOY_LSTICK_LEFT, "Left Stick Left"},
		{Keys::JOY_LSTICK_RIGHT, "Left Stick Right"},

		{Keys::JOY_RSTICK, "Right Stick Press"},
		{Keys::JOY_RSTICK_UP, "Right Stick Up"},
		{Keys::JOY_RSTICK_DOWN, "Right Stick Down"},
		{Keys::JOY_RSTICK_LEFT, "Right Stick Left"},
		{Keys::JOY_RSTICK_RIGHT, "Right Stick Right"},
	};
}

void Input::GetSupportedConfig(Game_ConfigInput& cfg) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	cfg.gamepad_swap_ab_and_xy.SetOptionVisible(true);
	cfg.gamepad_swap_analog.SetOptionVisible(true);
	cfg.gamepad_swap_dpad_with_buttons.SetOptionVisible(true);
#endif
}
