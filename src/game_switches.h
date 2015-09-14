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

#ifndef _GAME_SWITCHES_H_
#define _GAME_SWITCHES_H_

// Headers
#include <vector>
#include "main_data.h"
#include "output.h"

#define PLAYER_VAR_LIMIT 1000000

/**
 * Game_Switches class
 */
class Game_Switches_Class {
public:
	Game_Switches_Class(std::vector<bool>& switches) :
		switches(switches) {}

	std::vector<bool>::reference operator[](int switch_id) {
		if (!isValidSwitch(switch_id)) {
			if (switch_id > 0 && switch_id <= PLAYER_VAR_LIMIT) {
				Output::Debug("Resizing switch array to %d elements.", switch_id);
				switches.resize(switch_id);
			}
			else {
				Output::Debug("Switch index %d is invalid.", switch_id);
				dummy.resize(1);
				return dummy[0];
			}
		}

		return switches[switch_id - 1];
	}
	
	std::string GetName(int _id) const {
		if (!(_id > 0 && _id <= (int)Data::switches.size())) {
			return "";
		}
		else {
			return Data::switches[_id - 1].name;
		}
	}

	bool isValidSwitch(int switch_id) const {
		return (switch_id > 0 && switch_id <= (int) switches.size());
	}

	int size () const {
		return (int)switches.size();
	}

	void Reset() {
		switches.assign(Data::switches.size(), false);
	}

private:
	std::vector<bool>& switches;
	std::vector<bool> dummy;
};

#undef PLAYER_VAR_LIMIT

// Global variable
extern Game_Switches_Class Game_Switches;

#endif
