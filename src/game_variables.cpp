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
#include "player.h"
#include "reader_util.h"

constexpr int kMaxWarnings = 10;

Game_Variables_Class::Game_Variables_Class() {}

static std::vector<int32_t>& variables() {
	return Main_Data::game_data.system.variables;
}

int Game_Variables_Class::Get(int variable_id) const {
	if ((variable_id <= 0 || variable_id > Data::variables.size()) && _warnings < kMaxWarnings) {
		Output::Debug("Invalid read var[%d]!", variable_id);
		++_warnings;
	}
	auto& vv = variables();
	if (variable_id <= 0 || variable_id > vv.size()) {
		return 0;
	}
	return vv[variable_id - 1];
}

void Game_Variables_Class::Set(int variable_id, int value) {
	if ((variable_id <= 0 || variable_id > Data::variables.size()) && _warnings < kMaxWarnings) {
		Output::Debug("Invalid write var[%d] = %d!", variable_id, value);
		++_warnings;
	}
	auto& vv = variables();
	if (variable_id <= 0) {
		return;
	}
	if (variable_id > vv.size()) {
		vv.resize(variable_id);
	}
	const int maxval = Player::IsRPG2k3() ? 9999999 : 999999;
	const int minval = Player::IsRPG2k3() ? -9999999 : -999999;
	vv[variable_id - 1] = std::max(std::min(value, maxval), minval);
}

std::string Game_Variables_Class::GetName(int _id) const {
	const RPG::Variable* var = ReaderUtil::GetElement(Data::variables, _id);

	if (!var) {
		// No warning, is valid because the variable array resizes dynamic during runtime
		return "";
	} else {
		return var->name;
	}
}

bool Game_Variables_Class::IsValid(int variable_id) const {
	return variable_id > 0 && variable_id <= (int)Data::variables.size();
}

int Game_Variables_Class::GetSize() const {
	return (int)variables().size();
}

void Game_Variables_Class::Reset() {
	variables().clear();
	_warnings = 0;
}
