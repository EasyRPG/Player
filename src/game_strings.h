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
#include <cstdint>
#include <string>
#include <lcf/data.h>
#include "compiler.h"
#include "game_variables.h"
#include "main_data.h"
#include "output.h"
#include "pending_message.h"
#include "player.h"
#include "string_view.h"

/**
 * Game_Strings class.
 */
class Game_Strings {
public:
	using Strings_t = std::unordered_map<int, std::string>;

	// currently only warns when ID <= 0
	static constexpr int max_warnings = 10;

	struct Str_Params {
		int string_id = 0, hex = 0, extract = 0;
	};

	Game_Strings() = default;

	void SetData(Strings_t s);
	void SetData(const std::vector<lcf::DBString>& s);
	const Strings_t& GetData() const;
	std::vector<lcf::DBString> GetLcfData() const;

	StringView Get(int id) const;
	StringView GetIndirect(int id, const Game_Variables& variables) const;
	StringView GetWithMode(StringView str_data, int mode, int arg, const Game_Variables& variables) const;
	StringView GetWithModeAndPos(StringView str_data, int mode, int arg, int* pos, const Game_Variables& variables);

	StringView Asg(Str_Params params, StringView string);
	StringView Cat(Str_Params params, StringView string);
	int ToNum(Str_Params params, int var_id, Game_Variables& variables);
	int GetLen(Str_Params params, int var_id, Game_Variables& variables) const;
	int InStr(Str_Params params, std::string search, int var_id, int begin, Game_Variables& variables) const;
	int Split(Str_Params params, const std::string& delimiter, int string_out_id, int var_id, Game_Variables& variables);
	static std::string FromFile(StringView filename, int encoding, bool& do_yield);
	StringView ToFile(Str_Params params, std::string filename, int encoding);
	StringView PopLine(Str_Params params, int offset, int string_out_id);
	StringView ExMatch(Str_Params params, std::string expr, int var_id, int begin, int string_out_id, Game_Variables& variables);

	const Strings_t& RangeOp(Str_Params params, int string_id_1, std::string string, int op, int args[], Game_Variables& variables);

	static std::string PrependMin(StringView string, int min_size, char c);
	static std::string Extract(StringView string, bool as_hex);

	static std::optional<std::string> ManiacsCommandInserter(char ch, const char** iter, const char* end, uint32_t escape_char);
	static std::optional<std::string> ManiacsCommandInserterHex(char ch, const char** iter, const char* end, uint32_t escape_char);

private:
	void Set(Str_Params params, StringView string);
	bool ShouldWarn(int id) const;
	void WarnGet(int id) const;

	Strings_t _strings;
	mutable int _warnings = max_warnings;
};


inline void Game_Strings::Set(Str_Params params, StringView string) {
	if (params.string_id <= 0) {
		return;
	}

	std::string ins_string = ToString(string);
	if (params.extract) {
		ins_string = Extract(ins_string, params.hex);
	}

	auto it = _strings.find(params.string_id);
	if (it == _strings.end()) {
		if (ins_string.empty()) {
			return;
		} else {
			_strings[params.string_id] = ins_string;
		}
	} else {
		it->second = ins_string;
	}
}

inline void Game_Strings::SetData(Strings_t s) {
	_strings = std::move(s);
}

inline void Game_Strings::SetData(const std::vector<lcf::DBString>& s) {
	int i = 1;
	for (const auto& string: s) {
		if (!s.empty()) {
			_strings[i] = ToString(string);
		}
		++i;
	}
}

inline const Game_Strings::Strings_t& Game_Strings::GetData() const {
	return _strings;
}

inline std::vector<lcf::DBString> Game_Strings::GetLcfData() const {
	std::vector<lcf::DBString> lcf_data;

	for (auto& [index, value]: _strings) {
		assert(index > 0);
		if (index >= lcf_data.size()) {
			lcf_data.resize(index + 1);
		}
		lcf_data[index - 1] = lcf::DBString(value);
	}

	return lcf_data;
}

inline bool Game_Strings::ShouldWarn(int id) const {
	return id <= 0 && _warnings > 0;
}

inline StringView Game_Strings::Get(int id) const {
	if (EP_UNLIKELY(ShouldWarn(id))) {
		WarnGet(id);
	}
	auto it = _strings.find(id);
	if (it == _strings.end()) {
		return {};
	}
	return it->second;
}

inline StringView Game_Strings::GetIndirect(int id, const Game_Variables& variables) const {
	auto val_indirect = variables.Get(id);
	return Get(static_cast<int>(val_indirect));
}

inline StringView Game_Strings::GetWithMode(StringView str_data, int mode, int arg, const Game_Variables& variables) const {
	switch (mode) {
	case 1: // direct string reference
		return Get(arg);
	case 2: // indirect string reference
		return GetIndirect(arg, variables);
	default:
		return str_data;
	}
}

inline StringView Game_Strings::GetWithModeAndPos(StringView str_data, int mode, int arg, int* pos, const Game_Variables& variables) {
	StringView ret;
	switch (mode) {
	case 0:
		assert(pos);
		ret = str_data.substr(*pos, arg);
		*pos += arg;
		return ret;
	case 1: // direct string reference
		return Get(arg);
	case 2: // indirect string reference
		return GetIndirect(arg, variables);
	default:
		return ret;
	}
}
