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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "input.h"
#include "player.h"
#include "system.h"

////////////////////////////////////////////////////////////
namespace Input {
	std::vector<int> press_time;
	std::vector<bool> triggered;
	std::vector<bool> repeated;
	std::vector<bool> released;
	int dir4;
	int dir8;
	int start_repeat_time;
	int repeat_time;
	std::vector<std::vector<int> > buttons;
	std::vector<std::vector<int> > dir_buttons;
}

////////////////////////////////////////////////////////////
void Input::Init() {
	InitButtons();

	press_time.resize(BUTTON_COUNT, 0);
	triggered.resize(BUTTON_COUNT, false);
	repeated.resize(BUTTON_COUNT, false);
	released.resize(BUTTON_COUNT, false);

	start_repeat_time = 20;
	repeat_time = 5;
}

////////////////////////////////////////////////////////////
void Input::Update() {
	std::vector<bool> keystates = DisplayUi->GetKeyStates();

	// Check button states
	for (uint i = 0; i < BUTTON_COUNT; ++i) {
		bool pressed = false;

		// Check state of keys assigned to button
		for (uint e = 0; e < buttons[i].size(); e++) {
			if (keystates[buttons[i][e]]) {
				pressed = true;
				break;
			}
		}

		if (pressed) {
			released[i] = false;
			press_time[i] += 1;
		} else {
			released[i] = press_time[i] > 0;
			press_time[i] = 0;
		}

		if (press_time[i] > 0) {
			triggered[i] = press_time[i] == 1;
			repeated[i] = press_time[i] == 1 || (press_time[i] >= start_repeat_time &&	press_time[i] % repeat_time == 0);
		} else {
			triggered[i] = false;
			repeated[i] = false;
		}
	}
	
	// Press time for directional buttons, the less they have been pressed, the higher their priority will be
	int dirpress[10];

	// Get max pressed time for each directional button
	for (uint i = 1; i < 10; i++) {
		dirpress[i] = 0;
		for (uint e = 0; e < dir_buttons[i].size(); e++) {
			if (dirpress[i] < press_time[dir_buttons[i][e]])
				dirpress[i] = press_time[dir_buttons[i][e]];
		}
	}

	// Calculate diagonal directions pressed time by dir4 combinations
	dirpress[1] += (dirpress[2] > 0 && dirpress[4] > 0) ? dirpress[2] + dirpress[4] : 0;
	dirpress[3] += (dirpress[2] > 0 && dirpress[6] > 0) ? dirpress[2] + dirpress[6] : 0;
	dirpress[7] += (dirpress[8] > 0 && dirpress[4] > 0) ? dirpress[8] + dirpress[4] : 0;
	dirpress[9] += (dirpress[8] > 0 && dirpress[6] > 0) ? dirpress[8] + dirpress[6] : 0;

	dir4 = 0;
	dir8 = 0;
	
	// Check if no opposed keys are being pressed at the same time
	if (!(dirpress[2] > 0 && dirpress[8] > 0) && !(dirpress[4] > 0 && dirpress[6] > 0)) {

		// Get dir4 by the with lowest press time (besides 0 frames)
		int min_press_time = 0;
		for (int i = 2; i <= 8; i += 2) {
			if (dirpress[i] > 0) {
				if (min_press_time == 0 || dirpress[i] < min_press_time) {
					dir4 = i;
					min_press_time = dirpress[i];
				}
			}
		}

		// Dir8 will be at least equal to Dir4
		dir8 = dir4;

		// Check diagonal directions (There is a priority order)
		if		(dirpress[9] > 0)	dir8 = 9;
		else if (dirpress[7] > 0)	dir8 = 7;
		else if (dirpress[3] > 0)	dir8 = 3;
		else if (dirpress[1] > 0)	dir8 = 1;
	}
}

////////////////////////////////////////////////////////////
void Input::ResetKeys() {
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		press_time[i] = 0;
		triggered[i] = false;
		repeated[i] = false;
		released[i] = false;
	}
	dir4 = 0;
	dir8 = 0;
}

////////////////////////////////////////////////////////////
bool Input::IsPressed(InputButton button) {
	return press_time[button] > 0;
}

bool Input::IsTriggered(InputButton button) {
	return triggered[button];
}

bool Input::IsRepeated(InputButton button) {
	return repeated[button];
}

bool Input::IsReleased(InputButton button) {
	return released[button];
}

bool Input::IsAnyPressed() {
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (press_time[i] > 0)
			return true;
	}
	return false;
}

bool Input::IsAnyTriggered() {
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (triggered[i])
			return true;
	}
	return false;
}

bool Input::IsAnyRepeated() {
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (repeated[i])
			return true;
	}
	return false;
}

bool Input::IsAnyReleased() {
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (released[i])
			return true;
	}
	return false;
}

std::vector<Input::InputButton> Input::GetAllPressed() {
	std::vector<InputButton> vector;
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (press_time[i] > 0)
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllTriggered() {
	std::vector<InputButton> vector;
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (triggered[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllRepeated() {
	std::vector<InputButton> vector;
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (repeated[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllReleased() {
	std::vector<InputButton> vector;
	for (uint i = 0; i < BUTTON_COUNT; i++) {
		if (released[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}
