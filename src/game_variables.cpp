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
#include "game_variables.h"
#include "main_data.h"
#include "output.h"

#define PLAYER_VAR_LIMIT 1000000

Game_Variables_Class::Game_Variables_Class() {}

static std::vector<uint32_t>& variables() {
	return Main_Data::game_data.system.variables;
}

static int resize_report_limit = 10;

int& Game_Variables_Class::operator[] (int variable_id) {
	if (!IsValid(variable_id)) {
		if (variable_id > 0 && variable_id <= PLAYER_VAR_LIMIT) {
			if (resize_report_limit > 0) {
				Output::Debug("Resizing variable array to %d elements.", variable_id);
				--resize_report_limit;
			}
			variables().reserve(variable_id + 1000);
			variables().resize(variable_id);
			Main_Data::game_data.system.variables_size = variables().size();
		} else {
			Output::Debug("Variable index %d is invalid.",
				variable_id);
			dummy = 0;
			return dummy;
		}
	}

	return (int&)variables()[variable_id - 1];
}

std::string Game_Variables_Class::GetName(int _id) const {
	if (!(_id > 0 && _id <= (int)Data::variables.size())) {
		return "";
	} else {
		return Data::variables[_id - 1].name;
	}
}

bool Game_Variables_Class::IsValid(int variable_id) const {
	return (variable_id > 0 && variable_id <= (int)variables().size());
}

int Game_Variables_Class::GetSize() const {
	return (int)variables().size();
}

void Game_Variables_Class::Reset() {
	resize_report_limit = 10;
	variables().assign(Data::variables.size(), 0);
}
