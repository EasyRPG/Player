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

// FIXME: Move in platform/generic (?) and handle with CMake
#if !(defined(OPENDINGUX) || defined(GEKKO) || defined(USE_LIBRETRO) || defined(__vita__) || defined(__3DS__) || defined(__SWITCH__))

// Headers
#include "input_buttons.h"
#include "keys.h"
#include "game_config.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	return {
		{UP, Keys::UP},
		{UP, Keys::K},
		{UP, Keys::KP8},
		{UP, Keys::W},
		{DOWN, Keys::DOWN},
		{DOWN, Keys::J},
		{DOWN, Keys::KP2},
		{DOWN, Keys::S},
		{LEFT, Keys::LEFT},
		{LEFT, Keys::H},
		{LEFT, Keys::KP4},
		{LEFT, Keys::A},
		{RIGHT, Keys::RIGHT},
		{RIGHT, Keys::L},
		{RIGHT, Keys::KP6},
		{RIGHT, Keys::D},
		{DECISION, Keys::Z},
		{DECISION, Keys::SPACE},
		{DECISION, Keys::RETURN},
		{DECISION, Keys::SELECT},
		{CANCEL, Keys::AC_BACK},
		{CANCEL, Keys::X},
		{CANCEL, Keys::C},
		{CANCEL, Keys::V},
		{CANCEL, Keys::B},
		{CANCEL, Keys::N},
		{CANCEL, Keys::ESCAPE},
		{CANCEL, Keys::KP0},
		{SHIFT, Keys::LSHIFT},
		{SHIFT, Keys::RSHIFT},
		{N0, Keys::N0},
		{N0, Keys::KP0},
		{N1, Keys::N1},
		{N1, Keys::KP1},
		{N2, Keys::N2},
		{N2, Keys::KP2},
		{N3, Keys::N3},
		{N3, Keys::KP3},
		{N4, Keys::N4},
		{N4, Keys::KP4},
		{N5, Keys::N5},
		{N5, Keys::KP5},
		{N6, Keys::N6},
		{N6, Keys::KP6},
		{N7, Keys::N7},
		{N7, Keys::KP7},
		{N8, Keys::N8},
		{N8, Keys::KP8},
		{N9, Keys::N9},
		{N9, Keys::KP9},
		{PLUS, Keys::KP_ADD},
		{PLUS, Keys::RIGHT_BRACKET},
		{MINUS, Keys::KP_SUBTRACT},
		{MINUS, Keys::APOSTROPH},
		{MULTIPLY, Keys::KP_MULTIPLY},
		{MULTIPLY, Keys::LEFT_BRACKET},
		{DIVIDE, Keys::KP_DIVIDE},
		{DIVIDE, Keys::SEMICOLON},
		{PERIOD, Keys::KP_PERIOD},
		{PERIOD, Keys::PERIOD},
		{SETTINGS_MENU, Keys::F1},
		{DEBUG_MENU, Keys::F9},
		{DEBUG_THROUGH, Keys::LCTRL},
		{DEBUG_THROUGH, Keys::RCTRL},
		{DEBUG_SAVE, Keys::F11},
		{DEBUG_ABORT_EVENT, Keys::F10},
		{TAKE_SCREENSHOT, Keys::F7},
		{TOGGLE_FPS, Keys::F2},
		{SHOW_LOG, Keys::F3},
		{TOGGLE_FULLSCREEN, Keys::F4},
		{TOGGLE_ZOOM, Keys::F5},
		{PAGE_UP, Keys::PGUP},
		{PAGE_DOWN, Keys::PGDN},
		{RESET, Keys::F12},
		{FAST_FORWARD, Keys::F},
		{FAST_FORWARD_PLUS, Keys::G},

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
		{MOUSE_LEFT, Keys::MOUSE_LEFT},
		{MOUSE_RIGHT, Keys::MOUSE_RIGHT},
		{MOUSE_MIDDLE, Keys::MOUSE_MIDDLE},
		{SCROLL_UP, Keys::MOUSE_SCROLLUP},
		{SCROLL_DOWN, Keys::MOUSE_SCROLLDOWN},
#endif

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
		{MULTIPLY, Keys::JOY_REAR_LEFT_1},
		{DIVIDE, Keys::JOY_REAR_LEFT_2},
		{PLUS, Keys::JOY_REAR_RIGHT_1},
		{MINUS, Keys::JOY_REAR_RIGHT_2},
		{DEBUG_ABORT_EVENT, Keys::JOY_SHOULDER_LEFT},
		{TOGGLE_FPS, Keys::JOY_SHOULDER_RIGHT},
		{SETTINGS_MENU, Keys::JOY_START},
		{RESET, Keys::JOY_BACK},

#if USE_SDL==1
		// Arbitrary: Remap this when porting to a embedded platform with SDL1
		// (or even better: Provide your own input_buttons.cpp file)
		{DECISION, Keys::JOY_OTHER_0},
		{CANCEL, Keys::JOY_OTHER_1},
		{SHIFT, Keys::JOY_OTHER_2},
		{TOGGLE_FPS, Keys::JOY_OTHER_3},
		{SETTINGS_MENU, Keys::JOY_OTHER_4},
		{RESET, Keys::JOY_OTHER_5},
#endif
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
		{FAST_FORWARD, Keys::JOY_RTRIGGER_SOFT},
		{FAST_FORWARD_PLUS, Keys::JOY_RTRIGGER_FULL},
		{DEBUG_THROUGH, Keys::JOY_LTRIGGER_SOFT},
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
	return {};
}

void Input::GetSupportedConfig(Game_ConfigInput& cfg) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	cfg.gamepad_swap_ab_and_xy.SetOptionVisible(true);
	cfg.gamepad_swap_analog.SetOptionVisible(true);
	cfg.gamepad_swap_dpad_with_buttons.SetOptionVisible(true);
#endif
}

#if USE_SDL==1
#include "platform/sdl/axis.h"
SdlAxis Input::GetSdlAxis() {
	return {
		0, 1, 2, 3, 4, 5, false, false
	};
}
#endif

#endif
