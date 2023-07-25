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
#include "game_config.h"
#include "input_buttons.h"
#include "input_source.h"
#include "output.h"
#include "player.h"
#include "system.h"
#include "baseui.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <utility>
#include <cassert>

namespace Input {
	/**
	 * Start repeat time (in frames) a key has
	 * to be maintained pressed before being
	 * repeated for fist time.
	 */
	constexpr int start_repeat_time = 23;

	/**
	 * Repeat time (in frames) a key has to be
	 * maintained pressed after the start repeat time
	 * has passed for being repeated again.
	 */
	constexpr int repeat_time = 4;

	std::array<int, BUTTON_COUNT> press_time;
	std::bitset<BUTTON_COUNT> triggered, repeated, released;
	Input::KeyStatus raw_triggered, raw_pressed, raw_released;
	int dir4;
	int dir8;
	std::unique_ptr<Source> source;

	bool game_focused = true;
	Input::KeyStatus raw_disabled;
	std::string text_input;
	std::string text_input_disabled;

	bool wait_input = false;
}

bool Input::IsGameFocused() { return game_focused; }
void Input::SetGameFocus(bool focused) {
	game_focused = focused;
	// prevent external inputs from being readable by game on the frame focus is switched (and vice versa)
	if (game_focused) {
		ResetKeys();
	}
}

bool Input::IsWaitingInput() { return wait_input; }
void Input::WaitInput(bool v) { wait_input = v; }

void Input::Init(
	Game_ConfigInput cfg,
	const std::string& replay_from_path,
	const std::string& record_to_path
) {
	std::fill(press_time.begin(), press_time.end(), 0);
	triggered.reset();
	repeated.reset();
	released.reset();
	raw_triggered.reset();
	raw_pressed.reset();
	raw_released.reset();

	DirectionMappingArray directions = {
		{ Direction::DOWN, DOWN },
		{ Direction::LEFT, LEFT },
		{ Direction::RIGHT, RIGHT },
		{ Direction::UP, UP }
	};

	cfg.Hide();
	Input::GetSupportedConfig(cfg);

	source = Source::Create(cfg, std::move(directions), replay_from_path);
	source->InitRecording(record_to_path);

	ResetMask();
}

static void UpdateButton(int i, bool pressed) {
	using namespace Input;

	if (pressed) {
		released[i] = false;
		press_time[i] += 1;
	} else {
		released[i] = press_time[i] > 0;
		press_time[i] = 0;
	}

	if (press_time[i] > 0) {
		triggered[i] = press_time[i] == 1;
		repeated[i] = press_time[i] == 1 || (press_time[i] >= start_repeat_time && press_time[i] % repeat_time == 0);
	} else {
		triggered[i] = false;
		repeated[i] = false;
	}
}

void Input::Update() {
	wait_input = false; // clear each frame

	source->Update();
	auto& pressed_buttons = source->GetPressedButtons();

	// Check button states
	for (unsigned i = 0; i < BUTTON_COUNT; ++i) {
		bool pressed = pressed_buttons[i];
		UpdateButton(i, pressed);
	}

	text_input = source->GetTextInput();

	auto& directions = source->GetDirectionMappings();

	// Press time for directional buttons, the less they have been pressed, the higher their priority will be
	int dirpress[Direction::NUM_DIRECTIONS] = {};

	// Get max pressed time for each directional button
	for (auto& dm: directions) {
		if (dirpress[dm.first] < press_time[dm.second]) {
			dirpress[dm.first] = press_time[dm.second];
		};
	}

	// Calculate diagonal directions pressed time by dir4 combinations
	dirpress[Direction::DOWNLEFT] += (dirpress[Direction::DOWN] > 0 && dirpress[Direction::LEFT] > 0) ? dirpress[Direction::DOWN] + dirpress[Direction::LEFT] : 0;
	dirpress[Direction::DOWNRIGHT] += (dirpress[Direction::DOWN] > 0 && dirpress[Direction::RIGHT] > 0) ? dirpress[Direction::DOWN] + dirpress[Direction::RIGHT] : 0;
	dirpress[Direction::UPLEFT] += (dirpress[Direction::UP] > 0 && dirpress[Direction::LEFT] > 0) ? dirpress[Direction::UP] + dirpress[Direction::LEFT] : 0;
	dirpress[Direction::UPRIGHT] += (dirpress[Direction::UP] > 0 && dirpress[Direction::RIGHT] > 0) ? dirpress[Direction::UP] + dirpress[Direction::RIGHT] : 0;

	dir4 = Direction::NONE;
	dir8 = Direction::NONE;

	// Check if no opposed keys are being pressed at the same time
	if (!(dirpress[Direction::DOWN] > 0 && dirpress[Direction::UP] > 0) && !(dirpress[Direction::LEFT] > 0 && dirpress[Direction::RIGHT] > 0)) {

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
		if		(dirpress[Direction::UPRIGHT] > 0)	dir8 = Direction::UPRIGHT;
		else if (dirpress[Direction::UPLEFT] > 0)	dir8 = Direction::UPLEFT;
		else if (dirpress[Direction::DOWNRIGHT] > 0)	dir8 = Direction::DOWNRIGHT;
		else if (dirpress[Direction::DOWNLEFT] > 0)	dir8 = Direction::DOWNLEFT;
	}

	// Determine pressed & released keys from raw keystate
	const auto& raw_pressed_now = source->GetPressedKeys();
	for (unsigned i = 0; i < Input::Keys::KEYS_COUNT; ++i) {
		raw_triggered[i] = raw_pressed_now[i] && !raw_pressed[i];
		raw_released[i] = !raw_pressed_now[i] && raw_pressed[i];
	}
	raw_pressed = raw_pressed_now;
}

void Input::UpdateSystem() {
	source->UpdateSystem();
	auto& pressed_buttons = source->GetPressedButtons();

	// Check button states
	for (unsigned i = 0; i < BUTTON_COUNT; ++i) {
		if (IsSystemButton(static_cast<InputButton>(i))) {
			bool pressed = pressed_buttons[i];
			UpdateButton(i, pressed);
		}
	}
}

void Input::ResetKeys() {
	triggered.reset();
	repeated.reset();
	released.reset();
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		press_time[i] = 0;
	}
	dir4 = Direction::NONE;
	dir8 = Direction::NONE;

	text_input = "";

	// TODO: we want Input to be agnostic to where the button
	// presses are coming from, and if there's a UI at all.
	// Move this into the callers?
	if (DisplayUi) {
		DisplayUi->GetKeyStates().reset();
	}
}

void Input::ResetTriggerKeys() {
	triggered.reset();
}

void Input::ResetNonSystemKeys() {
	auto buttons = source->GetButtonMappings();
	auto& key_states = DisplayUi->GetKeyStates();

	for (auto& bm: buttons) {
		if (IsSystemButton(bm.first)) {
			continue;
		}

		triggered[bm.first] = false;
		repeated[bm.first] = false;
		released[bm.first] = false;
		press_time[bm.first] = 0;
		key_states[bm.second] = false;
	}

	dir4 = Direction::NONE;
	dir8 = Direction::NONE;
}

void Input::ResetDefaultMapping(Input::InputButton button) {
	auto def_mappings = GetDefaultButtonMappings();
	auto& mappings = Input::GetInputSource()->GetButtonMappings();

	mappings.RemoveAll(button);

	for (auto ki = def_mappings.LowerBound(button); ki != def_mappings.end() && ki->first == button;++ki) {
		mappings.Add(*ki);
	}
}

void Input::ResetAllMappings() {
	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	mappings = GetDefaultButtonMappings();
}

/**
 * pressed: press_time > 0
 * triggered: press_time == 1
 * repeated: press_time == 1 || press_time > start_repeat_time_23 and divide into repeat_time_4
 * released: pressed == false && press_time > 0
 */

bool Input::IsPressed(InputButton button) {
	assert(!IsSystemButton(button));
	if(!IsGameFocused()) return false;
	WaitInput(true);
	return press_time[button] > 0;
}

bool Input::IsTriggered(InputButton button) {
	assert(!IsSystemButton(button));
	if(!IsGameFocused()) return false;
	WaitInput(true);
	return triggered[button];
}

bool Input::IsRepeated(InputButton button) {
	assert(!IsSystemButton(button));
	if(!IsGameFocused()) return false;
	WaitInput(true);
	return repeated[button];
}

bool Input::IsReleased(InputButton button) {
	assert(!IsSystemButton(button));
	if(!IsGameFocused()) return false;
	WaitInput(false);
	return released[button];
}

bool Input::IsSystemPressed(InputButton button) {
	assert(IsSystemButton(button));
	if(!IsGameFocused()) return false;
	return press_time[button] > 0;
}

bool Input::IsSystemTriggered(InputButton button) {
	assert(IsSystemButton(button));
	if(!IsGameFocused()) return false;
	return triggered[button];
}

bool Input::IsSystemRepeated(InputButton button) {
	assert(IsSystemButton(button));
	if(!IsGameFocused()) return false;
	return repeated[button];
}

bool Input::IsSystemReleased(InputButton button) {
	assert(IsSystemButton(button));
	if(!IsGameFocused()) return false;
	return released[button];
}

bool Input::IsAnyPressed() {
	if(!IsGameFocused()) return false;
	WaitInput(true);
	return std::find_if(press_time.begin(), press_time.end(),
						[](int t) {return t > 0;}) != press_time.end();
}

bool Input::IsAnyTriggered() {
	if(!IsGameFocused()) return false;
	WaitInput(true);
	return triggered.any();
}

bool Input::IsAnyRepeated() {
	if(!IsGameFocused()) return false;
	WaitInput(true);
	return repeated.any();
}

bool Input::IsAnyReleased() {
	if(!IsGameFocused()) return false;
	WaitInput(false);
	return released.any();
}

std::vector<Input::InputButton> Input::GetAllPressed() {
	if(!IsGameFocused()) return std::vector<InputButton>();
	WaitInput(true);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (press_time[i] > 0)
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllTriggered() {
	if(!IsGameFocused()) return std::vector<InputButton>();
	WaitInput(true);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (triggered[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllRepeated() {
	if(!IsGameFocused()) return std::vector<InputButton>();
	WaitInput(true);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (repeated[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

std::vector<Input::InputButton> Input::GetAllReleased() {
	if(!IsGameFocused()) return std::vector<InputButton>();
	WaitInput(false);
	std::vector<InputButton> vector;
	for (unsigned i = 0; i < BUTTON_COUNT; i++) {
		if (released[i])
			vector.push_back((InputButton)i);
	}
	return vector;
}

bool Input::IsRawKeyPressed(Input::Keys::InputKey key) {
	if(!IsGameFocused()) return false;
	return raw_pressed[key];
}

bool Input::IsRawKeyTriggered(Input::Keys::InputKey key) {
	if(!IsGameFocused()) return false;
	return raw_triggered[key];
}

bool Input::IsRawKeyReleased(Input::Keys::InputKey key) {
	if(!IsGameFocused()) return false;
	return raw_released[key];
}

const Input::KeyStatus& Input::GetAllRawPressed() {
	if(!IsGameFocused()) return raw_disabled;
	return raw_pressed;
}

const Input::KeyStatus& Input::GetAllRawTriggered() {
	if(!IsGameFocused()) return raw_disabled;
	return raw_triggered;
}

const Input::KeyStatus& Input::GetAllRawReleased() {
	if(!IsGameFocused()) return raw_disabled;
	return raw_released;
}

int Input::GetDir4() {
	if(!IsGameFocused()) return Direction::NONE;
	return dir4;
}

int Input::GetDir8() {
	if(!IsGameFocused()) return Direction::NONE;
	return dir8;
}

bool Input::IsExternalPressed(InputButton button) {
	assert(!IsSystemButton(button));
	if(IsGameFocused()) return false;
	WaitInput(true);
	return press_time[button] > 0;
}

bool Input::IsExternalTriggered(InputButton button) {
	assert(!IsSystemButton(button));
	if(IsGameFocused()) return false;
	WaitInput(true);
	return triggered[button];
}

bool Input::IsExternalRepeated(InputButton button) {
	assert(!IsSystemButton(button));
	if(IsGameFocused()) return false;
	WaitInput(true);
	return repeated[button];
}

bool Input::IsExternalReleased(InputButton button) {
	assert(!IsSystemButton(button));
	if(IsGameFocused()) return false;
	WaitInput(false);
	return released[button];
}

std::string& Input::GetExternalTextInput() {
	if(IsGameFocused()) return text_input_disabled;
	return text_input;
}

Point Input::GetMousePosition() {
	return source->GetMousePosition();
}

void Input::AddRecordingData(Input::RecordingData type, StringView data) {
	assert(source);
	source->AddRecordingData(type, data);
}

bool Input::IsRecording() {
	assert(source);
	return source->IsRecording();
}

Input::Source *Input::GetInputSource() {
	assert(source);
	return source.get();
}

Input::KeyStatus Input::GetMask() {
	assert(source);
	return source->GetMask();
}

void Input::SetMask(Input::KeyStatus new_mask) {
	auto& old_mask = source->GetMask();
	old_mask = new_mask;
}

void Input::ResetMask() {
	assert(source);
	SetMask(source->GetMask());
}

