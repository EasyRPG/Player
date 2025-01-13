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
#include "game_variables.h"
#include "output.h"
#include <lcf/reader_util.h>
#include <lcf/data.h>
#include "utils.h"
#include "rand.h"
#include <cmath>

namespace {
using Var_t = Game_Variables::Var_t;

constexpr Var_t VarSet(Var_t o, Var_t n) {
	(void)o;
	return n;
}

constexpr Var_t VarAdd(Var_t l, Var_t r) {
	Var_t res = 0;

#ifdef _MSC_VER
	res = l + r;
	if (res < 0 && l > 0 && r > 0) {
		return std::numeric_limits<Var_t>::max();
	} else if (res > 0 && l < 0 && r < 0) {
		return std::numeric_limits<Var_t>::min();
	}
#else
	if (EP_UNLIKELY(__builtin_add_overflow(l, r, &res))) {
		if (l >= 0 && r >= 0) {
			return std::numeric_limits<Var_t>::max();
		}
		return std::numeric_limits<Var_t>::min();
	}
#endif

	return res;
}

constexpr Var_t VarSub(Var_t l, Var_t r) {
	Var_t res = 0;

#ifdef _MSC_VER
	res = l - r;
	if (res < 0 && l > 0 && r < 0) {
		return std::numeric_limits<Var_t>::max();
	} else if (res > 0 && l < 0 && r > 0) {
		return std::numeric_limits<Var_t>::min();
	}
#else
	if (EP_UNLIKELY(__builtin_sub_overflow(l, r, &res))) {
		if (r < 0) {
			return std::numeric_limits<Var_t>::max();
		}
		return std::numeric_limits<Var_t>::min();
	}
#endif

	return res;
}

constexpr Var_t VarMult(Var_t l, Var_t r) {
	Var_t res = 0;

#ifdef _MSC_VER
	res = l * r;
	if (l != 0 && res / l != r) {
		if ((l > 0 && r > 0) || (l < 0 && r < 0)) {
			return std::numeric_limits<Var_t>::max();
		} else {
			return std::numeric_limits<Var_t>::min();
		}
	}
#else
	if (EP_UNLIKELY(__builtin_mul_overflow(l, r, &res))) {
		if ((l > 0 && r > 0) || (l < 0 && r < 0)) {
			return std::numeric_limits<Var_t>::max();
		}
		return std::numeric_limits<Var_t>::min();
	}
#endif

	return res;
}

constexpr Var_t VarDiv(Var_t n, Var_t d) {
	return EP_LIKELY(d != 0) ? n / d : n;
};

constexpr Var_t VarMod(Var_t n, Var_t d) {
	return EP_LIKELY(d != 0) ? n % d : 0;
};

constexpr Var_t VarBitOr(Var_t n, Var_t d) {
	return n | d;
};

constexpr Var_t VarBitAnd(Var_t n, Var_t d) {
	return n & d;
};

constexpr Var_t VarBitXor(Var_t n, Var_t d) {
	return n ^ d;
};

constexpr Var_t VarBitShiftLeft(Var_t n, Var_t d) {
	return n << d;
};

constexpr Var_t VarBitShiftRight(Var_t n, Var_t d) {
	return n >> d;
};

}

Game_Variables::Game_Variables(Var_t minval, Var_t maxval)
	: _min(minval), _max(maxval)
{
	if (minval >= maxval) {
		Output::Error("Variables: Invalid var range: [{}, {}]", minval, maxval);
	}
}

void Game_Variables::WarnGet(int variable_id) const {
	Output::Debug("Invalid read var[{}]!", variable_id);
	--_warnings;
}

template <typename F>
Game_Variables::Var_t Game_Variables::SetOp(int variable_id, Var_t value, F&& op, const char* warn) {
	if (EP_UNLIKELY(ShouldWarn(variable_id, variable_id))) {
		Output::Debug(warn, variable_id, value);
		--_warnings;
	}
	if (variable_id <= 0) {
		return 0;
	}
	if (EP_UNLIKELY(variable_id > static_cast<int>(_variables.size()))) {
		_variables.resize(variable_id, 0);
	}
	auto& v = _variables[variable_id - 1];
	value = op(v, value);
	v = Utils::Clamp(value, _min, _max);
	return v;
}

template <typename... Args>
void Game_Variables::PrepareRange(const int first_id, const int last_id, const char* warn, Args... args) {
	if (EP_UNLIKELY(ShouldWarn(first_id, last_id))) {
		Output::Debug(warn, first_id, last_id, args...);
		--_warnings;
	}
	auto& vv = _variables;
	if (EP_UNLIKELY(last_id > static_cast<int>(vv.size()))) {
		vv.resize(last_id, 0);
	}
}

template <typename... Args>
void Game_Variables::PrepareArray(const int first_id_a, const int last_id_a, const int first_id_b, const char* warn, Args... args) {
	const int last_id_b = first_id_b + last_id_a - first_id_a;
	if (EP_UNLIKELY(ShouldWarn(first_id_a, last_id_a) || ShouldWarn(first_id_b, last_id_b))) {
		Output::Debug(warn, first_id_a, last_id_a, first_id_b, last_id_b, args...);
		--_warnings;
	}
	auto& vv = _variables;
	if (EP_UNLIKELY(last_id_a > static_cast<int>(vv.size()))) {
		vv.resize(last_id_a, 0);
	}
	if (EP_UNLIKELY(last_id_b > static_cast<int>(vv.size()))) {
		vv.resize(last_id_b, 0);
	}
}

template <typename V, typename F>
void Game_Variables::WriteRange(const int first_id, const int last_id, V&& value, F&& op) {
	auto& vv = _variables;
	for (int i = std::max(0, first_id - 1); i < last_id; ++i) {
		auto& v = vv[i];
		v = Utils::Clamp(op(v, value()), _min, _max);
	}
}

template <typename F>
void Game_Variables::WriteArray(const int first_id_a, const int last_id_a, const int first_id_b, F&& op) {
	auto& vv = _variables;
	int out_b = std::max(0, first_id_b - 1);
	for (int i = std::max(0, first_id_a - 1); i < last_id_a; ++i) {
		auto& v_a = vv[i];
		auto v_b = vv[out_b++];
		v_a = Utils::Clamp(op(v_a, v_b), _min, _max);
	}
}

Game_Variables::Var_t Game_Variables::Set(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarSet, "Invalid write var[{}] = {}!");
}

Game_Variables::Var_t Game_Variables::Add(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarAdd, "Invalid write var[{}] += {}!");
}

Game_Variables::Var_t Game_Variables::Sub(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarSub, "Invalid write var[{}] -= {}!");
}

Game_Variables::Var_t Game_Variables::Mult(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarMult, "Invalid write var[{}] *= {}!");
}

Game_Variables::Var_t Game_Variables::Div(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarDiv, "Invalid write var[{}] /= {}!");
}

Game_Variables::Var_t Game_Variables::Mod(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarMod, "Invalid write var[{}] %= {}!");
}

Game_Variables::Var_t Game_Variables::BitOr(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarBitOr, "Invalid write var[{}] |= {}!");
}

Game_Variables::Var_t Game_Variables::BitAnd(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarBitAnd, "Invalid write var[{}] &= {}!");
}

Game_Variables::Var_t Game_Variables::BitXor(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarBitXor, "Invalid write var[{}] ^= {}!");
}

Game_Variables::Var_t Game_Variables::BitShiftLeft(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarBitShiftLeft, "Invalid write var[{}] <<= {}!");
}

Game_Variables::Var_t Game_Variables::BitShiftRight(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarBitShiftRight, "Invalid write var[{}] >>= {}!");
}

void Game_Variables::SetRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] = {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarSet);
}

void Game_Variables::AddRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] += {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarAdd);
}

void Game_Variables::SubRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] -= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarSub);
}

void Game_Variables::MultRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] *= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarMult);
}

void Game_Variables::DivRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] /= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarDiv);
}

void Game_Variables::ModRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] %= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarMod);
}

void Game_Variables::BitOrRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] |= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarBitOr);
}

void Game_Variables::BitAndRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] &= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarBitAnd);
}

void Game_Variables::BitXorRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] ^= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarBitXor);
}

void Game_Variables::BitShiftLeftRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] <<= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarBitShiftLeft);
}

void Game_Variables::BitShiftRightRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] >>= {}!", value);
	WriteRange(first_id, last_id, [value](){ return value; }, VarBitShiftRight);
}

template <typename F>
void Game_Variables::WriteRangeVariable(int first_id, const int last_id, const int var_id, F&& op) {
	if (var_id >= first_id && var_id <= last_id) {
		auto value = Get(var_id);
		WriteRange(first_id, var_id, [value](){ return value; }, std::forward<F>(op));
		first_id = var_id + 1;
	}
	auto value = Get(var_id);
	WriteRange(first_id, last_id, [value](){ return value; }, std::forward<F>(op));
}


void Game_Variables::SetRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] = Var({})!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarSet);
}

void Game_Variables::AddRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] += var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarAdd);
}

void Game_Variables::SubRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] -= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarSub);
}

void Game_Variables::MultRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] *= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarMult);
}

void Game_Variables::DivRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] /= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarDiv);
}

void Game_Variables::ModRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] /= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarMod);
}

void Game_Variables::BitOrRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] |= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarBitOr);
}

void Game_Variables::BitAndRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] &= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarBitAnd);
}

void Game_Variables::BitXorRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] ^= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarBitXor);
}

void Game_Variables::BitShiftLeftRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] <<= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarBitShiftLeft);
}

void Game_Variables::BitShiftRightRangeVariable(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] >>= var[{}]!", var_id);
	WriteRangeVariable(first_id, last_id, var_id, VarBitShiftRight);
}

void Game_Variables::SetRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] = var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarSet);
}

void Game_Variables::AddRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] += var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarAdd);
}

void Game_Variables::SubRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] -= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarSub);
}

void Game_Variables::MultRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] *= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarMult);
}

void Game_Variables::DivRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] /= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarDiv);
}

void Game_Variables::ModRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] %= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarMod);
}

void Game_Variables::BitOrRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] |= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarBitOr);
}

void Game_Variables::BitAndRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] &= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarBitAnd);
}

void Game_Variables::BitXorRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] ^= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarBitXor);
}

void Game_Variables::BitShiftLeftRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] <<= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarBitShiftLeft);
}

void Game_Variables::BitShiftRightRangeVariableIndirect(int first_id, int last_id, int var_id) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] >>= var[var[{}]]!", var_id);
	WriteRange(first_id, last_id, [this,var_id](){ return Get(Get(var_id)); }, VarBitShiftRight);
}

void Game_Variables::SetRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] = rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarSet);
}

void Game_Variables::AddRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] += rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarAdd);
}

void Game_Variables::SubRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] -= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarSub);
}

void Game_Variables::MultRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] *= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarMult);
}

void Game_Variables::DivRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] /= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarDiv);
}

void Game_Variables::ModRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] %= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarMod);
}

void Game_Variables::BitOrRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] |= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarBitOr);
}

void Game_Variables::BitAndRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] &= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarBitAnd);
}

void Game_Variables::BitXorRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] ^= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarBitXor);
}

void Game_Variables::BitShiftLeftRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] <<= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarBitShiftLeft);
}

void Game_Variables::BitShiftRightRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] >>= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarBitShiftRight);
}

void Game_Variables::EnumerateRange(int first_id, int last_id, Var_t value) {
	PrepareRange(first_id, last_id, "Invalid write enumerate(var[{},{}])!");
	Var_t out_value = value;
	WriteRange(first_id, last_id, [&out_value](){ return out_value++; }, VarSet);
}

void Game_Variables::SortRange(int first_id, int last_id, bool asc) {
	PrepareRange(first_id, last_id, "Invalid write sort(var[{},{}])!");
	auto& vv = _variables;
	int i = std::max(0, first_id - 1);
	if (i < last_id) {
		auto sorter = [&](auto&& fn) {
			std::stable_sort(vv.begin() + i, vv.begin() + last_id, fn);
		};
		if (asc) {
			sorter(std::less<>());
		} else {
			sorter(std::greater<>());
		}
	}
}

void Game_Variables::ShuffleRange(int first_id, int last_id) {
	PrepareRange(first_id, last_id, "Invalid write shuffle(var[{},{}])!");
	auto& vv = _variables;
	for (int i = std::max(0, first_id - 1); i < last_id; ++i) {
		int rnd_num = Rand::GetRandomNumber(first_id, last_id) - 1;
		std::swap(vv[i], vv[rnd_num]);
	}
}

void Game_Variables::SetArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] = var[{},{}]!");
	// Maniac Patch uses memcpy which is actually a memmove
	// This ensures overlapping areas are copied properly
	if (first_id_a < first_id_b) {
		WriteArray(first_id_a, last_id_a, first_id_b, VarSet);
	} else {
		auto& vv = _variables;
		const int steps = std::max(0, last_id_a - first_id_a + 1);
		int out_b = std::max(0, first_id_b + steps - 2);
		int out_a = std::max(0, last_id_a - 1);
		for (int i = 0; i < steps; ++i) {
			auto& v_a = vv[out_a--];
			auto v_b = vv[out_b--];
			v_a = Utils::Clamp(VarSet(v_a, v_b), _min, _max);
		}
	}
}

void Game_Variables::AddArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] += var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarAdd);
}

void Game_Variables::SubArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] -= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarSub);
}

void Game_Variables::MultArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] *= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarMult);
}

void Game_Variables::DivArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] /= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarDiv);
}

void Game_Variables::ModArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] %= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarMod);
}

void Game_Variables::BitOrArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] |= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarBitOr);
}

void Game_Variables::BitAndArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] &= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarBitAnd);
}

void Game_Variables::BitXorArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] ^= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarBitXor);
}

void Game_Variables::BitShiftLeftArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] <<= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarBitShiftLeft);
}

void Game_Variables::BitShiftRightArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] >>= var[{},{}]!");
	WriteArray(first_id_a, last_id_a, first_id_b, VarBitShiftRight);
}

void Game_Variables::SwapArray(int first_id_a, int last_id_a, int first_id_b) {
	PrepareArray(first_id_a, last_id_a, first_id_b, "Invalid write var[{},{}] <-> var[{},{}]!");
	auto& vv = _variables;
	const int steps = std::max(0, last_id_a - first_id_a + 1);
	int out_b = std::max(0, first_id_b + steps - 2);
	int out_a = std::max(0, last_id_a - 1);
	for (int i = 0; i < steps; ++i) {
		std::swap(vv[out_a--], vv[out_b--]);
	}
}

StringView Game_Variables::GetName(int _id) const {
	const auto* var = lcf::ReaderUtil::GetElement(lcf::Data::variables, _id);

	if (!var) {
		// No warning, is valid because the variable array resizes dynamic during runtime
		return {};
	} else {
		return var->name;
	}
}

int Game_Variables::GetMaxDigits() const {
	auto val = std::max(std::llabs(_max), std::llabs(_min));
	return static_cast<int>(std::log10(val) + 1);
}
