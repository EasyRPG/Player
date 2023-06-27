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

const Game_Strings::Strings_t& Game_Strings::RangeOp(int string_id_0, int string_id_1, Str_t string, int op) {
	if (EP_UNLIKELY(ShouldWarn(string_id_0))) {
		WarnGet(string_id_0);
	}
	if (EP_UNLIKELY(ShouldWarn(string_id_1))) {
		WarnGet(string_id_1);
	}

	if (string_id_0 <= 0) {	string_id_0 = 1; }
	if (string_id_1 <= 0) {	string_id_1 = 1; }

	// swap so that id_0 is < id_1
	if (string_id_0 > string_id_1) {
		string_id_0 = string_id_0 ^ string_id_1;
		string_id_1 = string_id_1 ^ string_id_0;
		string_id_0 = string_id_0 ^ string_id_1;
	}

	if (EP_UNLIKELY(string_id_1 > static_cast<int>(_strings.size()))) {
		_strings.resize(string_id_1, "");
	}

	for (int i = string_id_0; i <= string_id_1; i++) {
		switch (op) {
		case 0: Asg(i, string); break;
		case 1: Cat(i, string); break;
		}
	}
	return GetData();
}

Game_Strings::Str_t Game_Strings::PrependMin(Str_t string, int min_size) {
	if (string.size() < min_size) {
		int s = min_size - string.size();
		std::string res = std::string(s, ' ') + (std::string)string;
		return (Str_t)res;
	}
	return string;
}
