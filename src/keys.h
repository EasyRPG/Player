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

#ifndef _INPUT_KEYS_H_
#define _INPUT_KEYS_H_

// Headers
#include "system.h"

namespace Input {
	/**
	 * Keys namespace
	 */
	namespace Keys {
		enum InputKey {
			NONE,

			BACKSPACE,
			TAB,
			CLEAR,
			RETURN,
			PAUSE,
			ESCAPE,
			SPACE,
			PGUP,
			PGDN,
			ENDS,
			HOME,
			LEFT,
			UP,
			RIGHT,
			DOWN,
			SNAPSHOT,
			INSERT,
			DEL,
			SHIFT,
			LSHIFT,
			RSHIFT,
			CTRL,
			LCTRL,
			RCTRL,
			ALT,
			LALT,
			RALT,
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
			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			LOS,
			ROS,
			MENU,
			KP0,
			KP1,
			KP2,
			KP3,
			KP4,
			KP5,
			KP6,
			KP7,
			KP8,
			KP9,
			MULTIPLY,
			ADD,
			SUBTRACT,
			PERIOD,
			DIVIDE,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			CAPS_LOCK,
			NUM_LOCK,
			SCROLL_LOCK,
			AC_BACK,
			SELECT,

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
			MOUSE_LEFT,
			MOUSE_RIGHT,
			MOUSE_MIDDLE,
			MOUSE_XBUTTON1,
			MOUSE_XBUTTON2,
			MOUSE_SCROLLUP,
			MOUSE_SCROLLDOWN,
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
			JOY_0,
			JOY_1,
			JOY_2,
			JOY_3,
			JOY_4,
			JOY_5,
			JOY_6,
			JOY_7,
			JOY_8,
			JOY_9,
			JOY_10,
			JOY_11,
			JOY_12,
			JOY_13,
			JOY_14,
			JOY_15,
			JOY_16,
			JOY_17,
			JOY_18,
			JOY_19,
			JOY_20,
			JOY_21,
			JOY_22,
			JOY_23,
			JOY_24,
			JOY_25,
			JOY_26,
			JOY_27,
			JOY_28,
			JOY_29,
			JOY_30,
			JOY_31,
#endif

#if defined(USE_JOYSTICK_HAT)  && defined(SUPPORT_JOYSTICK_HAT)
			JOY_HAT_LOWER_LEFT,
			JOY_HAT_DOWN,
			JOY_HAT_LOWER_RIGHT,
			JOY_HAT_LEFT,
			JOY_HAT_RIGHT,
			JOY_HAT_UPPER_LEFT,
			JOY_HAT_UP,
			JOY_HAT_UPPER_RIGHT,
#endif

#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
			JOY_AXIS_X_LEFT,
			JOY_AXIS_X_RIGHT,
			JOY_AXIS_Y_DOWN,
			JOY_AXIS_Y_UP,
#endif

#if defined(USE_TOUCH) && defined(SUPPORT_TOUCH)
			ONE_FINGER,
			TWO_FINGERS,
#endif

			KEYS_COUNT
		};
	}
}

#endif
