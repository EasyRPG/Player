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
#include "game_strings.h"
#include "game_variables.h"
#include "output.h"

Game_Strings::Game_Strings()
{}

void Game_Strings::WarnGet(int id) const {
	Output::Debug("Invalid read strvar[{}]!", id);
	--_warnings;
}

Game_Strings::Str_t Game_Strings::Asg(int string_id, Str_t string) {
	if (EP_UNLIKELY(ShouldWarn(string_id))) {
		WarnGet(string_id);
	}
	if (string_id <= 0) {
		return "";
	}
	if (EP_UNLIKELY(string_id > static_cast<int>(_strings.size()))) {
		_strings.resize(string_id, "");
	}
	auto& s = _strings[string_id - 1];
	s = string;
	return s;
}

Game_Strings::Str_t Game_Strings::Cat(int string_id, Str_t string) {
	if (EP_UNLIKELY(ShouldWarn(string_id))) {
		WarnGet(string_id);
	}
	if (string_id <= 0) {
		return "";
	}
	if (EP_UNLIKELY(string_id > static_cast<int>(_strings.size()))) {
		_strings.resize(string_id, "");
	}
	auto& s = _strings[string_id - 1];
	std::string op_string = (std::string)s;
	op_string.append((std::string)string);
	s = (Str_t)op_string;
	return s;
}
