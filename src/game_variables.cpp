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
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "reader_util.h"
#include "utils.h"

constexpr int kMaxWarnings = 10;

namespace {
using Var_t = Game_Variables_Class::Var_t;

constexpr Var_t VarSet(Var_t o, Var_t n) {
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

Game_Variables_Class::Game_Variables_Class()
	: _variables(Main_Data::game_data.system.variables),
	  _warnings(kMaxWarnings)
{
}

void Game_Variables_Class::WarnGet(int variable_id) const {
	Output::Debug("Invalid read var[%d]!", variable_id);
	--_warnings;
}

template <typename F>
Game_Variables_Class::Var_t Game_Variables_Class::SetOp(int variable_id, Var_t value, F&& op, const char* warn) {
	if (EP_UNLIKELY(ShouldWarn(variable_id, variable_id))) {
		Output::Debug(warn, variable_id, value);
		--_warnings;
	}
	if (variable_id <= 0) {
		return 0;
	}
	if (EP_UNLIKELY(variable_id > _variables.size())) {
		_variables.resize(variable_id, 0);
	}
	auto& v = _variables[variable_id - 1];
	value = op(v, value);
	const auto maxval = Player::IsRPG2k3() ? 9999999 : 999999;
	const auto minval = Player::IsRPG2k3() ? -9999999 : -999999;
	v = Utils::Clamp(value, minval, maxval);
	return v;
}

template <typename F>
void Game_Variables_Class::SetOpRange(const int first_id, const int last_id, const Var_t value, F&& op, const char* warn) {
	if (EP_UNLIKELY(ShouldWarn(first_id, last_id))) {
		Output::Debug(warn, first_id, last_id, value);
		--_warnings;
	}
	auto& vv = _variables;
	if (EP_UNLIKELY(last_id > vv.size())) {
		vv.resize(last_id, 0);
	}
	const auto maxval = Player::IsRPG2k3() ? 9999999 : 999999;
	const auto minval = Player::IsRPG2k3() ? -9999999 : -999999;
	for (int i = std::max(0, first_id - 1); i < last_id; ++i) {
		auto& v = vv[i];
		v = Utils::Clamp(op(v, value), minval, maxval);
	}
}

Game_Variables_Class::Var_t Game_Variables_Class::Set(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarSet, "Invalid write var[%d] = %d!");
}

Game_Variables_Class::Var_t Game_Variables_Class::Add(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarAdd, "Invalid write var[%d] += %d!");
}

Game_Variables_Class::Var_t Game_Variables_Class::Sub(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarSub, "Invalid write var[%d] -= %d!");
}

Game_Variables_Class::Var_t Game_Variables_Class::Mult(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarMult, "Invalid write var[%d] *= %d!");
}

Game_Variables_Class::Var_t Game_Variables_Class::Div(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarDiv, "Invalid write var[%d] /= %d!");
}

Game_Variables_Class::Var_t Game_Variables_Class::Mod(int variable_id, Var_t value) {
	return SetOp(variable_id, value, VarMod, "Invalid write var[%d] %= %d!");
}

void Game_Variables_Class::SetRange(int first_id, int last_id, Var_t value) {
	SetOpRange(first_id, last_id, value, VarSet, "Invalid write var[%d,%d] = %d!");
}

void Game_Variables_Class::AddRange(int first_id, int last_id, Var_t value) {
	SetOpRange(first_id, last_id, value, VarAdd, "Invalid write var[%d,%d] += %d!");
}

void Game_Variables_Class::SubRange(int first_id, int last_id, Var_t value) {
	SetOpRange(first_id, last_id, value, VarSub, "Invalid write var[%d,%d] -= %d!");
}

void Game_Variables_Class::MultRange(int first_id, int last_id, Var_t value) {
	SetOpRange(first_id, last_id, value, VarMult, "Invalid write var[%d,%d] *= %d!");
}

void Game_Variables_Class::DivRange(int first_id, int last_id, Var_t value) {
	SetOpRange(first_id, last_id, value, VarDiv, "Invalid write var[%d,%d] /= %d!");
}

void Game_Variables_Class::ModRange(int first_id, int last_id, Var_t value) {
	SetOpRange(first_id, last_id, value, VarMod, "Invalid write var[%d,%d] %= %d!");
}

std::string Game_Variables_Class::GetName(int _id) const {
	const RPG::Variable* var = ReaderUtil::GetElement(Data::variables, _id);

	if (!var) {
		// No warning, is valid because the variable array resizes dynamic during runtime
		return "";
	} else {
		return var->name;
	}
}

void Game_Variables_Class::Reset() {
	_variables.clear();
	_warnings = kMaxWarnings;
}
