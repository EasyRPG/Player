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

#include "maniac_patch.h"
#include "main_data.h"
#include "game_switches.h"
#include "game_variables.h"
#include "output.h"

#include <vector>

namespace {
	enum class Op {
		Null = 0,
		U8,
		U16,
		S32,
		UX8,
		UX16,
		SX32,
		Var = 8,
		Switch,
		VarIndirect = 13,
		SwitchIndirect,
		Array = 19,
		Negate = 24,
		Not,
		Flip,
		AssignInplace = 34,
		AddInplace,
		SubInplace,
		MulInplace,
		DivInplace,
		ModInplace,
		BitOrInplace,
		BitAndInplace,
		BitXorInplace,
		BitShiftLeftInplace,
		BitShiftRightInplace,
		Add = 48,
		Sub,
		Mul,
		Div,
		Mod,
		BitOr,
		BitAnd,
		BitXor,
		BitShiftLeft,
		BitShiftRight,
		Equal,
		GreaterEqual,
		LessEqual,
		Greater,
		Less,
		NotEqual,
		Or,
		And,
		Range,
		Subscript,
		Ternary = 72,
		Function = 78
	};

	using Result = std::tuple<std::vector<uint32_t>::iterator, int>;
}

Result process(std::vector<uint32_t>::iterator it, std::vector<uint32_t>::iterator end) {
	int value = 0;
	int imm = 0;
	int imm2 = 0;
	int imm3 = 0;

	auto op = static_cast<Op>(*it);
	++it;

	// When entering the switch it is on the first argument
	switch (op) {
		case Op::Null:
			return {++it, 0};
		case Op::U8:
		case Op::UX8:
			value = *it++;
			return {it, static_cast<uint8_t>(value)};
		case Op::U16:
		case Op::UX16:
			imm = *it++;
			imm2 = *it++;
			value = (imm2 << 8) + imm;
			return {it, static_cast<uint16_t>(value)};
		case Op::S32:
		case Op::SX32:
			imm = *it++;
			imm2 = *it++;
			imm3 = *it++;
			value = *it++;
			value = (value << 24) + (imm3 << 16) + (imm2 << 8) + imm;
			return {it, value};
		case Op::Var:
			std::tie(it, imm) = process(it, end);
			return {it, Main_Data::game_variables->Get(imm)};
		case Op::Switch:
			std::tie(it, imm) = process(it, end);
			return {it, Main_Data::game_switches->GetInt(imm)};
		case Op::VarIndirect:
			std::tie(it, imm) = process(it, end);
			return {it, Main_Data::game_variables->GetIndirect(imm)};
		case Op::SwitchIndirect:
			std::tie(it, imm) = process(it, end);
			return {it, Main_Data::game_switches->GetInt(Main_Data::game_variables->Get(imm))};
		case Op::Negate:
			std::tie(it, imm) = process(it, end);
			return {it, -imm};
		case Op::Not:
			std::tie(it, imm) = process(it, end);
			return {it, !imm ? 0 : 1};
		case Op::Flip:
			std::tie(it, imm) = process(it, end);
			return {it, ~imm};
		case Op::Add:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm + imm2};
		case Op::Sub:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm - imm2};
		case Op::Mul:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm * imm2};
		case Op::Div:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			if (imm2 == 0) {
				return {it, 0};
			}
			return {it, imm / imm2};
		case Op::Mod:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			if (imm2 == 0) {
				return {it, 0};
			}
			return {it, imm % imm2};
		case Op::BitOr:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm | imm2};
		case Op::BitAnd:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm & imm2};
		case Op::BitXor:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm ^ imm2};
		case Op::BitShiftLeft:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm << imm2};
		case Op::BitShiftRight:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm >> imm2};
		case Op::Equal:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm == imm2 ? 1 : 0};
		case Op::GreaterEqual:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm >= imm2 ? 1 : 0};
		case Op::LessEqual:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm <= imm2 ? 1 : 0};
		case Op::Greater:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm > imm2 ? 1 : 0};
		case Op::Less:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm < imm2 ? 1 : 0};
		case Op::NotEqual:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, imm != imm2 ? 1 : 0};
		case Op::Or:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, !!imm || !!imm2 ? 1 : 0};
		case Op::And:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, !!imm && !!imm2 ? 1 : 0};
		case Op::Subscript:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			return {it, 0};
		case Op::Ternary:
			std::tie(it, imm) = process(it, end);
			std::tie(it, imm2) = process(it, end);
			std::tie(it, imm3) = process(it, end);
			return {it, imm != 0 ? imm2 : imm3};
		default:
			Output::Warning("Maniac: Expression contains unsupported operation {}", static_cast<int>(op));
			return {end, 0};
	}
}

int32_t ManiacPatch::ParseExpression(Span<const int32_t> op_codes) {
	std::vector<uint32_t> ops;
	for (auto &o: op_codes) {
		ops.push_back((o & 0x000000FF));
		ops.push_back((o & 0x0000FF00) >> 8);
		ops.push_back((o & 0x00FF0000) >> 16);
		ops.push_back((o & 0xFF000000) >> 24);
	}
	return std::get<int>(process(ops.begin(), ops.end()));
}