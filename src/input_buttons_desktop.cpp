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

#if !(defined(OPENDINGUX) || defined(GEKKO) || defined(USE_LIBRETRO))

// Headers
#include "input_buttons.h"
#include "keys.h"

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
		{DECISION, Keys::Y},
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
		{DECISION, Keys::MOUSE_LEFT},
		{CANCEL, Keys::MOUSE_RIGHT},
		{SHIFT, Keys::MOUSE_MIDDLE},
		{SCROLL_UP, Keys::MOUSE_SCROLLUP},
		{SCROLL_DOWN, Keys::MOUSE_SCROLLDOWN},
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
		// FIXME: Random joystick keys mapping, better to read joystick configuration from .ini
		{UP, Keys::JOY_8},
		{DOWN, Keys::JOY_2},
		{LEFT, Keys::JOY_4},
		{RIGHT, Keys::JOY_6},
		{DECISION, Keys::JOY_1},
		{CANCEL, Keys::JOY_3},
		{SHIFT, Keys::JOY_5},
		{N0, Keys::JOY_10},
		{N1, Keys::JOY_11},
		{N2, Keys::JOY_12},
		{N3, Keys::JOY_13},
		{N4, Keys::JOY_14},
		{N5, Keys::JOY_15},
		{N6, Keys::JOY_16},
		{N7, Keys::JOY_17},
		{N8, Keys::JOY_18},
		{N9, Keys::JOY_19},
		{PLUS, Keys::JOY_20},
		{MINUS, Keys::JOY_21},
		{MULTIPLY, Keys::JOY_22},
		{DIVIDE, Keys::JOY_23},
		{PERIOD, Keys::JOY_24},
		{DEBUG_MENU, Keys::JOY_7},
		{DEBUG_THROUGH, Keys::JOY_9},
#endif

#if defined(USE_JOYSTICK_HAT)  && defined(SUPPORT_JOYSTICK_HAT)
		{DOWN, Keys::JOY_HAT_DOWN},
		{LEFT, Keys::JOY_HAT_LEFT},
		{RIGHT, Keys::JOY_HAT_RIGHT},
		{UP, Keys::JOY_HAT_UP},

#endif

#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
		{LEFT, Keys::JOY_AXIS_X_LEFT},
		{RIGHT, Keys::JOY_AXIS_X_RIGHT},
		{DOWN, Keys::JOY_AXIS_Y_DOWN},
		{UP, Keys::JOY_AXIS_Y_UP},
#endif

#if defined(USE_TOUCH) && defined(SUPPORT_TOUCH)
		{DECISION, Keys::ONE_FINGER},
		{CANCEL, Keys::TWO_FINGERS},
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

#endif
