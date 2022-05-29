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
#include "input.h"

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

std::array<bool, 50> ManiacPatch::GetKeyRange() {
	std::array<Input::Keys::InputKey, 50> keys = {
		Input::Keys::A,
		Input::Keys::B,
		Input::Keys::C,
		Input::Keys::D,
		Input::Keys::E,
		Input::Keys::F,
		Input::Keys::G,
		Input::Keys::H,
		Input::Keys::I,
		Input::Keys::J,
		Input::Keys::K,
		Input::Keys::L,
		Input::Keys::M,
		Input::Keys::N,
		Input::Keys::O,
		Input::Keys::P,
		Input::Keys::Q,
		Input::Keys::R,
		Input::Keys::S,
		Input::Keys::T,
		Input::Keys::U,
		Input::Keys::V,
		Input::Keys::W,
		Input::Keys::X,
		Input::Keys::Y,
		Input::Keys::Z,
		Input::Keys::N0,
		Input::Keys::N1,
		Input::Keys::N2,
		Input::Keys::N3,
		Input::Keys::N4,
		Input::Keys::N5,
		Input::Keys::N6,
		Input::Keys::N7,
		Input::Keys::N8,
		Input::Keys::N9,
		Input::Keys::LEFT,
		Input::Keys::UP,
		Input::Keys::RIGHT,
		Input::Keys::DOWN,
		Input::Keys::RETURN,
		Input::Keys::SHIFT,
		Input::Keys::CTRL,
		Input::Keys::ALT,
		Input::Keys::SPACE,
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
		Input::Keys::MOUSE_LEFT,
		Input::Keys::MOUSE_RIGHT,
		Input::Keys::MOUSE_MIDDLE,
		Input::Keys::MOUSE_SCROLLUP,
		Input::Keys::MOUSE_SCROLLDOWN
#endif
	};

	std::array<bool, 50> pressed = {};

	for (size_t i = 0; i < pressed.size(); ++i) {
		pressed[i] = Input::IsRawKeyPressed(keys[i]);
	}

	return pressed;
}

bool ManiacPatch::GetKeyState(uint32_t key_id) {
	Input::Keys::InputKey key;

	// see https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	switch (key_id) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
		case 0x1: key = Input::Keys::MOUSE_LEFT; break;
		case 0x2: key = Input::Keys::MOUSE_RIGHT; break;
		case 0x4: key = Input::Keys::MOUSE_MIDDLE; break;
		case 0x5: key = Input::Keys::MOUSE_XBUTTON1; break;
		case 0x6: key = Input::Keys::MOUSE_XBUTTON2; break;
#endif
		case 0x8: key = Input::Keys::BACKSPACE; break;
		case 0x9: key = Input::Keys::TAB; break;
		case 0xD: key = Input::Keys::RETURN; break;
		case 0x10: key = Input::Keys::SHIFT; break;
		case 0x11: key = Input::Keys::CTRL; break;
		case 0x12: key = Input::Keys::ALT; break;
		case 0x13: key = Input::Keys::PAUSE; break;
		case 0x14: key = Input::Keys::CAPS_LOCK; break;
		case 0x1B: key = Input::Keys::ESCAPE; break;
		case 0x20: key = Input::Keys::SPACE; break;
		case 0x21: key = Input::Keys::PGUP; break;
		case 0x22: key = Input::Keys::PGDN; break;
		case 0x23: key = Input::Keys::ENDS; break;
		case 0x24: key = Input::Keys::HOME; break;
		case 0x25: key = Input::Keys::LEFT; break;
		case 0x26: key = Input::Keys::UP; break;
		case 0x27: key = Input::Keys::RIGHT; break;
		case 0x28: key = Input::Keys::DOWN; break;
		case 0x2D: key = Input::Keys::INSERT; break;
		case 0x2E: key = Input::Keys::DEL; break;
		case 0x30: key = Input::Keys::N0; break;
		case 0x31: key = Input::Keys::N1; break;
		case 0x32: key = Input::Keys::N2; break;
		case 0x33: key = Input::Keys::N3; break;
		case 0x34: key = Input::Keys::N4; break;
		case 0x35: key = Input::Keys::N5; break;
		case 0x36: key = Input::Keys::N6; break;
		case 0x37: key = Input::Keys::N7; break;
		case 0x38: key = Input::Keys::N8; break;
		case 0x39: key = Input::Keys::N9; break;
		case 0x41: key = Input::Keys::A; break;
		case 0x42: key = Input::Keys::B; break;
		case 0x43: key = Input::Keys::C; break;
		case 0x44: key = Input::Keys::D; break;
		case 0x45: key = Input::Keys::E; break;
		case 0x46: key = Input::Keys::F; break;
		case 0x47: key = Input::Keys::G; break;
		case 0x48: key = Input::Keys::H; break;
		case 0x49: key = Input::Keys::I; break;
		case 0x4A: key = Input::Keys::J; break;
		case 0x4B: key = Input::Keys::K; break;
		case 0x4C: key = Input::Keys::L; break;
		case 0x4D: key = Input::Keys::M; break;
		case 0x4E: key = Input::Keys::N; break;
		case 0x4F: key = Input::Keys::O; break;
		case 0x50: key = Input::Keys::P; break;
		case 0x51: key = Input::Keys::Q; break;
		case 0x52: key = Input::Keys::R; break;
		case 0x53: key = Input::Keys::S; break;
		case 0x54: key = Input::Keys::T; break;
		case 0x55: key = Input::Keys::U; break;
		case 0x56: key = Input::Keys::V; break;
		case 0x57: key = Input::Keys::W; break;
		case 0x58: key = Input::Keys::X; break;
		case 0x59: key = Input::Keys::Y; break;
		case 0x5A: key = Input::Keys::Z; break;
		case 0x60: key = Input::Keys::KP0; break;
		case 0x61: key = Input::Keys::KP1; break;
		case 0x62: key = Input::Keys::KP2; break;
		case 0x63: key = Input::Keys::KP3; break;
		case 0x64: key = Input::Keys::KP4; break;
		case 0x65: key = Input::Keys::KP5; break;
		case 0x66: key = Input::Keys::KP6; break;
		case 0x67: key = Input::Keys::KP7; break;
		case 0x68: key = Input::Keys::KP8; break;
		case 0x69: key = Input::Keys::KP9; break;
		case 0x6A: key = Input::Keys::KP_MULTIPLY; break;
		case 0x6B: key = Input::Keys::KP_ADD; break;
		case 0x6D: key = Input::Keys::KP_SUBTRACT; break;
		case 0x6E: key = Input::Keys::KP_PERIOD; break;
		case 0x6F: key = Input::Keys::KP_DIVIDE; break;
		case 0x70: key = Input::Keys::F1; break;
		case 0x71: key = Input::Keys::F2; break;
		case 0x72: key = Input::Keys::F3; break;
		case 0x73: key = Input::Keys::F4; break;
		case 0x74: key = Input::Keys::F5; break;
		case 0x75: key = Input::Keys::F6; break;
		case 0x76: key = Input::Keys::F7; break;
		case 0x77: key = Input::Keys::F8; break;
		case 0x78: key = Input::Keys::F9; break;
		case 0x79: key = Input::Keys::F10; break;
		case 0x7A: key = Input::Keys::F11; break;
		case 0x7B: key = Input::Keys::F12; break;
		case 0x90: key = Input::Keys::NUM_LOCK; break;
		case 0x91: key = Input::Keys::SCROLL_LOCK; break;
		case 0xA0: key = Input::Keys::LSHIFT; break;
		case 0xA1: key = Input::Keys::RSHIFT; break;
		case 0xA2: key = Input::Keys::LCTRL; break;
		case 0xA3: key = Input::Keys::RCTRL; break;
		default:
			Output::Debug("Maniac KeyInputProcEx: Unsupported keycode {}", key_id);
			key = Input::Keys::NONE;
			break;
	}

	return Input::IsRawKeyPressed(key);
}
