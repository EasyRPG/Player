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

#ifndef _GAME_SWITCHES_H_
#define _GAME_SWITCHES_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "main_data.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Game_Switches class
////////////////////////////////////////////////////////////
class Game_Switches_Class {
public:
	std::vector<bool>::reference operator[](int switch_id) {
		if ((size_t)switch_id < Data::switches.size())
			Output::Error("Switch index %d is invalid.", switch_id);

		if ((size_t)switch_id >= data.size())
			data.resize(switch_id + 1, false);

		return data[switch_id];
	}

private:
	std::vector<bool> data;
};

// Global variable
extern Game_Switches_Class Game_Switches;

#endif
