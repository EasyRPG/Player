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

#ifndef EP_GAME_VARIABLES_H
#define EP_GAME_VARIABLES_H

// Headers
#include <lcf/data.h>
#include "compiler.h"
#include "string_view.h"
#include <cstdint>
#include <string>

/**
 * Game_Variables class.
 */
class Game_Variables {
public:
	using Var_t = int32_t;
	using Variables_t = std::vector<Var_t>;

	static constexpr int max_warnings = 10;
	static constexpr Var_t min_2k = -999999;
	static constexpr Var_t max_2k = 999999;
	static constexpr Var_t min_2k3 = -9999999;
	static constexpr Var_t max_2k3 = 9999999;

	Game_Variables(Var_t minval, Var_t maxval);

	void SetData(Variables_t);
	const Variables_t& GetData() const;

	void SetLowerLimit(size_t limit);

	Var_t Get(int variable_id) const;
	Var_t GetIndirect(int variable_id) const;
	Var_t GetWithMode(int id, int mode) const;

	Var_t Set(int variable_id, Var_t value);
	Var_t Add(int variable_id, Var_t value);
	Var_t Sub(int variable_id, Var_t value);
	Var_t Mult(int variable_id, Var_t value);
	Var_t Div(int variable_id, Var_t value);
	Var_t Mod(int variable_id, Var_t value);
	Var_t BitOr(int variable_id, Var_t value);
	Var_t BitAnd(int variable_id, Var_t value);
	Var_t BitXor(int variable_id, Var_t value);
	Var_t BitShiftLeft(int variable_id, Var_t value);
	Var_t BitShiftRight(int variable_id, Var_t value);

	void SetRange(int first_id, int last_id, Var_t value);
	void AddRange(int first_id, int last_id, Var_t value);
	void SubRange(int first_id, int last_id, Var_t value);
	void MultRange(int first_id, int last_id, Var_t value);
	void DivRange(int first_id, int last_id, Var_t value);
	void ModRange(int first_id, int last_id, Var_t value);
	void BitOrRange(int first_id, int last_id, Var_t value);
	void BitAndRange(int first_id, int last_id, Var_t value);
	void BitXorRange(int first_id, int last_id, Var_t value);
	void BitShiftLeftRange(int first_id, int last_id, Var_t value);
	void BitShiftRightRange(int first_id, int last_id, Var_t value);

	void SetRangeVariable(int first_id, int last_id, int var_id);
	void AddRangeVariable(int first_id, int last_id, int var_id);
	void SubRangeVariable(int first_id, int last_id, int var_id);
	void MultRangeVariable(int first_id, int last_id, int var_id);
	void DivRangeVariable(int first_id, int last_id, int var_id);
	void ModRangeVariable(int first_id, int last_id, int var_id);
	void BitOrRangeVariable(int first_id, int last_id, int var_id);
	void BitAndRangeVariable(int first_id, int last_id, int var_id);
	void BitXorRangeVariable(int first_id, int last_id, int var_id);
	void BitShiftLeftRangeVariable(int first_id, int last_id, int var_id);
	void BitShiftRightRangeVariable(int first_id, int last_id, int var_id);

	void SetRangeVariableIndirect(int first_id, int last_id, int var_id);
	void AddRangeVariableIndirect(int first_id, int last_id, int var_id);
	void SubRangeVariableIndirect(int first_id, int last_id, int var_id);
	void MultRangeVariableIndirect(int first_id, int last_id, int var_id);
	void DivRangeVariableIndirect(int first_id, int last_id, int var_id);
	void ModRangeVariableIndirect(int first_id, int last_id, int var_id);
	void BitOrRangeVariableIndirect(int first_id, int last_id, int var_id);
	void BitAndRangeVariableIndirect(int first_id, int last_id, int var_id);
	void BitXorRangeVariableIndirect(int first_id, int last_id, int var_id);
	void BitShiftLeftRangeVariableIndirect(int first_id, int last_id, int var_id);
	void BitShiftRightRangeVariableIndirect(int first_id, int last_id, int var_id);

	void SetRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void AddRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void SubRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void MultRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void DivRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void ModRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void BitOrRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void BitAndRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void BitXorRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void BitShiftLeftRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);
	void BitShiftRightRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval);

	void EnumerateRange(int first_id, int last_id, Var_t value);
	void SortRange(int first_id, int last_id, bool asc);
	void ShuffleRange(int first_id, int last_id);

	void SetArray(int first_id_a, int last_id_a, int first_id_b);
	void AddArray(int first_id_a, int last_id_a, int first_id_b);
	void SubArray(int first_id_a, int last_id_a, int first_id_b);
	void MultArray(int first_id_a, int last_id_a, int first_id_b);
	void DivArray(int first_id_a, int last_id_a, int first_id_b);
	void ModArray(int first_id_a, int last_id_a, int first_id_b);
	void BitOrArray(int first_id_a, int last_id_a, int first_id_b);
	void BitAndArray(int first_id_a, int last_id_a, int first_id_b);
	void BitXorArray(int first_id_a, int last_id_a, int first_id_b);
	void BitShiftLeftArray(int first_id_a, int last_id_a, int first_id_b);
	void BitShiftRightArray(int first_id_a, int last_id_a, int first_id_b);
	void SwapArray(int first_id_a, int last_id_a, int first_id_b);

	StringView GetName(int _id) const;

	int GetSize() const;
	int GetSizeWithLimit() const;

	bool IsValid(int variable_id) const;

	void SetWarning(int w);

	Var_t GetMaxValue() const;
	Var_t GetMinValue() const;

	int GetMaxDigits() const;
private:
	bool ShouldWarn(int first_id, int last_id) const;
	void WarnGet(int variable_id) const;
	template <typename F>
		Var_t SetOp(int variable_id, Var_t value, F&& op, const char* warn);
	template <typename... Args>
		void PrepareRange(const int first_id, const int last_id, const char* warn, Args... args);
	template <typename... Args>
		void PrepareArray(const int first_id_a, const int last_id_a, const int first_id_b, const char* warn, Args... args);
	template <typename V, typename F>
		void WriteRange(const int first_id, const int last_id, V&& value, F&& op);
	template <typename F>
		void WriteRangeVariable(const int first_id, const int last_id, int var_id, F&& op);
	template <typename F>
		void WriteArray(const int first_id_a, const int last_id_a, const int first_id_b, F&& op);

	Variables_t _variables;
	Var_t _min = 0;
	Var_t _max = 0;
	size_t lower_limit = 0;
	mutable int _warnings = max_warnings;
};

inline void Game_Variables::SetData(Variables_t v) {
	_variables = std::move(v);
}

inline const Game_Variables::Variables_t& Game_Variables::GetData() const {
	return _variables;
}

inline void Game_Variables::SetLowerLimit(size_t limit) {
	lower_limit = limit;
}

inline int Game_Variables::GetSize() const {
	return static_cast<int>(_variables.size());
}

inline int Game_Variables::GetSizeWithLimit() const {
	return std::max<int>(lower_limit, _variables.size());
}

inline bool Game_Variables::IsValid(int variable_id) const {
	return variable_id > 0 && variable_id <= GetSizeWithLimit();
}

inline bool Game_Variables::ShouldWarn(int first_id, int last_id) const {
	return (first_id <= 0 || last_id > GetSizeWithLimit()) && _warnings > 0;
}

inline Game_Variables::Var_t Game_Variables::Get(int variable_id) const {
	if (EP_UNLIKELY(ShouldWarn(variable_id, variable_id))) {
		WarnGet(variable_id);
	}
	if (variable_id <= 0 || variable_id > static_cast<int>(_variables.size())) {
		return 0;
	}
	return _variables[variable_id - 1];
}

inline Game_Variables::Var_t Game_Variables::GetIndirect(int variable_id) const {
	auto val_indirect = Get(variable_id);
	return Get(static_cast<int>(val_indirect));
}

// mode 0: pass through id value
// mode 1: get by id
// mode 2: get indirect by id
inline Game_Variables::Var_t Game_Variables::GetWithMode(int id, int mode) const {
	switch (mode) {
	case 1: return Get(id); break;
	case 2: return GetIndirect(id); break;
	}
	return id;
}

inline void Game_Variables::SetWarning(int w) {
	_warnings = w;
}

inline Game_Variables::Var_t Game_Variables::GetMaxValue() const {
	return _max;
}

inline Game_Variables::Var_t Game_Variables::GetMinValue() const {
	return _min;
}

#endif
