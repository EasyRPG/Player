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
#include "system.h"
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
#include "utils.h"

#include <regex>

#ifdef HAVE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

/**
 * Game_Strings class.
 * For all operations codepoints are used (instead of bytes).
 * This way operations that use the length or the index work almost like in Maniac Patch.
 * And using codepoints is better anyway because this is a single character.
 * With bytes you have the risk to "chop" a character in half.
 *
 * Where simple to implement UTF8 is used directly.
 * In other cases the code does a roundtrip through UTF32.
 */
class Game_Strings {
public:
	using Strings_t = std::unordered_map<int, std::string>;

	// currently only warns when ID <= 0
	static constexpr int max_warnings = 10;

	struct Str_Params {
		int string_id = 0, hex = 0, extract = 0;

		Str_Params(int string_id) : string_id(string_id) {}
		Str_Params(int string_id, int hex, int extract) : string_id(string_id), hex(hex), extract(extract) {}
	};

	enum StringEvalMode : std::int8_t { // 4 bits
		eStringEval_Text = 0, // String is taken from com.string
		eStringEval_Direct = 1, // String is referenced directly by string id
		eStringEval_Indirect = 2 // String is referenced indirectly by variable id
	};

	Game_Strings() = default;

	void SetData(Strings_t s);
	void SetData(const std::vector<lcf::DBString>& s);
	const Strings_t& GetData() const;
	std::vector<lcf::DBString> GetLcfData() const;

	std::string_view Get(int id) const;
	std::string_view GetIndirect(int id, const Game_Variables& variables) const;
	std::string_view GetWithMode(std::string_view str_data, int mode, int arg, const Game_Variables& variables) const;
	std::string GetWithModeAndPos(std::string_view str_data, int mode, int arg, int& pos, const Game_Variables& variables);

#ifdef HAVE_NLOHMANN_JSON
	nlohmann::ordered_json* ParseJson(int id);
#endif

	std::string_view Asg(Str_Params params, std::string_view string);
	std::string_view Cat(Str_Params params, std::string_view string);
	int ToNum(Str_Params params, int var_id, Game_Variables& variables);
	int GetLen(Str_Params params, int var_id, Game_Variables& variables) const;
	int InStr(Str_Params params, std::string search, int var_id, int begin, Game_Variables& variables) const;
	int Split(Str_Params params, const std::string& delimiter, int string_out_id, int var_id, Game_Variables& variables);
	static std::string FromFile(std::string_view filename, int encoding, bool& do_yield);
	std::string_view ToFile(Str_Params params, std::string filename, int encoding);
	std::string_view PopLine(Str_Params params, int offset, int string_out_id);
	std::string_view ExMatch(Str_Params params, std::string expr, int var_id, int begin, int string_out_id, Game_Variables& variables);

	const Strings_t& RangeOp(Str_Params params, int string_id_1, std::string string, int op, int args[], Game_Variables& variables);

	static std::string PrependMin(std::string_view string, int min_size, char c);
	static std::string Extract(std::string_view string, bool as_hex);
	static std::string Substring(std::string_view source, int begin, int length);
	static std::string Insert(std::string_view source, std::string_view what, int where);
	static std::string Erase(std::string_view source, int begin, int length);
	static std::string RegExReplace(std::string_view str, std::string_view search, std::string_view replace, std::regex_constants::match_flag_type flags = std::regex_constants::match_default);
	static int AdjustIndex(std::string_view str, int index);

	static std::optional<std::string> ManiacsCommandInserter(char ch, const char** iter, const char* end, uint32_t escape_char);
	static std::optional<std::string> ManiacsCommandInserterHex(char ch, const char** iter, const char* end, uint32_t escape_char);

private:
	void Set(Str_Params params, std::string_view string);
	bool ShouldWarn(int id) const;
	void WarnGet(int id) const;

	Strings_t _strings;
	mutable int _warnings = max_warnings;

#ifdef HAVE_NLOHMANN_JSON
	std::unordered_map<int, nlohmann::ordered_json> _json_cache;
#endif
};

inline void Game_Strings::Set(Str_Params params, std::string_view string) {
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

#ifdef HAVE_NLOHMANN_JSON
	_json_cache.erase(params.string_id);
#endif
}

inline void Game_Strings::SetData(Strings_t s) {
	_strings = std::move(s);

#ifdef HAVE_NLOHMANN_JSON
	_json_cache.clear();
#endif
}

inline void Game_Strings::SetData(const std::vector<lcf::DBString>& s) {
	int i = 1;
	for (const auto& string: s) {
		if (!s.empty()) {
			_strings[i] = ToString(string);
		}
		++i;
	}

#ifdef HAVE_NLOHMANN_JSON
	_json_cache.clear();
#endif
}

inline const Game_Strings::Strings_t& Game_Strings::GetData() const {
	return _strings;
}

inline std::vector<lcf::DBString> Game_Strings::GetLcfData() const {
	std::vector<lcf::DBString> lcf_data;

	for (auto& [index, value]: _strings) {
		assert(index > 0);
		if (index >= static_cast<int>(lcf_data.size())) {
			lcf_data.resize(index + 1);
		}
		lcf_data[index - 1] = lcf::DBString(value);
	}

	return lcf_data;
}

inline bool Game_Strings::ShouldWarn(int id) const {
	return id <= 0 && _warnings > 0;
}

inline std::string_view Game_Strings::Get(int id) const {
	if (EP_UNLIKELY(ShouldWarn(id))) {
		WarnGet(id);
	}
	auto it = _strings.find(id);
	if (it == _strings.end()) {
		return {};
	}
	return it->second;
}

inline std::string_view Game_Strings::GetIndirect(int id, const Game_Variables& variables) const {
	auto val_indirect = variables.Get(id);
	return Get(static_cast<int>(val_indirect));
}

inline std::string_view Game_Strings::GetWithMode(std::string_view str_data, int mode, int arg, const Game_Variables& variables) const {
	switch (mode) {
	case StringEvalMode::eStringEval_Direct:
		return Get(arg);
	case StringEvalMode::eStringEval_Indirect:
		return GetIndirect(arg, variables);
	default:
		return str_data;
	}
}

inline std::string Game_Strings::GetWithModeAndPos(std::string_view str_data, int mode, int arg, int& pos, const Game_Variables& variables) {
	std::string ret;
	switch (mode) {
		case StringEvalMode::eStringEval_Text: {
			const auto end = str_data.data() + str_data.size();

			auto left = Utils::UTF8Skip(str_data.data(), end, pos);
			auto right = Utils::UTF8Skip(left.next, end, arg);

			ret = std::string(left.next, right.next);
			pos += arg;
			return ret;
		}
		case StringEvalMode::eStringEval_Direct:
			return ToString(Get(arg));
		case StringEvalMode::eStringEval_Indirect:
			return ToString(GetIndirect(arg, variables));
		default:
			return ret;
	}
}
