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

#ifndef _GAME_VARIABLES_H_
#define _GAME_VARIABLES_H_

// Headers
#include "main_data.h"
#include "output.h"
#include <vector>

/**
 * Game_Variables class.
 */
class Game_Variables_Class {
public:
	Game_Variables_Class(std::vector<uint32_t>& variables) :
		variables(variables) {}

	int& operator[] (int variable_id) {
		if (!isValidVar(variable_id)) {
			Output::Warning("Variable index %d is invalid.",
							variable_id);
			dummy = 0;
			return dummy;
		}

		return (int&) variables[variable_id - 1];
	}

	std::string GetName(int _id) {
		if (!isValidVar(_id)) {
			Output::Warning("Variable index %d is invalid.\n",
				_id);
			return "";
		}
		else
			return Data::variables.at(_id - 1).name;
	}

	bool isValidVar(int variable_id) {
		return (variable_id > 0 && variable_id <= (int) variables.size());
	}

	int size (){
		return variables.size();
	}

private:
	std::vector<uint32_t>& variables;
	int dummy;
};

// Global variable
extern Game_Variables_Class Game_Variables;

#endif
