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
	using Str_t = lcf::DBString;
	using Strings_t = std::vector<Str_t>;

	static constexpr int max_warnings = 10;

	struct Str_Params {
		int string_id, hex, extract;
	};

	Game_Strings();

	void SetData(Strings_t s);
	const Strings_t& GetData() const;

	Str_t Get(int id) const;
	Str_t GetIndirect(int id) const;
	Str_t GetWithMode(Str_t str_data, int mode, int arg) const;
	Str_t GetWithModeAndPos(Str_t str_data, int mode, int arg, int* pos);

	Str_t Asg(Str_Params params, Str_t string);
	Str_t Cat(Str_Params params, Str_t string);
	int ToNum(Str_Params params, int var_id);
	int GetLen(Str_Params params, int var_id);
	int InStr(Str_Params params, std::string search, int var_id, int begin = 0);
	int Split(Str_Params params, std::string delimiter, int string_out_id, int var_id);
	Str_t PopLine(Str_Params params, int offset, int string_out_id);
	Str_t ExMatch(Str_Params params, std::string expr, int var_id, int begin, int string_out_id = -1);

	const Strings_t& RangeOp(Str_Params params, int string_id_1, Str_t string, int op, int args[] = nullptr);

	Str_t PrependMin(Str_t string, int min_size, char c);

private:
	Str_t Set(Str_Params params, Str_t string);
	bool ResizeWithId(int id);
	bool ShouldWarn(int id) const;
	void WarnGet(int id) const;
	Str_t Extract(Str_t string);

private:
	Strings_t _strings;
	mutable int _warnings = max_warnings;
};


inline Game_Strings::Str_t Game_Strings::Set(Str_Params params, Str_t string) {
	if (!ResizeWithId(params.string_id)) return "";

	auto& s = _strings[params.string_id - 1];
	s = string;
	if (params.extract)
		s = Extract(s);
	return s;
}

inline void Game_Strings::SetData(Strings_t s) {
	_strings = std::move(s);
}

inline const Game_Strings::Strings_t& Game_Strings::GetData() const {
	return _strings;
}

inline bool Game_Strings::ShouldWarn(int id) const {
	return id <= 0 && _warnings > 0;
}

inline Game_Strings::Str_t Game_Strings::Get(int id) const {
	if (EP_UNLIKELY(ShouldWarn(id))) {
		WarnGet(id);
	}
	if (id <= 0 || id > static_cast<int>(_strings.size())) {
		return "";
	}
	return _strings[id - 1];
}

inline bool Game_Strings::ResizeWithId(int id) {
	if (EP_UNLIKELY(ShouldWarn(id))) {
		WarnGet(id);
	}
	if (id <= 0) {
		return false;
	}
	if (EP_UNLIKELY(id > static_cast<int>(_strings.size()))) {
		_strings.resize(id, "");
	}
	return true;
}

inline Game_Strings::Str_t Game_Strings::GetIndirect(int id) const {
	auto val_indirect = Main_Data::game_variables->Get(id);
	return Get(static_cast<int>(val_indirect));
}

inline Game_Strings::Str_t Game_Strings::GetWithMode(Str_t str_data, int arg, int mode) const {
	switch (mode) {
	case 1: // direct string reference
		return Get(arg);
		break;
	case 2: // indirect string reference
		return GetIndirect(arg);
		break;
	}
	return str_data;
}

inline Game_Strings::Str_t Game_Strings::GetWithModeAndPos(Str_t str_data, int arg, int mode, int* pos) {
	Str_t ret;
	switch (mode) {
	case 0:
		ret = static_cast<Str_t>(static_cast<std::string>(str_data).substr(*pos, arg));
		*pos += arg;
		break;
	case 1: // direct string reference
		ret = Get(arg);
		break;
	case 2: // indirect string reference
		ret = GetIndirect(arg);
		break;
	}
	return ret;
}
