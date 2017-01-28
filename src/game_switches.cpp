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

#define PLAYER_VAR_LIMIT 1000000

Game_Switches_Class::Game_Switches_Class() {}

static std::vector<bool>& switches() {
	return Main_Data::game_data.system.switches;
}

static int resize_report_limit = 10;

std::vector<bool>::reference Game_Switches_Class::operator[](int switch_id) {
	if (!IsValid(switch_id)) {
		if (switch_id > 0 && switch_id <= PLAYER_VAR_LIMIT) {
			if (resize_report_limit > 0) {
				Output::Debug("Resizing switch array to %d elements.", switch_id);
				--resize_report_limit;
			}
			switches().reserve(switch_id + 1000);
			switches().resize(switch_id);
			Main_Data::game_data.system.switches_size = switches().size();
		} else {
			Output::Debug("Switch index %d is invalid.", switch_id);
			dummy.resize(1);
			return dummy[0];
		}
	}

	return switches()[switch_id - 1];
}

std::string Game_Switches_Class::GetName(int _id) const {
	if (!(_id > 0 && _id <= (int)Data::switches.size())) {
		return "";
	} else {
		return Data::switches[_id - 1].name;
	}
}

bool Game_Switches_Class::IsValid(int switch_id) const {
	return (switch_id > 0 && switch_id <= (int)switches().size());
}

int Game_Switches_Class::GetSize() const {
	return (int)switches().size();
}

void Game_Switches_Class::Reset() {
	resize_report_limit = 10;
	switches().assign(Data::switches.size(), false);
}
