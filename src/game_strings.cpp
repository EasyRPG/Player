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
#include <regex>
#include <lcf/encoder.h>
#include "game_strings.h"
#include "game_variables.h"
#include "output.h"

void Game_Strings::WarnGet(int id) const {
	Output::Debug("Invalid read strvar[{}]!", id);
	--_warnings;
}

Game_Strings::Str_t Game_Strings::Asg(Str_Params params, Str_t string) {
	return Set(params, string);
}

Game_Strings::Str_t Game_Strings::Cat(Str_Params params, Str_t string) {
	if (!ResizeWithId(params.string_id)) return "";

	Str_t s = Get(params.string_id);
	std::string op_string = s;
	op_string.append(string);
	Set(params, op_string);
	return s;
}

int Game_Strings::ToNum(Str_Params params, int var_id) {
	if (!ResizeWithId(params.string_id)) return -1;
	std::string str = Get(params.string_id);

	int num;
	if (params.hex)
		num = static_cast<int>(std::strtol(str.c_str(), nullptr, 16));
	else
		num = static_cast<int>(std::strtol(str.c_str(), nullptr, 0));

	Main_Data::game_variables->Set(var_id, num);
	return num;
}

int Game_Strings::GetLen(Str_Params params, int var_id) {
	if (!ResizeWithId(params.string_id)) return -1;

	int len = Get(params.string_id).length();
	Main_Data::game_variables->Set(var_id, len);
	return len;
}

int Game_Strings::InStr(Str_Params params, std::string search, int var_id, int begin) {
	if (!ResizeWithId(params.string_id)) return -1;

	if (params.extract) {
		search = Extract(search, params.hex);
	}

	std::string str = Get(params.string_id);

	Output::Debug("Searching for {} in {}", search, str);

	int index = Get(params.string_id).find(search, begin);
	Main_Data::game_variables->Set(var_id, index);
	return index;
}

int Game_Strings::Split(Str_Params params, std::string delimiter, int string_out_id, int var_id) {
	if (!ResizeWithId(params.string_id)) return -1;

	size_t index;
	std::string token;

	// always returns at least 1
	int splits = 1;
	std::string str = Get(params.string_id);

	params.string_id = string_out_id;

	for (index = str.find(delimiter); index != std::string::npos; index = str.find(delimiter)) {
		token = str.substr(0, index);
		Set(params, token);
		params.string_id++;
		splits++;
		str.erase(0, index + delimiter.length());
	}

	// set the remaining string
	Set(params, str);
	Main_Data::game_variables->Set(var_id, splits);
	return splits;
}

Game_Strings::Str_t Game_Strings::FromFile(StringView filename, int encoding) {
	Filesystem_Stream::InputStream is = FileFinder::OpenText(filename);
	if (!is) {
		return {};
	}

	auto vec = Utils::ReadStream(is);
	Str_t file_content(vec.begin(), vec.end());

	if (encoding == 0) {
		lcf::Encoder enc(Player::encoding);
		enc.Decode(file_content);
	}

	return file_content;
}

Game_Strings::Str_t Game_Strings::ToFile(Str_Params params, std::string filename, int encoding) {
	std::string str = Get(params.string_id);

	if (params.extract) {
		filename = Extract(filename, params.hex);
	}

	// Maniacs forces the File in Text/ folder with .txt extension
	// TODO: Maybe relax this?
	filename = "Text/" + filename + ".txt";

	auto txt_out = FileFinder::Save().OpenOutputStream(filename);
	if (!txt_out) {
		if (!FileFinder::Save().MakeDirectory("Text", false)) {
			Output::Warning("Maniac String Op ToFile failed: Cannot create Text directory");
			return {};
		}

		txt_out = FileFinder::Save().OpenOutputStream(filename);
		if (!txt_out) {
			Output::Warning("Maniac String Op ToFile failed: Cannot write to {}", filename);
			return {};
		}
	}

	if (encoding == 0) {
		lcf::Encoder enc(Player::encoding);
		enc.Encode(str);
	}

	txt_out << str;
	txt_out.Close();

	return str;
}

Game_Strings::Str_t Game_Strings::PopLine(Str_Params params, int offset, int string_out_id) {
	// FIXME: consideration needed around encoding -- what mode are files read in?
	if (!ResizeWithId(params.string_id)) return "";

	int index;
	std::string result;
	std::string str = Get(params.string_id);

	std::stringstream ss(str);

	while (offset >= 0 && Utils::ReadLine(ss, result)) { offset--; }

	offset = ss.rdbuf()->in_avail();

	Set(params, ss.str().substr(str.length() - offset));
	params.string_id = string_out_id;
	return Set(params, result);
}

Game_Strings::Str_t Game_Strings::ExMatch(Str_Params params, std::string expr, int var_id, int begin, int string_out_id) {
	int var_result;
	Str_t str_result;
	std::smatch match;

	if (params.extract) {
		expr = Extract(expr, params.hex);
	}

	std::string base = Get(params.string_id).erase(0, begin);
	std::regex r(expr);

	std::regex_search(base, match, r);

	var_result = match.position() + begin;
	Main_Data::game_variables->Set(var_id, var_result);

	str_result = match.str();
	if (string_out_id > 0) {
		params.string_id = string_out_id;
		Set(params, str_result);
	}
	return str_result;
}

const Game_Strings::Strings_t& Game_Strings::RangeOp(Str_Params params, int string_id_1, Str_t string, int op, int args[]) {
	if (EP_UNLIKELY(ShouldWarn(params.string_id))) {
		WarnGet(params.string_id);
	}
	if (EP_UNLIKELY(ShouldWarn(string_id_1))) {
		WarnGet(string_id_1);
	}
	if (params.string_id <= 0 && string_id_1 <= 0) { return GetData(); }

	// maniacs just ignores if only one of the params is <= 0
	if (params.string_id <= 0) { params.string_id = 1; }
	if (string_id_1 <= 0) {	string_id_1 = 1; }

	// swap so that id_0 is < id_1
	if (params.string_id > string_id_1) {
		params.string_id = params.string_id ^ string_id_1;
		string_id_1 = string_id_1 ^ params.string_id;
		params.string_id = params.string_id ^ string_id_1;
	}

	if (EP_UNLIKELY(string_id_1 > static_cast<int>(_strings.size()))) {
		_strings.resize(string_id_1, "");
	}

	for (int start = params.string_id; params.string_id <= string_id_1; params.string_id++) {
		switch (op) {
		case 0:  Asg(params, string); break;
		case 1:  Cat(params, string); break;
		case 2:  ToNum(params, args[0] + (params.string_id - start)); break;
		case 3:  GetLen(params, args[0] + (params.string_id - start)); break;
		case 4:  InStr(params, string, args[1], args[2]); break;
		case 5:  params.string_id += Split(params, string, args[1], args[2]); break;
		case 8:  break; // range case not applicable for popLine; see case in game_interpreter.cpp
		case 9:  ExMatch(params, string, args[1] + (params.string_id - start), args[2]); break;
		case 10: ExMatch(params, string, args[1] + (params.string_id - start), args[2], args[3]); break;
		}
	}
	return GetData();
}

Game_Strings::Str_t Game_Strings::PrependMin(Str_t string, int min_size, char c) {
	if (string.size() < min_size) {
		int s = min_size - string.size();
		std::string res = std::string(s, c) + (std::string)string;
		return res;
	}
	return string;
}

