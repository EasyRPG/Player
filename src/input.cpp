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
#include "input_source.h"
#include "output.h"
#include "player.h"
#include "system.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <utility>

namespace Input {
	std::array<int, BUTTON_COUNT> press_time;
	std::bitset<BUTTON_COUNT> triggered, repeated, released;
	int dir4;
	int dir8;
	int start_repeat_time;
	int repeat_time;
	std::vector<std::vector<int> > buttons;
	std::vector<std::vector<int> > dir_buttons;
	std::unique_ptr<Source> source;

	bool wait_input = false;
}

namespace {
	bool recording_input;
	std::ofstream record_log;
}

bool Input::IsWaitingInput() { return wait_input; }
void Input::WaitInput(bool v) { wait_input = v; }

static bool InitRecording(const std::string& record_to_path) {
	if (!record_to_path.empty()) {
		auto path = record_to_path.c_str();

		record_log.open(path, std::ios::out|std::ios::trunc);

		if (!record_log) {
			Output::Warning("Failed to open file for input recording: %s", path);
			return false;
		}
	}
	return true;
}

static std::unique_ptr<Input::Source> InitSource(const std::string& replay_from_path) {
	std::unique_ptr<Input::Source> src;

	if (!replay_from_path.empty()) {
		auto path = replay_from_path.c_str();

		std::unique_ptr<Input::LogSource> log_src(new Input::LogSource(path));

		if (!*log_src) {
			Output::Warning("Failed to open file for input replaying: %s", path);
		} else {
			src = std::move(log_src);
		}
	}

	if (!src) {
		src.reset(new Input::UiSource);
	}

	return src;
}

void Input::Init(
	const std::string& replay_from_path,
	const std::string& record_to_path
) {
	InitButtons();

	std::fill(press_time.begin(), press_time.end(), 0);
	triggered.reset();
	repeated.reset();
	released.reset();

	start_repeat_time = 20;
	repeat_time = 5;

	source = InitSource(replay_from_path);
	recording_input = InitRecording(record_to_path);
}

void Input::Update() {
	wait_input = false; // clear each frame

	source->Update();
	auto& pressed_buttons = source->GetPressedButtons();

	if (recording_input) {
		record_log << pressed_buttons << '\n';
	}

	// Check button states
	for (unsigned i = 0; i < BUTTON_COUNT; ++i) {
		bool pressed = pressed_buttons[i];

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

	// TODO: we want Input to be agnostic to where the button
	// presses are coming from, and if there's a UI at all.
	// Move this into the callers?
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
						[](int t) {return t > 0;}) != press_time.end();
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
