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

#include "baseui.h"
#include "input_source.h"
#include "player.h"

void Input::UiSource::Update() {
	BaseUi::KeyStatus& keystates = DisplayUi->GetKeyStates();

	for (unsigned i = 0; i < BUTTON_COUNT; ++i) {
		bool pressed = std::any_of(
			buttons[i].cbegin(), buttons[i].cend(),
			[&](int key) { return keystates[key]; }
		);
		pressed_buttons[i] = pressed;
	}
}

Input::LogSource::LogSource(const char* log_path) :
	log_file(log_path, std::ios::in)
{}

void Input::LogSource::Update() {
	log_file >> pressed_buttons;

	if (!log_file) {
		Player::exit_flag = true;
	}
}
