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

#include <algorithm>
#include <cstring>
#include <cerrno>

#include "baseui.h"
#include "input_source.h"
#include "player.h"
#include "output.h"

std::unique_ptr<Input::Source> Input::Source::Create(
		Input::ButtonMappingArray buttons,
		Input::DirectionMappingArray directions,
		const std::string& replay_from_path)
{
	if (!replay_from_path.empty()) {
		auto path = replay_from_path.c_str();

		auto log_src = std::make_unique<Input::LogSource>(path, std::move(buttons), std::move(directions));

		if (*log_src) {
			return log_src;
		}
		Output::Warning("Failed to open file for input replaying: %s", path);

		buttons = std::move(log_src->GetButtonMappings());
		directions = std::move(log_src->GetDirectionMappings());
	}

	return std::make_unique<Input::UiSource>(std::move(buttons), std::move(directions));
}

void Input::UiSource::DoUpdate(bool system_only) {
	BaseUi::KeyStatus& keystates = DisplayUi->GetKeyStates();

	pressed_buttons = {};

	for (auto& bm: button_mappings) {
		if (!system_only || Input::IsSystemButton(bm.button)) {
			pressed_buttons[bm.button] = pressed_buttons[bm.button] | keystates[bm.key];
		}
	}

	Record();
}

void Input::UiSource::Update() {
	DoUpdate(false);
}

void Input::UiSource::UpdateSystem() {
	DoUpdate(true);
}

Input::LogSource::LogSource(const char* log_path, ButtonMappingArray buttons, DirectionMappingArray directions)
	: Source(std::move(buttons), std::move(directions)),
	log_file(log_path, std::ios::in)
{}

void Input::LogSource::Update() {
	log_file >> pressed_buttons;

	if (!log_file) {
		Player::exit_flag = true;
	}

	Record();
}


bool Input::Source::InitRecording(const std::string& record_to_path) {
	if (!record_to_path.empty()) {
		auto path = record_to_path.c_str();

		record_log.open(path, std::ios::out|std::ios::trunc);

		if (!record_log) {
			Output::Warning("Failed to open file %s for input recording : %s", path, strerror(errno));
			return false;
		}
	}
	return true;
}

void Input::Source::Record() {
	if (record_log.is_open()) {
		record_log << GetPressedNonSystemButtons() << '\n';
	}
}

void Input::LogSource::UpdateSystem() {
	// input log does not record actions outside of logical frames.
}
