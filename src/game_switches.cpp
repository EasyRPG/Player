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
#include "game_switches.h"
#include "main_data.h"
#include "output.h"
#include "reader_util.h"

constexpr int kMaxWarnings = 10;

Game_Switches_Class::Game_Switches_Class() {}

static std::vector<bool>& switches() {
	return Main_Data::game_data.system.switches;
}

bool Game_Switches_Class::Get(int switch_id) const {
	if ((switch_id <= 0 || switch_id > Data::switches.size()) && _warnings < kMaxWarnings) {
		Output::Debug("Invalid read sw[%d]!", switch_id);
		++_warnings;
	}
	auto& sv = switches();
	if (switch_id <= 0 || switch_id > sv.size()) {
		return false;
	}
	return sv[switch_id - 1];
}

void Game_Switches_Class::Set(int switch_id, bool value) {
	if ((switch_id <= 0 || switch_id > Data::switches.size()) && _warnings < kMaxWarnings) {
		Output::Debug("Invalid write sw[%d] = %d!", switch_id, value);
		++_warnings;
	}
	auto& sv = switches();
	if (switch_id <= 0) {
		return;
	}
	if (switch_id > sv.size()) {
		sv.resize(switch_id);
	}
	sv[switch_id - 1] = value;
}

void Game_Switches_Class::Flip(int switch_id) {
	Set(switch_id, !Get(switch_id));
}

std::string Game_Switches_Class::GetName(int _id) const {
	const RPG::Switch* sw = ReaderUtil::GetElement(Data::switches, _id);

	if (!sw) {
		// No warning, is valid because the switch array resizes dynamic during runtime
		return "";
	} else {
		return sw->name;
	}
}

bool Game_Switches_Class::IsValid(int switch_id) const {
	return switch_id > 0 && switch_id <= (int)Data::switches.size();
}

int Game_Switches_Class::GetSize() const {
	return (int)switches().size();
}

void Game_Switches_Class::Reset() {
	switches().clear();
	_warnings = 0;
}
