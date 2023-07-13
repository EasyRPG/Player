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


#ifndef EP_MANIAC_PATCH
#define EP_MANIAC_PATCH

#include <array>
#include <cstdint>
#include "span.h"

#include "game_strings.h"

class Game_Interpreter;

namespace ManiacPatch {
	int32_t ParseExpression(Span<const int32_t> op_codes, const Game_Interpreter& interpreter);

	std::array<bool, 50> GetKeyRange();

	bool GetKeyState(uint32_t key_id);

	bool CheckString(std::string str_l, std::string str_r, int op, bool ignore_case);

	Game_Strings::Str_t GetLcfName(int data_type, int id, bool is_dynamic);
	Game_Strings::Str_t GetLcfDescription(int data_type, int id, bool is_dynamic);

	template <typename T> Game_Strings::Str_t GetNameSafely(std::vector<T> vec, int id);
	template <typename T> Game_Strings::Str_t GetDescriptionSafely(std::vector<T> vec, int id);
	template <>			  Game_Strings::Str_t GetDescriptionSafely<lcf::rpg::Actor>(std::vector<lcf::rpg::Actor> vec, int id);
}

#endif
