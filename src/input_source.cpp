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
#include <ctime>
#include <cmath>

#include "baseui.h"
#include "input_source.h"
#include "player.h"
#include "output.h"
#include "game_system.h"
#include "main_data.h"
#include "version.h"

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
		Output::Warning("Failed to open file for input replaying: {}", path);

		buttons = std::move(log_src->GetButtonMappings());
		directions = std::move(log_src->GetDirectionMappings());
	}

	return std::make_unique<Input::UiSource>(std::move(buttons), std::move(directions));
}

void Input::UiSource::DoUpdate(bool system_only) {
	keystates = DisplayUi->GetKeyStates();

	pressed_buttons = {};

	UpdateGamepad();

	for (auto& bm: button_mappings) {
		if (keymask[bm.second]) {
			continue;
		}

		if (!system_only || Input::IsSystemButton(bm.first)) {
			pressed_buttons[bm.first] = pressed_buttons[bm.first] | keystates[bm.second];
		}
	}

	Record();

	mouse_pos = DisplayUi->GetMousePosition();
}

void Input::UiSource::Update() {
	DoUpdate(false);
}

void Input::UiSource::UpdateSystem() {
	DoUpdate(true);
}

Input::LogSource::LogSource(const char* log_path, ButtonMappingArray buttons, DirectionMappingArray directions)
	: Source(std::move(buttons), std::move(directions)),
	log_file(FileFinder::Root().OpenInputStream(log_path, std::ios::in))
{
	if (!log_file) {
		Output::Error("Error reading input logfile {}", log_path);
		return;
	}

	std::string header;
	Utils::ReadLine(log_file, header);
	if (StringView(header).starts_with("H EasyRPG")) {
		std::string ver;
		Utils::ReadLine(log_file, ver);
		if (StringView(ver).starts_with("V 2")) {
			version = 2;
		} else {
			Output::Error("Unsupported logfile version {}", ver);
		}
	} else {
		Output::Debug("Using legacy inputlog format");
	}
}

void Input::LogSource::Update() {
	if (version == 2) {
		if (!Main_Data::game_system) {
			return;
		}

		if (last_read_frame == -1) {
			pressed_buttons.reset();

			std::string line;
			while (Utils::ReadLine(log_file, line) && !StringView(line).starts_with("F ")) {
				// no-op
			}
			if (!line.empty()) {
				keys = Utils::Tokenize(line.substr(2), [](char32_t c) { return c == ','; });
				if (!keys.empty()) {
					last_read_frame = atoi(keys[0].c_str());
				}
			}
		}
		if (Main_Data::game_system->GetFrameCounter() == last_read_frame) {
			for (const auto& key : keys) {
				auto it = std::find(Input::kButtonNames.begin(), Input::kButtonNames.end(), key);
				if (it != Input::kButtonNames.end()) {
					pressed_buttons[std::distance(Input::kButtonNames.begin(), it)] = true;
				}
			}
			last_read_frame = -1;
		}
	} else {
		log_file >> pressed_buttons;
	}

	if (!log_file) {
		Player::exit_flag = true;
	}

	Record();
}


bool Input::Source::InitRecording(const std::string& record_to_path) {
	if (!record_to_path.empty()) {
		auto path = record_to_path.c_str();

		record_log = std::make_unique<Filesystem_Stream::OutputStream>(FileFinder::Root().OpenOutputStream(path, std::ios::out | std::ios::trunc));

		if (!record_log) {
			Output::Error("Failed to open file {} for input recording : {}", path, strerror(errno));
			return false;
		}

		*record_log << "H EasyRPG Player Recording\n";
		*record_log << "V 2 " << Version::STRING << "\n";

		std::time_t t = std::time(nullptr);
		// trigraph ?-escapes
		std::string date = R"(????-??-?? ??:??:??)";
		char timestr[100];
		if (std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", std::localtime(&t))) {
			date = std::string(timestr);
		}

		*record_log << "D " << date << '\n';
	}
	return true;
}

void Input::Source::Record() {
	if (record_log) {
		const auto& buttons = GetPressedNonSystemButtons();
		if (buttons.any()) {
			if (!Main_Data::game_system) {
				return;
			}
			int cur_frame = Main_Data::game_system->GetFrameCounter();
			if (cur_frame == last_written_frame) {
				return;
			}
			last_written_frame = cur_frame;

			*record_log << "F " << cur_frame;

			for (size_t i = 0; i < buttons.size(); ++i) {
				if (!buttons[i]) {
					continue;
				}

				*record_log << ',' << Input::kButtonNames[i];
			}

			*record_log << '\n';
		}
	}
}

void Input::Source::UpdateGamepad() {
	// Primary Analog Stick (For directions, does not support diagonals)
	keystates[Input::Keys::JOY_STICK_PRIMARY_RIGHT] = analog_input.primary.x > JOYSTICK_STICK_SENSIBILITY;
	keystates[Input::Keys::JOY_STICK_PRIMARY_LEFT] = analog_input.primary.x < -JOYSTICK_STICK_SENSIBILITY;
	keystates[Input::Keys::JOY_STICK_PRIMARY_UP] = analog_input.primary.y > JOYSTICK_STICK_SENSIBILITY;
	keystates[Input::Keys::JOY_STICK_PRIMARY_DOWN] = analog_input.primary.y < -JOYSTICK_STICK_SENSIBILITY;

	// Secondary Analog Stick (For other things, supports diagonals)
	if (analog_input.secondary.x > JOYSTICK_STICK_SENSIBILITY || analog_input.secondary.x < -JOYSTICK_STICK_SENSIBILITY ||
		analog_input.secondary.y > JOYSTICK_STICK_SENSIBILITY || analog_input.secondary.y < -JOYSTICK_STICK_SENSIBILITY) {

		auto angle = static_cast<int>(std::atan2(analog_input.secondary.y, analog_input.secondary.x) * 180.0f / M_PI);
		if (angle >= -22 && angle <= 22) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_RIGHT] = true;
		} else if (angle >= 23 && angle <= 67) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_UP_RIGHT] = true;
		} else if (angle >= 68 && angle <= 112) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_UP] = true;
		} else if (angle >= 113 && angle <= 157) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_UP_LEFT] = true;
		} else if (angle >= 158 || angle <= -158) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_LEFT] = true;
		} else if (angle >= -157 && angle <= -113) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_DOWN_LEFT] = true;
		} else if (angle >= -112 && angle <= -68) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_DOWN] = true;
		} else if (angle >= -67 && angle <= -23) {
			keystates[Input::Keys::JOY_STICK_SECONDARY_DOWN_RIGHT] = true;
		}
	}

	// Trigger
	analog_input = DisplayUi->GetAnalogInput();
	keystates[Input::Keys::JOY_TRIGGER_LEFT_FULL] = (analog_input.trigger_left > AnalogInput::kMaxValue * 0.9);
	keystates[Input::Keys::JOY_TRIGGER_LEFT_PARTIAL] =
			(analog_input.trigger_left > JOYSTICK_TRIGGER_SENSIBILITY) &&
			!keystates[Input::Keys::JOY_TRIGGER_LEFT_FULL];
	keystates[Input::Keys::JOY_TRIGGER_RIGHT_FULL] = (analog_input.trigger_right > AnalogInput::kMaxValue * 0.9);
	keystates[Input::Keys::JOY_TRIGGER_RIGHT_PARTIAL] =
			(analog_input.trigger_right > JOYSTICK_TRIGGER_SENSIBILITY) &&
			!keystates[Input::Keys::JOY_TRIGGER_RIGHT_FULL];
}

void Input::Source::AddRecordingData(Input::RecordingData type, StringView data) {
	if (record_log) {
		*record_log << static_cast<char>(type) << " " << data << "\n";
	}
}

void Input::LogSource::UpdateSystem() {
	// input log does not record actions outside of logical frames.
}


