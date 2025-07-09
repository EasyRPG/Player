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

#include "filesystem_stream.h"
#include "input.h"
#include "game_actors.h"
#include "game_interpreter_control_variables.h"
#include "game_map.h"
#include "game_interpreter.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "main_data.h"
#include "output.h"
#include "player.h"

#include <lcf/reader_lcf.h>
#include <lcf/reader_util.h>
#include <lcf/writer_lcf.h>
#include <vector>

/*
The following operations are unsupported:

All array functions (Array, Range and Subscript):
They could be implemented but are not very useful

All Inplace functions:
These functions are disabled when EasyRpg Extensions are active.
Inplace assigns to variables while the ControlVariables event command is executed.
This violates how the command is supposed to work because more variables than the target variables can be set.
*/

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

	enum class Fn {
		Rand = 0,
		Item,
		Event,
		Actor,
		Party,
		Enemy,
		Misc,
		Pow,
		Sqrt,
		Sin,
		Cos,
		Atan2,
		Min,
		Max,
		Abs,
		Clamp,
		Muldiv,
		Divmul,
		Between
	};

	bool global_save_opened = false;
}

struct ProcessAssignmentRet {
	Op op = Op::Null;
	int id = 0;

	int fetch() const {
		switch (op) {
		case Op::Var:
			return Main_Data::game_variables->Get(id);
		case Op::Switch:
			return Main_Data::game_switches->Get(id);
		case Op::VarIndirect: {
			return Main_Data::game_variables->GetIndirect(id);
		}
		case Op::SwitchIndirect: {
			int var = Main_Data::game_variables->GetIndirect(id);
			return Main_Data::game_switches->Get(var);
		}
		default:
			Output::Warning("Maniac: Expression assignment {} is not a lvalue", static_cast<int>(op));
			return 0;
		}
	}

	int assign(int value) const {
		if (Player::HasEasyRpgExtensions()) {
			Output::Warning("Maniac: Inplace assignments are not allowed in expressions when running in EasyRpg Mode");
			return fetch();
		}

		switch (op) {
		case Op::Var:
			Game_Map::SetNeedRefreshForVarChange(id);
			return Main_Data::game_variables->Set(id, value);
		case Op::Switch:
			Game_Map::SetNeedRefreshForSwitchChange(id);
			return Main_Data::game_switches->Set(id, value > 0);
		case Op::VarIndirect: {
			int var = Main_Data::game_variables->GetIndirect(id);
			Game_Map::SetNeedRefreshForVarChange(var);
			return Main_Data::game_variables->Set(var, value);
		}
		case Op::SwitchIndirect: {
			int var = Main_Data::game_variables->GetIndirect(id);
			Game_Map::SetNeedRefreshForSwitchChange(var);
			return Main_Data::game_switches->Set(var, value > 0);
		}
		default:
			Output::Warning("Maniac: Expression assignment {} is not a lvalue", static_cast<int>(op));
			return 0;
		}
	}
};

ProcessAssignmentRet ProcessAssignment(std::vector<int32_t>::iterator& it, std::vector<int32_t>::iterator end, const Game_BaseInterpreterContext& ip);

int Process(std::vector<int32_t>::iterator& it, std::vector<int32_t>::iterator end, const Game_BaseInterpreterContext& ip) {
	int value = 0;
	int imm = 0;
	int imm2 = 0;
	int imm3 = 0;

	if (it == end) {
		return 0;
	}

	auto op = static_cast<Op>(*it);
	++it;

	// When entering the switch it is on the first argument
	switch (op) {
		case Op::Null:
			it++;
			return 0;
		case Op::U8:
		case Op::UX8:
			value = *it++;
			return value;
		case Op::U16:
		case Op::UX16:
			imm = *it++;
			if (it == end) {
				return 0;
			}
			imm2 = *it++;
			value = (imm2 << 8) + imm;
			return value;
		case Op::S32:
		case Op::SX32:
			imm = *it++;
			if (it == end) {
				return 0;
			}
			imm2 = *it++;
			if (it == end) {
				return 0;
			}
			imm3 = *it++;
			if (it == end) {
				return 0;
			}
			value = *it++;
			value = (value << 24) + (imm3 << 16) + (imm2 << 8) + imm;
			return value;
		case Op::Var:
			imm = Process(it, end, ip);
			return Main_Data::game_variables->Get(imm);
		case Op::Switch:
			imm = Process(it, end, ip);
			return Main_Data::game_switches->GetInt(imm);
		case Op::VarIndirect:
			imm = Process(it, end, ip);
			return Main_Data::game_variables->GetIndirect(imm);
		case Op::SwitchIndirect:
			imm = Process(it, end, ip);
			return Main_Data::game_switches->GetInt(Main_Data::game_variables->Get(imm));
		case Op::Negate:
			imm = Process(it, end, ip);
			return -imm;
		case Op::Not:
			imm = Process(it, end, ip);
			return !imm ? 0 : 1;
		case Op::Flip:
			imm = Process(it, end, ip);
			return ~imm;
		case Op::AssignInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(imm2);
		}
		case Op::AddInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(ret.fetch()) + imm2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max())));
		}
		case Op::SubInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(ret.fetch()) - imm2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max())));
		}
		case Op::MulInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(ret.fetch()) * imm2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max())));
		}
		case Op::DivInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			if (imm2 == 0) {
				return ret.fetch();
			}
			return ret.assign(ret.fetch() / imm2);
		}
		case Op::ModInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			if (imm2 == 0) {
				return ret.fetch();
			}
			return ret.assign(ret.fetch() % imm2);
		}
		case Op::BitOrInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(ret.fetch() | imm2);
		}
		case Op::BitAndInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(ret.fetch() & imm2);
		}
		case Op::BitXorInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(ret.fetch() ^ imm2);
		}
		case Op::BitShiftLeftInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(ret.fetch() << imm2);
		}
		case Op::BitShiftRightInplace: {
			auto ret = ProcessAssignment(it, end, ip);
			imm2 = Process(it, end, ip);
			return ret.assign(ret.fetch() >> imm2);
		}
		case Op::Add:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(imm) + imm2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
		case Op::Sub:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(imm) - imm2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
		case Op::Mul:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(imm) * imm2, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
		case Op::Div:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			if (imm2 == 0) {
				return imm;
			}
			return imm / imm2;
		case Op::Mod:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			if (imm2 == 0) {
				return imm;
			}
			return imm % imm2;
		case Op::BitOr:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm | imm2;
		case Op::BitAnd:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm & imm2;
		case Op::BitXor:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm ^ imm2;
		case Op::BitShiftLeft:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm << imm2;
		case Op::BitShiftRight:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm >> imm2;
		case Op::Equal:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm == imm2 ? 1 : 0;
		case Op::GreaterEqual:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm >= imm2 ? 1 : 0;
		case Op::LessEqual:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm <= imm2 ? 1 : 0;
		case Op::Greater:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm > imm2 ? 1 : 0;
		case Op::Less:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm < imm2 ? 1 : 0;
		case Op::NotEqual:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return imm != imm2 ? 1 : 0;
		case Op::Or:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return !!imm || !!imm2 ? 1 : 0;
		case Op::And:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			return !!imm && !!imm2 ? 1 : 0;
		case Op::Ternary:
			imm = Process(it, end, ip);
			imm2 = Process(it, end, ip);
			imm3 = Process(it, end, ip);
			return imm != 0 ? imm2 : imm3;
		case Op::Function:
			imm = *it++; // function
			imm2 = *it++; // arguments

			if ((imm2 & 0x80) != 0) {
				// Argument count is 4 bytes, that mode is not supported
				Output::Warning("Maniac: Expression func long args unsupported");
				return 0;
			}

			switch (static_cast<Fn>(imm)) {
				case Fn::Rand:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression rnd args {} != 2", imm2);
						return 0;
					}
					imm3 = Process(it, end, ip);
					return ControlVariables::Random(Process(it, end, ip), imm3);
				case Fn::Item:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression item args {} != 2", imm2);
						return 0;
					}
					imm3 = Process(it, end, ip);
					return ControlVariables::Item(Process(it, end, ip), imm3);
				case Fn::Event:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression event args {} != 2", imm2);
						return 0;
					}
					imm3 = Process(it, end, ip);
					return ControlVariables::Event(Process(it, end, ip), imm3, ip);
				case Fn::Actor:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression actor args {} != 2", imm2);
						return 0;
					}
					imm3 = Process(it, end, ip);
					return ControlVariables::Actor(Process(it, end, ip), imm3);
				case Fn::Party:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression member args {} != 2", imm2);
						return 0;
					}
					imm3 = Process(it, end, ip);
					return ControlVariables::Party(Process(it, end, ip), imm3);
				case Fn::Enemy:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression enemy args {} != 2", imm2);
						return 0;
					}
					imm3 = Process(it, end, ip);
					return ControlVariables::Enemy(Process(it, end, ip), imm3);
					break;
				case Fn::Misc:
					if (imm2 != 1) {
						Output::Warning("Maniac: Expression misc args {} != 1", imm2);
						return 0;
					}
					return ControlVariables::Other(Process(it, end, ip));
				case Fn::Pow:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression pow args {} != 2", imm2);
						return 0;
					}
					return ControlVariables::Pow(Process(it, end, ip), Process(it, end, ip));
				case Fn::Sqrt:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression sqrt args {} != 2", imm2);
						return 0;
					}
					return ControlVariables::Sqrt(Process(it, end, ip), Process(it, end, ip));
				case Fn::Sin:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression sin args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Sin(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				case Fn::Cos:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression cos args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Cos(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				case Fn::Atan2:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression atan2 args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Atan2(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				case Fn::Min:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression min args {} != 2", imm2);
						return 0;
					}
					return ControlVariables::Min(Process(it, end, ip), Process(it, end, ip));
				case Fn::Max:
					if (imm2 != 2) {
						Output::Warning("Maniac: Expression max args {} != 2", imm2);
						return 0;
					}
					return ControlVariables::Max(Process(it, end, ip), Process(it, end, ip));
				case Fn::Abs:
					if (imm2 != 1) {
						Output::Warning("Maniac: Expression abs args {} != 1", imm2);
						return 0;
					}
					return ControlVariables::Abs(Process(it, end, ip));
				case Fn::Clamp:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression clamp args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Clamp(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				case Fn::Muldiv:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression muldiv args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Muldiv(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				case Fn::Divmul:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression divmul args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Divmul(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				case Fn::Between:
					if (imm2 != 3) {
						Output::Warning("Maniac: Expression between args {} != 3", imm2);
						return 0;
					}
					return ControlVariables::Between(Process(it, end, ip), Process(it, end, ip), Process(it, end, ip));
				default:
					Output::Warning("Maniac: Expression Unknown Func {}", imm);
					for (int i = 0; i < imm2; ++i) {
						Process(it, end, ip);
					}
					return 0;
			}
		default:
			Output::Warning("Maniac: Expression contains unsupported operation {}", static_cast<int>(op));
			return 0;
	}
}

ProcessAssignmentRet ProcessAssignment(std::vector<int32_t>::iterator& it, std::vector<int32_t>::iterator end, const Game_BaseInterpreterContext& ip) {
	// Like process but it remembers the type (Variable or Switch) without evaluating it to allow assignments
	int imm = 0;

	if (it == end) {
		return {Op::Null, 0};
	}

	auto op = static_cast<Op>(*it);
	++it;

	// When entering the switch it is on the first argument
	switch (op) {
		case Op::Var:
		case Op::Switch:
		case Op::VarIndirect:
		case Op::SwitchIndirect:
			imm = Process(it, end, ip);
			return {op, imm};
		default:
			--it; // back on the op as op is fetched again by Process
			imm = Process(it, end, ip);
			return {op, imm};
	}
}

int32_t ManiacPatch::ParseExpression(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter) {
	std::vector<int32_t> ops;
	for (auto &o: op_codes) {
		auto uo = static_cast<uint32_t>(o);
		ops.push_back(static_cast<int32_t>(uo & 0x000000FF));
		ops.push_back(static_cast<int32_t>((uo & 0x0000FF00) >> 8));
		ops.push_back(static_cast<int32_t>((uo & 0x00FF0000) >> 16));
		ops.push_back(static_cast<int32_t>((uo & 0xFF000000) >> 24));
	}
	auto beg = ops.begin();
	return Process(beg, ops.end(), interpreter);
}

std::vector<int32_t> ManiacPatch::ParseExpressions(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter) {
	std::vector<int32_t> ops;
	for (auto& o : op_codes) {
		auto uo = static_cast<uint32_t>(o);
		ops.push_back(static_cast<int32_t>(uo & 0x000000FF));
		ops.push_back(static_cast<int32_t>((uo & 0x0000FF00) >> 8));
		ops.push_back(static_cast<int32_t>((uo & 0x00FF0000) >> 16));
		ops.push_back(static_cast<int32_t>((uo & 0xFF000000) >> 24));
	}

	if (ops.empty()) {
		return {};
	}

	auto it = ops.begin();

	std::vector<int32_t> results;

	while (true) {
		results.push_back(Process(it, ops.end(), interpreter));

		if (it == ops.end() || static_cast<Op>(*it) == Op::Null) {
			break;
		}
	}

	return results;
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
#else
		Input::Keys::NONE,
		Input::Keys::NONE,
		Input::Keys::NONE,
		Input::Keys::NONE,
		Input::Keys::NONE
#endif
	};

	std::array<bool, 50> pressed = {};

	for (size_t i = 0; i < pressed.size(); ++i) {
		pressed[i] = Input::IsRawKeyPressed(keys[i]);
	}

	if (!pressed[41]) {
		pressed[41] = Input::IsRawKeyPressed(Input::Keys::LSHIFT) || Input::IsRawKeyPressed(Input::Keys::RSHIFT);
	}
	if (!pressed[42]) {
		pressed[42] = Input::IsRawKeyPressed(Input::Keys::LCTRL) || Input::IsRawKeyPressed(Input::Keys::RCTRL);
	}
	if (!pressed[43]) {
		pressed[43] = Input::IsRawKeyPressed(Input::Keys::LALT) || Input::IsRawKeyPressed(Input::Keys::RALT);
	}

	return pressed;
}

bool ManiacPatch::CheckString(std::string_view str_l, std::string_view str_r, int op, bool ignore_case) {
	auto check = [op](const auto& l, const auto& r) {
		switch (op) {
			case 0: // eq
				return l == r;
			case 2: // contains (l contains r)
				return l.find(r) != std::string::npos;
			case 1: // neq
				return l != r;
			case 3: // notContains (l does not contain r)
				return l.find(r) == std::string::npos;
			default:
				return false;
		}
	};

	if (ignore_case) {
		std::string str_l_lower = Utils::LowerCase(str_l);
		std::string str_r_lower = Utils::LowerCase(str_r);
		return check(str_l_lower, str_r_lower);
	}

	return check(str_l, str_r);
}

std::string_view ManiacPatch::GetLcfName(int data_type, int id, bool is_dynamic) {
	auto get_name = [&id](std::string_view type, const auto& vec) -> std::string_view {
		auto* data = lcf::ReaderUtil::GetElement(vec, id);
		if (!data) {
			Output::Warning("Unable to read {} name: {}", type, id);
			return {};
		}
		return data->name;
	};

	switch (data_type)
	{
	case 0:  //.actor[a].name
		if (is_dynamic) {
			auto actor = Main_Data::game_actors->GetActor(id);
			if (actor != nullptr) {
				return actor->GetName();
			}
		}
		else {
			return get_name("Actor", lcf::Data::actors);
		}
		break;
	case 1:	 return get_name("Skill", lcf::Data::skills);   //.skill[a].name
	case 2:	 return get_name("Item", lcf::Data::items);   //.item[a].name
	case 3:	 return get_name("Enemy", lcf::Data::enemies);   //.enemy[a].name
	case 4:	 return get_name("Troop", lcf::Data::troops);   //.troop[a].name
	case 5:	 return get_name("Terrain", lcf::Data::terrains);   //.terrain[a].name
	case 6:	 return get_name("Attribute", lcf::Data::attributes);   //.element[a].name
	case 7:	 return get_name("State", lcf::Data::states);   //.state[a].name
	case 8:	 return get_name("Animation", lcf::Data::animations);   //.anim[a].name
	case 9:	 return get_name("Chipset", lcf::Data::chipsets);   //.tileset[a].name
	case 10: return Main_Data::game_switches->GetName(id);   //.s[a].name
	case 11: return Main_Data::game_variables->GetName(id);   //.v[a].name
	case 12: return {};  // FIXME: .t[a].name -- not sure how to get this for now
	case 13: //.cev[a].name
	{
		// assuming the vector of common events here is ordered by common event ID
		if (static_cast<int>(Game_Map::GetCommonEvents().size()) >= id) {
			return Game_Map::GetCommonEvents()[id - 1].GetName();
		}
		break;
	}
	case 14: return get_name("Class", lcf::Data::classes);   //.class[a].name
	case 15: return get_name("BattlerAnimation", lcf::Data::battleranimations);   //.anim2[a].name
	case 16: return Game_Map::GetMapName(id);   //.map[a].name
	case 17:   //.mev[a].name
	{
		auto map = Game_Map::GetEvent(id);
		if (map != nullptr) {
			return map->GetName();
		}
		break;
	}
	case 18: //.member[a].name, index starts from 0
	{
		auto actor = Main_Data::game_party->GetActor(id);
		if (actor != nullptr) {
			if (is_dynamic) {
				return actor->GetName();
			}
			else {
				id = actor->GetId();
				return get_name("Actor", lcf::Data::actors);
			}
		}
		break;
	}
	}

	Output::Warning("GetLcfName: Unsupported data_type {} {}", data_type, id);
	return {};
}

std::string_view ManiacPatch::GetLcfDescription(int data_type, int id, bool is_dynamic) {
	auto get_desc = [id](std::string_view type, const auto& vec) -> std::string_view {
		auto* data = lcf::ReaderUtil::GetElement(vec, id);
		if (!data) {
			Output::Warning("Unable to read {} description: {}", type, id);
			return {};
		}
		if constexpr (std::is_same_v<typename std::decay_t<decltype(vec)>::value_type, lcf::rpg::Actor>) {
			return data->title;
		} else {
			return data->description;
		}
	};

	switch (data_type)
	{
	case 0:  //.actor[a].desc
		if (is_dynamic) {
			auto actor = Main_Data::game_actors->GetActor(id);
			if (actor != nullptr) {
				return actor->GetTitle();
			}
		}
		else {
			return get_desc("Actor", lcf::Data::actors);
		}
		break;
	case 1: return get_desc("Skill", lcf::Data::skills); //.skill[a].desc
	case 2: return get_desc("Item", lcf::Data::items); //.item[a].desc
	case 18: //.member[a].desc
	{
		auto actor = Main_Data::game_party->GetActor(id);
		if (actor != nullptr) {
			if (is_dynamic) {
				return actor->GetTitle();
			}
			else {
				id = actor->GetId();
				return get_desc("Actor", lcf::Data::actors);
			}
		}
		break;
	}
	}

	Output::Warning("GetLcfDescription: Unsupported data_type {} {}", data_type, id);
	return {};
}

bool ManiacPatch::GlobalSave::Load() {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	if (global_save_opened) {
		return true;
	}

	// Even consider it opened when the file is missing
	// It will be created on Save
	global_save_opened = true;

	auto lgs_in = FileFinder::Save().OpenFile("Save.lgs");
	if (!lgs_in) {
		return false;
	}

	return Load(lgs_in);
}

bool ManiacPatch::GlobalSave::Load(Filesystem_Stream::InputStream& lgs_in) {
	if (!lgs_in) {
		return false;
	}

	lcf::LcfReader reader(lgs_in);
	std::string header;
	reader.ReadString(header, reader.ReadInt());
	if (header.length() != 13 || header != "LcfGlobalSave") {
		Output::Debug("This is not a valid global save.");
		return false;
	}

	lcf::LcfReader::Chunk chunk;

	while (!reader.Eof()) {
		chunk.ID = reader.ReadInt();
		chunk.length = reader.ReadInt();
		switch (chunk.ID) {
			case 1: {
				Game_Switches::Switches_t switches;
				reader.Read(switches, chunk.length);
				Main_Data::game_switches_global->SetData(std::move(switches));
				break;
			}
			case 2: {
				Game_Variables::Variables_t variables;
				reader.Read(variables, chunk.length);
				Main_Data::game_variables_global->SetData(std::move(variables));
				break;
			}
			default:
				reader.Skip(chunk, "CommandManiacControlGlobalSave");
		}
	}

	return true;
}

bool ManiacPatch::GlobalSave::Save(bool close_global_save) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	if (!global_save_opened) {
		return true;
	}

	auto savelgs_name = FileFinder::Save().FindFile("Save.lgs");
	if (savelgs_name.empty()) {
		savelgs_name = "Save.lgs";
	}

	auto lgs_out = FileFinder::Save().OpenOutputStream(savelgs_name);
	if (!Save(lgs_out)) {
		Output::Warning("Maniac ControlGlobalSave: Saving failed");
		return false;
	}

	global_save_opened = !close_global_save;

	AsyncHandler::SaveFilesystem();
	return true;
}

bool ManiacPatch::GlobalSave::Save(Filesystem_Stream::OutputStream& lgs_out) {
	if (!lgs_out) {
		return false;
	}

	lcf::LcfWriter writer(lgs_out, lcf::EngineVersion::e2k3);
	writer.WriteInt(13);
	const std::string header = "LcfGlobalSave";
	writer.Write(header);
	writer.WriteInt(1);
	writer.WriteInt(Main_Data::game_switches_global->GetSize());
	writer.Write(Main_Data::game_switches_global->GetData());
	writer.WriteInt(2);
	writer.WriteInt(Main_Data::game_variables_global->GetSize() * sizeof(int32_t));
	writer.Write(Main_Data::game_variables_global->GetData());
	return true;
}

void ManiacPatch::GlobalSave::Close() {
	global_save_opened = false;
}
