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

#ifndef _GAME_VARIABLES_H_
#define _GAME_VARIABLES_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "main_data.h"
#include "output.h"
#include <vector>

////////////////////////////////////////////////////////////
/// Game_Variables class
////////////////////////////////////////////////////////////
class Game_Variables_Class {
public:
	int& operator[] (int variable_id) {
		if (variable_id <= 0 || (size_t)variable_id >= Data::variables.size()) {
			Output::Warning("Variable index %d is invalid.\n",
							variable_id);

			if (data.size() == 0) {
				data.resize(1, 0);
			}
			data[0] = 0;
			return data[0];
		}

		if ((size_t)variable_id >= data.size())
			data.resize(variable_id + 1, 0);

		return data[variable_id];
	}

	bool isValidVar(int variable_id) {
		return (variable_id > 0 && (size_t)variable_id < Data::variables.size());
	}

private:
	std::vector<int> data;
};

// Global variable
extern Game_Variables_Class Game_Variables;

#endif
