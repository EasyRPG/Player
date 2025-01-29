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
#if USE_SDL==1
		// Wiimote
		{DECISION, Keys::JOY_OTHER_0}, // A, shared with Classic Controller (CC)
		{CANCEL, Keys::JOY_OTHER_1}, // B, shared with CC
		{CANCEL, Keys::JOY_OTHER_2}, // 1
		{DECISION, Keys::JOY_OTHER_3}, // 2
		{RESET, Keys::JOY_OTHER_4}, // -, shared with CC
		{FAST_FORWARD_A, Keys::JOY_OTHER_5}, // +, shared with CC
		{SETTINGS_MENU, Keys::JOY_OTHER_6}, // Home, shared with CC

		// Nunchuck
		{DECISION, Keys::JOY_OTHER_7}, // Z
		{TOGGLE_FPS, Keys::JOY_OTHER_8}, // C

		// Classic Controller
		{SHIFT, Keys::JOY_OTHER_9}, // X
		{CANCEL, Keys::JOY_OTHER_10}, // Y
		{N0, Keys::JOY_OTHER_11}, // L
		{FAST_FORWARD_B, Keys::JOY_OTHER_12}, // R
		{N5, Keys::JOY_OTHER_13}, // ZL
		{TOGGLE_FPS, Keys::JOY_OTHER_14}, // ZR
#else
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
		{UP, Keys::JOY_DPAD_UP},
		{DOWN, Keys::JOY_DPAD_DOWN},
		{LEFT, Keys::JOY_DPAD_LEFT},
		{RIGHT, Keys::JOY_DPAD_RIGHT},
		{DECISION, Keys::JOY_A},
		{CANCEL, Keys::JOY_B},
		{CANCEL, Keys::JOY_X},
		{SHIFT, Keys::JOY_Y},
		{DEBUG_ABORT_EVENT, Keys::JOY_SHOULDER_LEFT},
		{TOGGLE_FPS, Keys::JOY_SHOULDER_RIGHT},
		{SETTINGS_MENU, Keys::JOY_BACK},
		{RESET, Keys::JOY_GUIDE},
		{N5, Keys::N5},
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
#endif
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

		// D-Pad on Wiimote & Classic Controller
		{UP, Keys::JOY_DPAD_UP},
		{DOWN, Keys::JOY_DPAD_DOWN},
		{LEFT, Keys::JOY_DPAD_LEFT},
		{RIGHT, Keys::JOY_DPAD_RIGHT},
	};
}

Input::KeyNamesArray Input::GetInputKeyNames() {
	return {
		// SDL1
		{Keys::JOY_OTHER_0, "A (Wiimote / CC)"},
		{Keys::JOY_OTHER_1, "B (Wiimote / CC)"},
		{Keys::JOY_OTHER_2, "1 (Wiimote)"},
		{Keys::JOY_OTHER_3, "2 (Wiimote)"},
		{Keys::JOY_OTHER_4, "- (Wiimote / CC)"},
		{Keys::JOY_OTHER_5, "+ (Wiimote / CC)"},
		{Keys::JOY_OTHER_6, "Home (Wiimote / CC)"},
		{Keys::JOY_OTHER_7, "Z (Nunchuck)"},
		{Keys::JOY_OTHER_8, "C (Nunchuck)"},
		{Keys::JOY_OTHER_9, "X (CC)"},
		{Keys::JOY_OTHER_10, "Y (CC)"},
		{Keys::JOY_OTHER_11, "L (CC)"},
		{Keys::JOY_OTHER_12, "R (CC)"},
		{Keys::JOY_OTHER_13, "ZL (CC)"},
		{Keys::JOY_OTHER_14, "ZR (CC)"},

		// SDL2
		// FIXME: The ABXY buttons are swapped when a CC is connected, automatically handle this?
		// For Wiimote the reported buttons change depending whether a Nunchuck is connected
		// When only a Wiimote is connected it is handled like a sideways Wiimote
		// With Nunchuck/CC connected the buttons 1 and 2 report nothing
		{Keys::JOY_A, "A"}, // 1 (Wiimote), A (+Nunchuck), B (+CC)
		{Keys::JOY_B, "B"}, // 2 (Wiimote), B (+Nunchuck), A (+CC)
		{Keys::JOY_X, "X (CC) / Z (Nunchuck)"}, // B (Wiimote)
		{Keys::JOY_Y, "Y (CC) / C (Nunchuck)"}, // A (Wiimote)
		{Keys::JOY_BACK, "HOME"},
		{Keys::JOY_START, "+"},
		{Keys::JOY_GUIDE, "-"},
		{Keys::JOY_SHOULDER_LEFT, "L"},
		{Keys::JOY_SHOULDER_RIGHT, "R"},
		{Keys::JOY_LTRIGGER_FULL, "ZL"},
		{Keys::JOY_RTRIGGER_FULL, "ZR"},

		// Shared
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
	cfg.gamepad_swap_ab_and_xy.SetOptionVisible(true);
	cfg.gamepad_swap_analog.SetOptionVisible(true);
	cfg.gamepad_swap_dpad_with_buttons.SetOptionVisible(true);
}

#if USE_SDL==1
#include "platform/sdl/axis.h"
SdlAxis Input::GetSdlAxis() {
	// Classic Controller L/R Trigger axis do not report proper values
	// Handled above as Button 11/12
	return {
		0, 1, 2, 3, -1, -1, false, false
	};
}
#endif

