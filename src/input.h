/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _EASY_INPUT_H_
#define _EASY_INPUT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "input_buttons.h"

////////////////////////////////////////////////////////////
/// Input namespace.
/// Input works with Button states. Buttons are
/// representations of one or more keys or actions (like
/// keyboard keys, mouse buttons, joystick axis). This way
/// buttons are platform and device independent, while the
/// assigned keys can vary by the system.
////////////////////////////////////////////////////////////
namespace Input {
	////////////////////////////////////////////////////////
	/// Initialize Input.
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Update Input state.
	////////////////////////////////////////////////////////
	void Update();

	////////////////////////////////////////////////////////
	/// Reset all button states.
	////////////////////////////////////////////////////////
	void ResetKeys();

	////////////////////////////////////////////////////////
	/// Get if a button is being pressed.
	/// @param button : button id
	/// @return whether the button is being pressed
	////////////////////////////////////////////////////////
	bool IsPressed(InputButton button);

	////////////////////////////////////////////////////////
	/// Get if a button is starting to being pressed.
	/// @param button : button id
	/// @return whether the button is being triggered
	////////////////////////////////////////////////////////
	bool IsTriggered(InputButton button);

	////////////////////////////////////////////////////////
	/// Get if a button is being repeated. A button is being
	/// repeated while it is maintained pressed and a
	/// certain amount of frames has passed after last
	/// repetition.
	/// @param button : button id
	/// @return whether the button is being repeated
	////////////////////////////////////////////////////////
	bool IsRepeated(InputButton button);

	////////////////////////////////////////////////////////
	/// Get if a button is being released.
	/// @param button : button id
	/// @return whether the button is being released
	////////////////////////////////////////////////////////
	bool IsReleased(InputButton button);

	////////////////////////////////////////////////////////
	/// Get if any button is being pressed.
	/// @return whether any button is being pressed
	////////////////////////////////////////////////////////
	bool IsAnyPressed();

	////////////////////////////////////////////////////////
	/// Get if any button is being triggered.
	/// @return whether any button is being triggered
	////////////////////////////////////////////////////////
	bool IsAnyTriggered();

	////////////////////////////////////////////////////////
	/// Get if any button is being repeated.
	/// @return whether any button is being repeated
	////////////////////////////////////////////////////////
	bool IsAnyRepeated();

	////////////////////////////////////////////////////////
	/// Get if any button is being released.
	/// @return whether any button is being released
	////////////////////////////////////////////////////////
	bool IsAnyReleased();

	////////////////////////////////////////////////////////
	/// Get all buttons being pressed.
	/// @return a vector with the buttons ids
	////////////////////////////////////////////////////////
	std::vector<InputButton> GetAllPressed();

	////////////////////////////////////////////////////////
	/// Get all buttons being triggered.
	/// @return a vector with the buttons ids
	////////////////////////////////////////////////////////
	std::vector<InputButton> GetAllTriggered();

	////////////////////////////////////////////////////////
	/// Get all buttons being repeated.
	/// @return a vector with the buttons ids
	////////////////////////////////////////////////////////
	std::vector<InputButton> GetAllRepeated();

	////////////////////////////////////////////////////////
	/// Get all buttons being released.
	/// @return a vector with the buttons ids
	////////////////////////////////////////////////////////
	std::vector<InputButton> GetAllReleased();

	/// Buttons press time (in frames).
	extern std::vector<int> press_time;

	/// Buttons trigger state.
	extern std::vector<bool> triggered;

	/// Buttons trigger state.
	extern std::vector<bool> repeated;

	/// Buttons trigger state.
	extern std::vector<bool> released;

	/// Horizontal and vertical directions state.
	extern int dir4;

	/// All cardinal directions state.
	extern int dir8;

	/// Start repeat time (in frames) a key has
	/// to be maintained pressed before being
	/// repeated for fist time.
	extern int start_repeat_time;

	/// Repeat time (in frames) a key has to be
	/// maintained pressed after the start repeat time
	/// has passed for being repeated again.
	extern int repeat_time;
}

#endif
