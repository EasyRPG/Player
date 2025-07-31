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
#include <lcf/reader_util.h>
#include "async_handler.h"
#include "filefinder.h"
#include "filesystem_stream.h"
#include "game_map.h"
#include "game_message.h"
#include "game_strings.h"
#include "game_switches.h"
#include "game_variables.h"
#include "output.h"
#include "player.h"
#include "utils.h"

#ifdef HAVE_NLOHMANN_JSON
#include "json_helper.h"
#endif

void Game_Strings::WarnGet(int id) const {
	Output::Debug("Invalid read strvar[{}]!", id);
	--_warnings;
}

#ifdef HAVE_NLOHMANN_JSON
nlohmann::ordered_json* Game_Strings::ParseJson(int id) {
	auto it = _json_cache.find(id);
	if (it != _json_cache.end()) {
		return &(it->second);
	}

	auto str = ToString(Get(id));
	auto res = Json_Helper::Parse(str);

	if (!res) {
		return nullptr;
	} else {
		_json_cache[id] = *res;
		return &_json_cache[id];
	}
}
#endif

std::string_view Game_Strings::Asg(Str_Params params, std::string_view string) {
	Set(params, string);
	return Get(params.string_id);
}

std::string_view Game_Strings::Cat(Str_Params params, std::string_view string) {
	if (params.string_id <= 0) {
		return {};
	}

	auto it = _strings.find(params.string_id);
	if (it == _strings.end()) {
		Set(params, string);
		return Get(params.string_id);
	}
	it->second += ToString(string);
	return it->second;
}

int Game_Strings::ToNum(Str_Params params, int var_id, Game_Variables& variables) {
	if (params.string_id <= 0) {
		return -1;
	}

	auto it = _strings.find(params.string_id);
	if (it == _strings.end()) {
		return 0;
	}

	int num;
	if (params.hex)
		num = static_cast<int>(std::strtol(it->second.c_str(), nullptr, 16));
	else
		num = static_cast<int>(std::strtol(it->second.c_str(), nullptr, 0));

	variables.Set(var_id, num);

	Game_Map::SetNeedRefreshForVarChange(var_id);

	return num;
}

int Game_Strings::GetLen(Str_Params params, int var_id, Game_Variables& variables) const {
	if (params.string_id <= 0) {
		return -1;
	}

	int len = Utils::UTF8Length(Get(params.string_id));
	variables.Set(var_id, len);

	Game_Map::SetNeedRefreshForVarChange(var_id);

	return len;
}

int Game_Strings::InStr(Str_Params params, std::string search, int var_id, int begin, Game_Variables& variables) const {
	if (params.string_id <= 0) {
		return -1;
	}

	if (params.extract) {
		search = Extract(search, params.hex);
	}

	auto search32 = Utils::DecodeUTF32(search);
	auto string32 = Utils::DecodeUTF32(Get(params.string_id));

	int index = string32.find(search32, begin);
	variables.Set(var_id, index);

	Game_Map::SetNeedRefreshForVarChange(var_id);

	return index;
}

int Game_Strings::Split(Str_Params params, const std::string& delimiter, int string_out_id, int var_id, Game_Variables& variables) {
	if (params.string_id <= 0) {
		return -1;
	}

	std::string str = ToString(Get(params.string_id));

	params.string_id = string_out_id;

	int components = 0;

	if (delimiter.empty()) {
		// Count the characters (or the codepoints in our case)
		components = 0;
		const char* iter = str.data();
		const auto end = str.data() + str.size();

		while (iter != end) {
			const char* start_copy = iter;
			auto ret = Utils::UTF8Next(iter, end);
			iter = ret.next;

			if (iter == end) {
				break;
			}

			Set(params, std::string(start_copy, iter - start_copy));

			params.string_id++;
			components++;
		}
	} else {
		components = 1;

		if (str.find(delimiter) == std::string::npos) {
			// token not found
		} else {
			// This works for UTF-8
			std::string token;
			for (auto index = str.find(delimiter); index != std::string::npos; index = str.find(delimiter)) {
				token = str.substr(0, index);
				Set(params, token);
				params.string_id++;
				components++;
				str.erase(0, index + delimiter.length());
			}
		}
	}

	// set the remaining string
	Set(params, str);
	variables.Set(var_id, components);

	Game_Map::SetNeedRefreshForVarChange(var_id);

	return components;
}

std::string Game_Strings::FromFile(std::string_view filename, int encoding, bool& do_yield) {
	do_yield = false;

	Filesystem_Stream::InputStream is = FileFinder::OpenText(filename);
	if (!is) {
		// Emscripten: Try to async fetch the file
		auto* request = AsyncHandler::RequestFile("Text", filename);
		request->SetImportantFile(true);
		request->Start();
		do_yield = !request->IsReady();

		return {};
	}

	auto vec = Utils::ReadStream(is);
	std::string file_content(vec.begin(), vec.end());

	if (encoding == 0) {
		lcf::Encoder enc(Player::encoding);
		enc.Encode(file_content);
	} else {
		// UTF-8: Remove Byte Order Mask
		if (file_content.size() >= 3 && file_content[0] == '\xEF' && file_content[1] == '\xBB' && file_content[2] == '\xBF') {
			file_content.erase(0, 3);
		}
	}

	return file_content;
}

bool Game_Strings::ToFile(Str_Params params, std::string filename, int encoding) {
	std::string str = ToString(Get(params.string_id));

	if (params.extract) {
		filename = Extract(filename, params.hex);
	}

	// Maniacs forces the File in Text/ folder with .txt extension
	filename = "Text/" + filename;

	// EasyRPG Extension: When "*" is at the end of filename, ".txt" is not appended
	if (Player::HasEasyRpgExtensions() && filename.back() == '*') {
		filename.pop_back();
	} else {
		filename += ".txt";
	}

	filename = FileFinder::MakeCanonical(filename, 1);

	auto txt_file = FileFinder::Save().FindFile(filename);
	Filesystem_Stream::OutputStream txt_out;

	if (txt_file.empty()) {
		// File not found: Create directory hierarchy to ensure file creation succeeds
		auto txt_dir = FileFinder::GetPathAndFilename(filename).first;

		if (!txt_dir.empty() && !FileFinder::Save().MakeDirectory(txt_dir, false)) {
			Output::Warning("Maniac String Op ToFile failed. Cannot create directory {}", txt_dir);
			return false;
		}

		txt_file = filename;
	}

	txt_out = FileFinder::Save().OpenOutputStream(txt_file);
	if (!txt_out) {
		Output::Warning("Maniac String Op ToFile failed. Cannot write to {}", filename);
		return false;
	}

	if (encoding == 0) {
		lcf::Encoder enc(Player::encoding);
		enc.Decode(str);
	}

	txt_out << str;
	txt_out.Close();

	AsyncHandler::SaveFilesystem();

	return true;
}

std::string_view Game_Strings::PopLine(Str_Params params, int offset, int string_out_id) {
	// FIXME: consideration needed around encoding -- what mode are files read in?
	if (params.string_id <= 0) {
		return {};
	}

	std::string result;
	std::string_view str = Get(params.string_id);

	std::stringstream ss(ToString(str));

	while (offset >= 0 && Utils::ReadLine(ss, result)) { offset--; }

	offset = ss.rdbuf()->in_avail();

	Set(params, ss.str().substr(str.length() - offset));

	// the maniacs implementation is to always preserve the mutated base string
	// so in the case where the out_id matches the base string id, the popped line is discarded.
	if (string_out_id != params.string_id) {
		params.string_id = string_out_id;
		Set(params, result);
	}
	return Get(params.string_id);
}

std::string_view Game_Strings::ExMatch(Str_Params params, std::string expr, int var_id, int begin, int string_out_id, Game_Variables& variables) {
	// std::regex only works with char and wchar, not char32
	// For full Unicode support requires the w-API, even on non-Windows systems
	int var_result;
	std::string str_result;

	if (params.extract) {
		expr = Extract(expr, params.hex);
	}

	auto source = Get(params.string_id);
	std::string base = Substring(source, begin, Utils::UTF8Length(source));

	std::wsmatch match;
	auto wbase = Utils::ToWideString(base);
	auto wexpr = Utils::ToWideString(expr);

	std::wregex r(wexpr);

	std::regex_search(wbase, match, r);
	str_result = Utils::FromWideString(match.str());

	var_result = match.position() + begin;
	variables.Set(var_id, var_result);
	Game_Map::SetNeedRefreshForVarChange(var_id);

	if (string_out_id > 0) {
		params.string_id = string_out_id;
		Set(params, str_result);

		return Get(params.string_id);
	}
	return {};
}

const Game_Strings::Strings_t& Game_Strings::RangeOp(Str_Params params, int string_id_1, std::string string, int op, int args[], Game_Variables& variables) {
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
		std::swap(params.string_id, string_id_1);
	}

	for (int start = params.string_id; params.string_id <= string_id_1; params.string_id++) {
		switch (op) {
		case 0:  Asg(params, string); break;
		case 1:  Cat(params, string); break;
		case 2:  ToNum(params, args[0] + (params.string_id - start), variables); break;
		case 3:  GetLen(params, args[0] + (params.string_id - start), variables); break;
		case 4:  InStr(params, string, args[1], args[2], variables); break;
		case 5:  params.string_id += Split(params, string, args[1], args[2], variables); break;
		case 8:  break; // range case not applicable for popLine; see case in game_interpreter.cpp
		case 9:  ExMatch(params, string, args[1] + (params.string_id - start), args[2], -1, variables); break;
		case 10: ExMatch(params, string, args[1] + (params.string_id - start), args[2], args[3], variables); break;
		}
	}
	return GetData();
}

std::string Game_Strings::PrependMin(std::string_view string, int min_size, char c) {
	int len = Utils::UTF8Length(string);

	if (min_size < 0) {
		// Left adjust
		min_size = abs(min_size);
		if (len < min_size) {
			int s = min_size - len;
			return ToString(string) + std::string(s, c);
		}
	} else if (len < min_size) {
		// Right adjust
		int s = min_size - len;
		return std::string(s, c) + ToString(string);
	}
	return ToString(string);
}

std::string Game_Strings::Extract(std::string_view string, bool as_hex) {
	PendingMessage::CommandInserter cmd_fn;

	if (as_hex) {
		cmd_fn = ManiacsCommandInserterHex;
	} else {
		cmd_fn = ManiacsCommandInserter;
	}

	return PendingMessage::ApplyTextInsertingCommands(ToString(string), Player::escape_char, cmd_fn);
}

std::string Game_Strings::Substring(std::string_view source, int begin, int length) {
	const char* iter = source.data();
	const auto end = source.data() + source.size();

	begin = AdjustIndex(source, begin);

	if (length < 0) {
		length = 0;
	}

	// Points at start of the substring
	auto left = Utils::UTF8Skip(iter, end, begin);

	// Points at end of the substring
	auto right = Utils::UTF8Skip(left.next, end, length);

	if (right.next == nullptr) {
		return std::string(left.next, end);
	} else {
		return std::string(left.next, right.next);
	}
}

std::string Game_Strings::Insert(std::string_view source, std::string_view what, int where) {
	const char* iter = source.data();
	const auto end = source.data() + source.size();

	where = AdjustIndex(source, where);

	// Points at insertion location
	auto ret = Utils::UTF8Skip(iter, end, where);

	return std::string(source.data(), ret.next) + ToString(what) + std::string(ret.next, end);
}

std::string Game_Strings::Erase(std::string_view source, int begin, int length) {
	const char* iter = source.data();
	const auto end = source.data() + source.size();

	begin = AdjustIndex(source, begin);

	if (length < 0) {
		length = 0;
	}

	// Points at start of deletion
	auto left = Utils::UTF8Skip(iter, end, begin);

	if (left.next == nullptr) {
		return ToString(source);
	}

	// Points at end of deletion
	auto right = Utils::UTF8Skip(left.next, end, length);

	std::string ret = std::string(source.data(), left.next);
	if (right.next != nullptr) {
		ret += std::string(right.next, end);
	}

	return ret;
}

std::string Game_Strings::RegExReplace(std::string_view str, std::string_view search, std::string_view replace, std::regex_constants::match_flag_type flags) {
	// std::regex only works with char and wchar, not char32
	// For full Unicode support requires the w-API, even on non-Windows systems
	auto wstr = Utils::ToWideString(str);
	auto wsearch = Utils::ToWideString(search);
	auto wreplace = Utils::ToWideString(replace);

	std::wregex rexp(wsearch);

	auto result = std::regex_replace(wstr, rexp, wreplace, flags);

	return Utils::FromWideString(result);
}

int Game_Strings::AdjustIndex(std::string_view str, int index) {
	if (index >= 0) {
		return index;
	}

	return std::max(Utils::UTF8Length(str) - abs(index), 0);
}

std::optional<std::string> Game_Strings::ManiacsCommandInserter(char ch, const char** iter, const char* end, uint32_t escape_char) {
	if (ch == 'S' || ch == 's') {
		// \s in a normal message is the speed modifier
		// parsing a switch within an extracted string var command will parse \s[N] as a switch (ON/OFF)
		auto parse_ret = Game_Message::ParseSpeed(*iter, end, escape_char, true);
		*iter = parse_ret.next;
		int value = parse_ret.value;

		return Main_Data::game_switches->Get(value) ? "ON" : "OFF";
	} else if (ch == 'T' || ch == 't') {
		auto parse_ret = Game_Message::ParseString(*iter, end, escape_char, true);
		*iter = parse_ret.next;
		int value = parse_ret.value;

		// Contrary to Messages, the content of \t[]-strings is not evaluated
		return ToString(Main_Data::game_strings->Get(value));
	}

	return Game_Message::CommandCodeInserter(ch, iter, end, escape_char);
};

std::optional<std::string> Game_Strings::ManiacsCommandInserterHex(char ch, const char** iter, const char* end, uint32_t escape_char) {
	if (ch == 'V' || ch == 'v') {
		auto parse_ret = Game_Message::ParseVariable(*iter, end, escape_char, true);
		*iter = parse_ret.next;
		int value = parse_ret.value;

		int variable_value = Main_Data::game_variables->Get(value);
		std::ostringstream ss;
		ss << std::hex << variable_value;
		return ss.str();
	}

	return ManiacsCommandInserter(ch, iter, end, escape_char);
};

int Game_Strings::GetSizeWithLimit() {
	if (_size < 0) {
		_size = 0;
		for (auto& [index, value] : _strings) {
			assert(index > 0);
			if (index > _size) {
				_size = index;
			}
		}
	}
	return std::max(_size, static_cast<int>(lcf::Data::maniac_string_variables.size()));
}

std::string_view Game_Strings::GetName(int id) const {
	const auto* strvar = lcf::ReaderUtil::GetElement(lcf::Data::maniac_string_variables, id);

	if (!strvar) {
		return {};
	} else {
		return strvar->name;
	}
}
