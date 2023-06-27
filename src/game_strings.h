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
#include <lcf/data.h>
#include "compiler.h"
#include "string_view.h"
#include <cstdint>
#include <string>
#include "output.h"
#include "main_data.h"
#include "game_variables.h"

/**
 * Game_Strings class.
 */
class Game_Strings {
public:
	using Str_t = lcf::DBString;
	using Strings_t = std::vector<Str_t>;

	static constexpr int max_warnings = 10;

	Game_Strings();

	void SetData(Strings_t s);
	const Strings_t& GetData() const;

	Str_t Get(int id) const;
	Str_t GetIndirect(int id) const;

	Str_t Asg(int string_id, Str_t string);
	Str_t Cat(int string_id, Str_t string);

	const Strings_t& RangeOp(int string_id_0, int string_id_1, Str_t string, int op);

	Str_t PrependMin(Str_t string, int min_size);
private:
	bool ShouldWarn(int id) const;
	void WarnGet(int id) const;

private:
	Strings_t _strings;
	mutable int _warnings = max_warnings;
};

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

inline Game_Strings::Str_t Game_Strings::GetIndirect(int id) const {
	auto val_indirect = Main_Data::game_variables->Get(id);
	return Get(static_cast<int>(val_indirect));
}
