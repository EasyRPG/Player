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
#include "input.h"
#include "player.h"
#include "system.h"

#include <algorithm>
#include <boost/lambda/lambda.hpp>

namespace Input {
	EASYRPG_ARRAY<int, BUTTON_COUNT> press_time;
	std::bitset<BUTTON_COUNT> triggered, repeated, released;
	int dir4;
	int dir8;
	int start_repeat_time;
	int repeat_time;
	std::vector<std::vector<int> > buttons;
	std::vector<std::vector<int> > dir_buttons;

	bool wait_input = false;
}

bool Input::IsWaitingInput() { return wait_input; }
void Input::WaitInput(bool v) { wait_input = v; }

void Input::Init() {
	InitButtons();

	std::fill(press_time.begin(), press_time.end(), 0);
	triggered.reset();
	repeated.reset();
	released.reset();

	start_repeat_time = 20;
	repeat_time = 5;
}

void Input::Update() {
	wait_input = false; // clear each frame

	BaseUi::KeyStatus& keystates = DisplayUi->GetKeyStates();

	// Check button states
	for (unsigned i = 0; i < BUTTON_COUNT; ++i) {
		bool pressed = false;

		// Check state of keys assigned to button
		for (unsigned e = 0; e < buttons[i].size(); e++) {
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
	for (unsigned i = 1; i < 10; i++) {
		dirpress[i] = 0;
		for (unsigned e = 0; e < dir_buttons[i].size(); e++) {
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

void Input::ResetKeys() {
	triggered.reset();
	repeated.reset();
	released.reset();
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		press_time[i] = 0;
	}
	dir4 = 0;
	dir8 = 0;

	DisplayUi->GetKeyStates().reset();
}

bool Input::IsPressed(InputButton button) {
	WaitInput(true);
	return press_time[button] > 0;
}

bool Input::IsTriggered(InputButton button) {
	WaitInput(true);
	return triggered[button];
}

bool Input::IsRepeated(InputButton button) {
	WaitInput(true);
	return repeated[button];
}

bool Input::IsReleased(InputButton button) {
	WaitInput(false);
	return released[button];
}

bool Input::IsAnyPressed() {
	WaitInput(true);
	return std::find_if(press_time.begin(), press_time.end(),
						boost::lambda::_1 > 0) != press_time.end();
}

bool Input::IsAnyTriggered() {
	WaitInput(true);
	return triggered.any();
}

bool Input::IsAnyRepeated() {
	WaitInput(true);
	return repeated.any();
}

bool Input::IsAnyReleased() {
	WaitInput(false);
	return released.any();
}

std::vector<Input::InputButton> Input::GetAllPressed() {
	WaitInput(true);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (press_time[i] > 0)
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllTriggered() {
	WaitInput(true);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (triggered[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllRepeated() {
	WaitInput(true);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (repeated[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllReleased() {
	WaitInput(false);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (released[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}
