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
#define _USE_MATH_DEFINES
#include "game_variables.h"
#include "output.h"
#include <lcf/reader_util.h>
#include <lcf/data.h>
#include "utils.h"
#include "rand.h"
#include <cmath>

constexpr int Game_Variables::max_warnings;
constexpr Game_Variables::Var_t Game_Variables::min_2k;
constexpr Game_Variables::Var_t Game_Variables::max_2k;
constexpr Game_Variables::Var_t Game_Variables::min_2k3;
constexpr Game_Variables::Var_t Game_Variables::max_2k3;

namespace {
using Var_t = Game_Variables::Var_t;

constexpr Var_t VarSet(Var_t o, Var_t n) {
	(void)o;
	return n;
}

constexpr Var_t VarAdd(Var_t l, Var_t r) {
	return l + r;
}

constexpr Var_t VarSub(Var_t l, Var_t r) {
	return l - r;
}

constexpr Var_t VarMult(Var_t l, Var_t r) {
	return l * r;
}

constexpr Var_t VarDiv(Var_t n, Var_t d) {
	return EP_LIKELY(d != 0) ? n / d : n;
};

constexpr Var_t VarMod(Var_t n, Var_t d) {
	return EP_LIKELY(d != 0) ? n % d : 0;
};
}

Game_Variables::Game_Variables(Var_t minval, Var_t maxval)
	: _min(minval), _max(maxval)
{
	if (minval >= maxval) {
		Output::Error("Variables: Invalid var range: [{}, {}]", minval, maxval);
	}
	_variables.reserve(lcf::Data::variables.size());
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

template <typename V, typename F>
void Game_Variables::WriteRange(const int first_id, const int last_id, V&& value, F&& op) {
	auto& vv = _variables;
	for (int i = std::max(0, first_id - 1); i < last_id; ++i) {
		auto& v = vv[i];
		v = Utils::Clamp(op(v, value()), _min, _max);
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

void Game_Variables::SetRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] = rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [this,minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarSet);
}

void Game_Variables::AddRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] += rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [this,minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarAdd);
}

void Game_Variables::SubRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] -= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [this,minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarSub);
}

void Game_Variables::MultRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] *= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [this,minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarMult);
}

void Game_Variables::DivRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] /= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [this,minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarDiv);
}

void Game_Variables::ModRangeRandom(int first_id, int last_id, Var_t minval, Var_t maxval) {
	PrepareRange(first_id, last_id, "Invalid write var[{},{}] %= rand({},{})!", minval, maxval);
	WriteRange(first_id, last_id, [this,minval,maxval](){ return Rand::GetRandomNumber(minval, maxval); }, VarMod);
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
	auto val = std::max(std::abs(_max), std::abs(_min));
	return std::log10(val) + 1;
}
