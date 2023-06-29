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
	return Set(string_id, string);
}

Game_Strings::Str_t Game_Strings::Cat(int string_id, Str_t string) {
	if (!ResizeWithId(string_id)) return "";

	Str_t s = Get(string_id);
	std::string op_string = static_cast<std::string>(s);
	op_string.append(static_cast<std::string>(string));
	Set(string_id, static_cast<Str_t>(op_string));
	return s;
}

int Game_Strings::ToNum(int string_id, int var_id) {
	if (!ResizeWithId(string_id)) return -1;

	int num = std::stoi(static_cast<std::string>(Get(string_id)));
	Main_Data::game_variables->Set(var_id, num);
	return num;
}

int Game_Strings::GetLen(int string_id, int var_id) {
	if (!ResizeWithId(string_id)) return -1;

	int len = static_cast<std::string>(Get(string_id)).length();
	Main_Data::game_variables->Set(var_id, len);
	return len;
}

int Game_Strings::InStr(int string_id, std::string search, int var_id, int begin) {
	if (!ResizeWithId(string_id)) return -1;

	int index = static_cast<std::string>(Get(string_id)).find(search, begin);
	Main_Data::game_variables->Set(var_id, index);
}

int Game_Strings::Split(int string_id, std::string delimiter, int string_out_id, int var_id) {
	if (!ResizeWithId(string_id)) return -1;

	// always returns at least 1
	int splits = 1;
	size_t index = 0;
	std::string str = static_cast<std::string>(Get(string_id));
	std::string token;
	
	while (index = str.find(delimiter) != std::string::npos) {
		token = str.substr(0, index);
		Set(string_out_id++, static_cast<Str_t>(token));
		splits++;
		str.erase(0, index + delimiter.length());
	}

	// set the remaining string
	Set(string_out_id, static_cast<Str_t>(str));
	Main_Data::game_variables->Set(var_id, splits);
	return splits;
}

Game_Strings::Str_t Game_Strings::PopLine(int string_id, int offset, int string_out_id) {
	if (!ResizeWithId(string_id)) return "";

	int index;
	std::string result;
	std::string str = static_cast<std::string>(Get(string_id));

	std::stringstream ss(str);

	while (offset >= 0 && std::getline(ss, result)) { offset--; }

	offset = ss.rdbuf()->in_avail();

	Set(string_id, static_cast<Str_t>(ss.str().substr(str.length() - offset)));
	return Set(string_out_id, static_cast<Str_t>(result));
}

const Game_Strings::Strings_t& Game_Strings::RangeOp(int string_id_0, int string_id_1, Str_t string, int op, int args[]) {
	if (EP_UNLIKELY(ShouldWarn(string_id_0))) {
		WarnGet(string_id_0);
	}
	if (EP_UNLIKELY(ShouldWarn(string_id_1))) {
		WarnGet(string_id_1);
	}
	if (string_id_0 <= 0 && string_id_1 <= 0) { return GetData(); }

	// maniacs just ignores if only one of the params is <= 0
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
		case 2: ToNum(i, args[0] + (i - string_id_0)); break;
		case 3: GetLen(i, args[0] + (i - string_id_0)); break;
		case 4: InStr(i, static_cast<std::string>(string), args[1], args[2]); break;
		case 5: i += Split(i, static_cast<std::string>(string), args[1], args[2]); break;
		case 8: break; // range case not applicable for popLine; see case in game_interpreter.cpp
		}
	}
	return GetData();
}

Game_Strings::Str_t Game_Strings::PrependMin(Str_t string, int min_size, char c) {
	if (string.size() < min_size) {
		int s = min_size - string.size();
		std::string res = std::string(s, c) + (std::string)string;
		return (Str_t)res;
	}
	return string;
}
