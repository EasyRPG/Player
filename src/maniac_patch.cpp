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

#include "bitmap.h"
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
#include "pixel_format.h"
#include "player.h"

#include <lcf/reader_lcf.h>
#include <lcf/reader_util.h>
#include <lcf/writer_lcf.h>
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
		Between,
		Lerp,
		ArraySum,
		ArrayMin,
		ArrayMax
	};

	bool global_save_opened = false;
}

struct ProcessAssignmentRet {
	Op op = Op::Null;
	std::vector<int> ids;

	std::vector<int> fetch() const {
		std::vector<int> res;
		res.reserve(ids.size());
		for (int id : ids) {
			switch (op) {
			case Op::Var:
				res.push_back(Main_Data::game_variables->Get(id));
				break;
			case Op::Switch:
				res.push_back(Main_Data::game_switches->GetInt(id));
				break;
			case Op::VarIndirect:
				res.push_back(Main_Data::game_variables->GetIndirect(id));
				break;
			case Op::SwitchIndirect:
				res.push_back(Main_Data::game_switches->GetInt(Main_Data::game_variables->Get(id)));
				break;
			default:
				Output::Warning("Maniac: Expression assignment {} is not a lvalue", static_cast<int>(op));
				res.push_back(0);
				break;
			}
		}
		return res;
	}

	std::vector<int> assign(const std::vector<int>& rhs) const {
		std::vector<int> res;
		res.reserve(ids.size());
		for (size_t i = 0; i < ids.size(); ++i) {
			int id = ids[i];
			int val = (i < rhs.size()) ? rhs[i] : 0;
			switch (op) {
			case Op::Var:
				Game_Map::SetNeedRefreshForVarChange(id);
				Main_Data::game_variables->Set(id, val);
				res.push_back(val);
				break;
			case Op::Switch:
				Game_Map::SetNeedRefreshForSwitchChange(id);
				Main_Data::game_switches->Set(id, val > 0);
				res.push_back(val);
				break;
			case Op::VarIndirect: {
				int var = Main_Data::game_variables->GetIndirect(id);
				Game_Map::SetNeedRefreshForVarChange(var);
				Main_Data::game_variables->Set(var, val);
				res.push_back(val);
				break;
			}
			case Op::SwitchIndirect: {
				int var = Main_Data::game_variables->GetIndirect(id);
				Game_Map::SetNeedRefreshForSwitchChange(var);
				Main_Data::game_switches->Set(var, val > 0);
				res.push_back(val);
				break;
			}
			default:
				res.push_back(0);
				break;
			}
		}
		return res;
	}

	template <typename F>
	std::vector<int> assign_op(const std::vector<int>& rhs, F&& fn) const {
		auto current = fetch();
		std::vector<int> new_vals;
		new_vals.reserve(current.size());
		for (size_t i = 0; i < current.size(); ++i) {
			int val = (i < rhs.size()) ? rhs[i] : (rhs.empty() ? 0 : rhs.back());
			new_vals.push_back(fn(current[i], val));
		}
		return assign(new_vals);
	}
};

std::vector<int> Process(std::vector<int32_t>::iterator& it, std::vector<int32_t>::iterator end, const Game_BaseInterpreterContext& ip, bool execute);

ProcessAssignmentRet ProcessAssignment(std::vector<int32_t>::iterator& it, std::vector<int32_t>::iterator end, const Game_BaseInterpreterContext& ip, bool execute) {
	if (it == end) {
		return { Op::Null, {0} };
	}

	auto op = static_cast<Op>(*it);
	++it;

	switch (op) {
	case Op::Var:
	case Op::Switch:
	case Op::VarIndirect:
	case Op::SwitchIndirect:
		return { op, Process(it, end, ip, execute) };
	default:
		--it; // back on the op as op is fetched again by Process
		return { Op::Null, Process(it, end, ip, execute) };
	}
}

std::vector<int> Process(std::vector<int32_t>::iterator& it, std::vector<int32_t>::iterator end, const Game_BaseInterpreterContext& ip, bool execute) {
	if (it == end) {
		return { 0 };
	}

	auto op = static_cast<Op>(*it);
	++it;

	auto eval1 = [&]() {
		auto r = Process(it, end, ip, execute);
		return r.empty() ? 0 : r[0];
	};

	auto eval2 = [&]() {
		int a = eval1();
		int b = eval1();
		return std::make_tuple(a, b);
	};

	auto eval3 = [&]() {
		int a = eval1();
		int b = eval1();
		int c = eval1();
		return std::make_tuple(a, b, c);
	};

	// When entering the switch it is on the first argument
	switch (op) {
	case Op::Null:
		return { 0 };
	case Op::U8:
	case Op::UX8:
		return { *it++ };
	case Op::U16:
	case Op::UX16: {
		uint32_t u0 = *it++;
		if (it == end) return { 0 };
		uint32_t u1 = *it++;
		return { static_cast<int32_t>((u1 << 8) | u0) };
	}
	case Op::S32:
	case Op::SX32: {
		uint32_t u0 = *it++;
		if (it == end) return { 0 };
		uint32_t u1 = *it++;
		if (it == end) return { 0 };
		uint32_t u2 = *it++;
		if (it == end) return { 0 };
		uint32_t u3 = *it++;
		uint32_t val = (u3 << 24) | (u2 << 16) | (u1 << 8) | u0;
		return { static_cast<int32_t>(val) };
	}
	case Op::Var: {
		auto ids = Process(it, end, ip, execute);
		std::vector<int> res;
		res.reserve(ids.size());
		for (int id : ids) {
			res.push_back(Main_Data::game_variables->Get(id));
		}
		return res;
	}
	case Op::Switch: {
		auto ids = Process(it, end, ip, execute);
		std::vector<int> res;
		res.reserve(ids.size());
		for (int id : ids) {
			res.push_back(Main_Data::game_switches->GetInt(id));
		}
		return res;
	}
	case Op::VarIndirect: {
		auto ids = Process(it, end, ip, execute);
		std::vector<int> res;
		res.reserve(ids.size());
		for (int id : ids) {
			res.push_back(Main_Data::game_variables->GetIndirect(id));
		}
		return res;
	}
	case Op::SwitchIndirect: {
		auto ids = Process(it, end, ip, execute);
		std::vector<int> res;
		res.reserve(ids.size());
		for (int id : ids) {
			res.push_back(Main_Data::game_switches->GetInt(Main_Data::game_variables->Get(id)));
		}
		return res;
	}
	case Op::Array: {
		int32_t n = *it++;
		if ((n & 0x80) != 0) {
			int32_t b0 = *it++;
			int32_t b1 = *it++;
			int32_t b2 = *it++;
			int32_t b3 = *it++;
			n = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
		}
		std::vector<int> res;
		for (int32_t i = 0; i < n; ++i) {
			auto elem = Process(it, end, ip, execute);
			res.insert(res.end(), elem.begin(), elem.end());
		}
		return res;
	}
	case Op::Range: {
		auto [first, second] = eval2();
		std::vector<int> res;
		if (first <= second) {
			for (int i = first; i <= second; ++i) {
				res.push_back(i);
			}
		} else {
			for (int i = first; i >= second; --i) {
				res.push_back(i);
			}
		}
		return res;
	}
	case Op::Subscript: {
		auto arr = Process(it, end, ip, execute);
		int idx = eval1();
		if (idx >= 0 && idx < static_cast<int>(arr.size())) {
			return { arr[idx] };
		} else {
			return { 0 };
		}
	}
	case Op::Negate:
		return { -eval1() };
	case Op::Not:
		return { !eval1() ? 1 : 0 };
	case Op::Flip:
		return { ~eval1() };
	case Op::AssignInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign(rhs);
		}
		return rhs;
	}
	case Op::AddInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) {
				return static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(a) + b, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
			});
		}
		return { 0 };
	}
	case Op::SubInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) {
				return static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(a) - b, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
			});
		}
		return { 0 };
	}
	case Op::MulInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) {
				return static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(a) * b, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
			});
		}
		return { 0 };
	}
	case Op::DivInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return b == 0 ? a : a / b; });
		}
		return { 0 };
	}
	case Op::ModInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return b == 0 ? 0 : a % b; });
		}
		return { 0 };
	}
	case Op::BitOrInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return a | b; });
		}
		return { 0 };
	}
	case Op::BitAndInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return a & b; });
		}
		return { 0 };
	}
	case Op::BitXorInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return a ^ b; });
		}
		return { 0 };
	}
	case Op::BitShiftLeftInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return a << b; });
		}
		return { 0 };
	}
	case Op::BitShiftRightInplace: {
		auto ret = ProcessAssignment(it, end, ip, execute);
		auto rhs = Process(it, end, ip, execute);
		if (execute) {
			return ret.assign_op(rhs, [](int a, int b) { return static_cast<int32_t>(static_cast<uint32_t>(a) >> b); });
		}
		return { 0 };
	}
	case Op::Add: {
		auto [first, second] = eval2();
		return {
			static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(first) + second, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()))
		};
	}
	case Op::Sub: {
		auto [first, second] = eval2();
		return {
			static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(first) - second, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()))
		};
	}
	case Op::Mul: {
		auto [first, second] = eval2();
		return {
			static_cast<int32_t>(Utils::Clamp<int64_t>(static_cast<int64_t>(first) * second, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()))
		};
	}
	case Op::Div: {
		auto [first, second] = eval2();
		return {
			second == 0 ? first : first / second
		};
	}
	case Op::Mod: {
		auto [first, second] = eval2();
		return {
			second == 0 ? 0 : first % second
		};
	}
	case Op::BitOr: {
		auto [first, second] = eval2();
		return {
			first | second
		};
	}
	case Op::BitAnd: {
		auto [first, second] = eval2();
		return {
			first & second
		};
	}
	case Op::BitXor: {
		auto [first, second] = eval2();
		return {
			first ^ second
		};
	}
	case Op::BitShiftLeft: {
		auto [first, second] = eval2();
		return {
			first << second
		};
	}
	case Op::BitShiftRight: {
		auto [first, second] = eval2();
		return {
			static_cast<int32_t>(static_cast<uint32_t>(first) >> second)
		};
	}
	case Op::Equal: {
		auto [first, second] = eval2();
		return {
			first == second ? 1 : 0
		};
	}
	case Op::GreaterEqual: {
		auto [first, second] = eval2();
		return {
			first >= second ? 1 : 0
		};
	}
	case Op::LessEqual: {
		auto [first, second] = eval2();
		return {
			first <= second ? 1 : 0
		};
	}
	case Op::Greater: {
		auto [first, second] = eval2();
		return {
			first > second ? 1 : 0
		};
	}
	case Op::Less: {
		auto [first, second] = eval2();
		return {
			first < second ? 1 : 0
		};
	}
	case Op::NotEqual: {
		auto [first, second] = eval2();
		return {
			first != second ? 1 : 0
		};
	}
	case Op::Or: {
		auto res_first = Process(it, end, ip, execute);
		int first = res_first.empty() ? 0 : res_first[0];
		bool next_execute = execute && (first == 0);
		auto res_second = Process(it, end, ip, next_execute);
		int second = res_second.empty() ? 0 : res_second[0];
		return { (first != 0 || second != 0) ? 1 : 0 };
	}
	case Op::And: {
		auto res_first = Process(it, end, ip, execute);
		int first = res_first.empty() ? 0 : res_first[0];
		bool next_execute = execute && (first != 0);
		auto res_second = Process(it, end, ip, next_execute);
		int second = res_second.empty() ? 0 : res_second[0];
		return { (first != 0 && second != 0) ? 1 : 0 };
	}
	case Op::Ternary: {
		auto res_cond = Process(it, end, ip, execute);
		int cond = res_cond.empty() ? 0 : res_cond[0];
		auto res_true = Process(it, end, ip, execute && (cond != 0));
		auto res_false = Process(it, end, ip, execute && (cond == 0));
		return cond != 0 ? res_true : res_false;
	}
	case Op::Function: {
		int fn = *it++;
		int argc = *it++;
		if ((argc & 0x80) != 0) {
			Output::Warning("Maniac: Expression func long args unsupported");
			return { 0 };
		}
		switch (static_cast<Fn>(fn)) {
		case Fn::Rand: {
			if (argc != 2) return { 0 };
			auto [first, second] = eval2();
			if (execute) {
				return {
					ControlVariables::Random(first, second)
				};
			}
			return { 0 };
		}
		case Fn::Item: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Item(first, second)
			};
		}
		case Fn::Event: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Event(first, second, ip)
			};
		}
		case Fn::Actor: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Actor(first, second)
			};
		}
		case Fn::Party: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Party(first, second)
			};
		}
		case Fn::Enemy: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Enemy(first, second)
			};
		}
		case Fn::Misc: {
			if (argc != 1) { return { 0 }; }
			return {
				ControlVariables::Other(eval1())
			};
		}
		case Fn::Pow: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Pow(first, second)
			};
		}
		case Fn::Sqrt: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Sqrt(first, second)
			};
		}
		case Fn::Sin: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Sin(first, second, third)
			};
		}
		case Fn::Cos: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Cos(first, second, third)
			};
		}
		case Fn::Atan2: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Atan2(first, second, third)
			};
		}
		case Fn::Min: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Min(first, second)
			};
		}
		case Fn::Max: {
			if (argc != 2) { return { 0 }; }
			auto [first, second] = eval2();
			return {
				ControlVariables::Max(first, second)
			};
		}
		case Fn::Abs: {
			if (argc != 1) { return { 0 }; }
			return {
				ControlVariables::Abs(eval1())
			};
		}
		case Fn::Clamp: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Clamp(first, second, third)
			};
		}
		case Fn::Muldiv: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Muldiv(first, second, third)
			};
		}
		case Fn::Divmul: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Divmul(first, second, third)
			};
		}
		case Fn::Between: {
			if (argc != 3) { return { 0 }; }
			auto [first, second, third] = eval3();
			return {
				ControlVariables::Between(first, second, third)
			};
		}
		case Fn::Lerp: {
			if (argc != 4) return { 0 };
			int a = eval1();
			int b = eval1();
			int num = eval1();
			int den = eval1();
			if (execute) {
				return {
					ControlVariables::Lerp(a, b, num, den)
				};
			}
			return { 0 };
		}
		case Fn::ArraySum: {
			if (argc != 2) return { 0 };
			auto [first, second] = eval2();
			if (execute) {
				return {
					ControlVariables::ArraySum(first, second)
				};
			}
			return { 0 };
		}
		case Fn::ArrayMin: {
			if (argc != 2) return { 0 };
			auto [first, second] = eval2();
			if (execute) {
				return {
					ControlVariables::ArrayMin(first, second)
				};
			}
			return { 0 };
		}
		case Fn::ArrayMax: {
			if (argc != 2) return { 0 };
			auto [first, second] = eval2();
			if (execute) {
				return {
					ControlVariables::ArrayMax(first, second)
				};
			}
			return { 0 };
		}
		default:
			Output::Warning("Maniac: Expression Unknown Func {}", fn);
			for (int i = 0; i < argc; ++i) {
				eval1();
			}
			return { 0 };
		}
	}
	default:
		Output::Warning("Maniac: Expression contains unsupported operation {}", static_cast<int>(op));
		return { 0 };
	}
}

int32_t ManiacPatch::ParseExpression(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter) {
	std::vector<int32_t> ops;
	for (auto& o : op_codes) {
		auto uo = static_cast<uint32_t>(o);
		ops.push_back(static_cast<int32_t>(uo & 0x000000FF));
		ops.push_back(static_cast<int32_t>((uo & 0x0000FF00) >> 8));
		ops.push_back(static_cast<int32_t>((uo & 0x00FF0000) >> 16));
		ops.push_back(static_cast<int32_t>((uo & 0xFF000000) >> 24));
	}
	auto beg = ops.begin();
	auto res = Process(beg, ops.end(), interpreter, true);
	return res.empty() ? 0 : res[0];
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
		auto res = Process(it, ops.end(), interpreter, true);
		if (!res.empty()) {
			results.push_back(res[0]);
		}

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

bool ManiacPatch::WritePixelsFromVariableToBitmap(Bitmap& dst, Rect dst_rect, int start_var_id, bool clear_dst, bool ignore_alpha, Game_Variables& variables) {
	int pic_x = dst_rect.x;
	int pic_y = dst_rect.y;
	int pic_w = dst_rect.width;
	int pic_h = dst_rect.height;

	if (pic_w <= 0 || pic_h <= 0) {
		return false;
	}

	// Format expected by Maniacs
	auto format = format_B8G8R8A8_a().format();
	if (ignore_alpha) {
		format = format_B8G8R8A8_n().format();
	}

	// Allocate an image as large as the requested dimensions (ignoring out of bounds)
	Rect bmp_rect = dst.GetRect();
	Rect frame_rect = bmp_rect.GetSubRect(dst_rect);

	if (frame_rect.width <= 0 || frame_rect.height <= 0) {
		return false;
	}

	BitmapRef frame = Bitmap::Create(nullptr, frame_rect.width, frame_rect.height, frame_rect.width * format.bytes, format);

	uint32_t* pixels = static_cast<uint32_t*>(frame->pixels());
	int px_per_row = frame->pitch() / sizeof(uint32_t);
	uint32_t* dst_row = pixels;

	// Rowwise memcpy
	int x_l = std::min(0, pic_x);
	int x_r = std::max(0, pic_x + pic_w - bmp_rect.width) + frame_rect.width;
	int y_t = std::min(0, pic_y);
	int y_b = std::max(0, pic_y + pic_h - bmp_rect.height) + frame_rect.height;

	int src_var_id = start_var_id;
	dst_row = pixels;
	for (int y = y_t; y < y_b; ++y) {
		// When row out of bounds skip
		if (y < 0 || y >= frame_rect.height) {
			src_var_id += pic_w;
			continue;
		}

		for (int x = x_l; x < x_r;) {
			if (x < 0) {
				// OOB to the left (skip)
				src_var_id += -x;
				x = 0;
			} else if (x >= frame_rect.width) {
				// OOB to the right (skip)
				int len = x_r - frame_rect.width;
				src_var_id += len;
				break;
			} else {
				auto in_range = variables.GetRange(src_var_id, frame_rect.width);
				std::copy(in_range.begin(), in_range.end(), dst_row);
				dst_row += px_per_row;
				src_var_id += frame_rect.width;
				x += frame_rect.width;
			}
		}
	}

	if (clear_dst) {
		dst.ClearRect({pic_x, pic_y, frame_rect.width, frame_rect.height});
	}

	dst.Blit(pic_x, pic_y, *frame, frame->GetRect(), Opacity::Opaque(),
		ignore_alpha ? Bitmap::BlendMode::NormalWithoutAlpha : Bitmap::BlendMode::Normal);

	return true;
}

bool ManiacPatch::WritePixelsFromBitmapToVariable(const Bitmap& src, Rect src_rect, int start_var_id, bool ignore_alpha, Game_Variables& variables) {
	// FIXME: Because we use premultiplied alpha the colors of transparent pixels are lost (always 0)
	// Maniacs appears to preserve them
	// E.g. when reading a transparent pixel from Chara1 (which was green) then Maniac will read green and we read 0
	// This is noticable e.g. when using the EditPicture command with the opaque flag when reading from a transparent image

	int pic_x = src_rect.x;
	int pic_y = src_rect.y;
	int pic_w = src_rect.width;
	int pic_h = src_rect.height;

	if (pic_w <= 0 || pic_h <= 0) {
		return false;
	}

	// Format expected by Maniacs
	auto format = format_B8G8R8A8_a().format();
	if (ignore_alpha) {
		format = format_B8G8R8A8_n().format();
	}

	// Allocate an image as large as the requested dimensions (ignoring out of bounds)
	Rect bmp_rect = src.GetRect();
	Rect frame_rect = bmp_rect.GetSubRect(src_rect);

	if (frame_rect.width <= 0 || frame_rect.height <= 0) {
		return false;
	}

	BitmapRef frame = Bitmap::Create(nullptr, frame_rect.width, frame_rect.height, frame_rect.width * format.bytes, format);

	// Then blit the screen (converts to the correct format)
	frame->Blit(0, 0, src, frame_rect, Opacity::Opaque(),
		ignore_alpha ? Bitmap::BlendMode::NormalWithoutAlpha : Bitmap::BlendMode::Default);

	uint32_t* pixels = static_cast<uint32_t*>(frame->pixels());
	int px_per_row = frame->pitch() / sizeof(uint32_t);
	uint32_t* src_row = pixels;

	if (ignore_alpha) {
		// Slow: Set all alpha values to 0
		const auto a_mask = format.a.mask;
		for (int y = 0; y < frame_rect.height; ++y) {
			for (int x = 0; x < frame_rect.width; ++x) {
				src_row[x] &= ~a_mask;
			}
			src_row += px_per_row;
		}
	}

	// Rowwise memcpy
	int x_l = std::min(0, pic_x);
	int x_r = std::max(0, pic_x + pic_w - bmp_rect.width) + frame_rect.width;
	int y_t = std::min(0, pic_y);
	int y_b = std::max(0, pic_y + pic_h - bmp_rect.height) + frame_rect.height;

	int dst_var_id = start_var_id;
	src_row = pixels;
	for (int y = y_t; y < y_b; ++y) {
		// When row out of bounds write 0 in this row
		if (y < 0 || y >= frame_rect.height) {
			auto out_range = variables.GetWritableRange(dst_var_id, pic_w);
			std::fill(out_range.begin(), out_range.end(), 0);
			dst_var_id += pic_w;
			continue;
		}

		for (int x = x_l; x < x_r;) {
			if (x < 0) {
				// OOB to the left (write 0 for remaining cols)
				auto out_range = variables.GetWritableRange(dst_var_id, -x_l);
				std::fill(out_range.begin(), out_range.end(), 0);
				dst_var_id += -x;
				x = 0;
			} else if (x >= frame_rect.width) {
				// OOB to the right (write 0 for remaining cols)
				int len = x_r - frame_rect.width;
				auto out_range = variables.GetWritableRange(dst_var_id, len);
				std::fill(out_range.begin(), out_range.end(), 0);
				dst_var_id += len;
				break;
			} else {
				auto out_range = variables.GetWritableRange(dst_var_id, frame_rect.width);
				std::copy(src_row, src_row + frame_rect.width, out_range.data());
				src_row += px_per_row;
				dst_var_id += frame_rect.width;
				x += frame_rect.width;
			}
		}
	}

	return true;
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

bool ManiacPatch::DecodeStringToInt(std::string_view str, uint32_t& out) {
	/*
	Is a custom 5 bit encoding:

	For the rightmost character c the value is:
	c - A + 1

	For all other characters c with index i the value is:
	(c - a + 1) << (i * 5)

	Due to integer overflow the max string length is 6.
	*/

	if (str.empty() || str.size() > 6) {
		out = 0;
		return false;
	}

	auto in_range = [](uint32_t value) {
		return value < 32;
	};

	out = (str.back() - 'A' + 1);

	if (!in_range(out)) {
		return false;
	}

	str.remove_suffix(1);

	for (size_t i = 0; i < str.size(); ++i) {
		uint32_t result = (str[i] - 'a' + 1);

		if (!in_range(result)) {
			return false;
		}

		int chidx = str.size() - i;
		out += (result << (chidx * 5));
	}

	return true;
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
