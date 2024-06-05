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

#include "game_interpreter_debug.h"
#include "game_interpreter.h"
#include "game_battle.h"
#include "game_map.h"
#include "main_data.h"
#include "game_variables.h"
#include "output.h"
#include <lcf/reader_util.h>

Debug::ParallelInterpreterStates Debug::ParallelInterpreterStates::GetCachedStates() {
	Game_Interpreter_Inspector inspector;

	std::vector<int> ev_ids;
	std::vector<int> ce_ids;

	std::vector<lcf::rpg::SaveEventExecState> state_ev;
	std::vector<lcf::rpg::SaveEventExecState> state_ce;

	if (Game_Map::GetMapId() > 0) {
		for (auto& ev : Game_Map::GetEvents()) {
			if (!inspector.IsInActiveExcecution(ev, true)) {
				continue;
			}

			ev_ids.emplace_back(ev.GetId());
			state_ev.emplace_back(inspector.GetExecState(ev));
		}
		for (auto& ce : Game_Map::GetCommonEvents()) {
			if (!inspector.IsInActiveExcecution(ce, true)) {
				continue;
			}
			ce_ids.emplace_back(ce.GetId());
			state_ce.emplace_back(inspector.GetExecState(ce));
		}
	} else if (Game_Battle::IsBattleRunning() && Player::IsPatchManiac()) {
		//FIXME: Not implemented: battle common events
	}

	return { ev_ids, ce_ids, state_ev, state_ce };
}

std::vector<Debug::CallStackItem> Debug::CreateCallStack(const lcf::rpg::SaveEventExecState& state) {
	std::vector<CallStackItem> items;
	items.reserve(state.stack.size());

	for (int i = state.stack.size() - 1; i >= 0; i--) {
		auto& frame = state.stack[i];

		bool map_has_changed = (frame.event_id == 0 && frame.maniac_event_id > 0);

		Debug::CallStackItem item = {
			Game_Interpreter_Shared::EasyRpgExecutionType(frame),
			Game_Interpreter_Shared::EasyRpgEventType(frame),
			frame.maniac_event_id,
			frame.maniac_event_page_id,
			GetEventName(frame),
			i + 1,					//stack_item_no
			frame.current_command,	// cmd_current
			frame.commands.size(),	// cmd_count
			map_has_changed
		};

		items.push_back(item);
	}

	return items;
}

std::string Debug::GetEventName(const lcf::rpg::SaveEventExecFrame& frame) {
	switch (Game_Interpreter_Shared::EasyRpgEventType(frame)) {
		case InterpreterEventType::MapEvent:
			if (auto* ev = Game_Map::GetEvent(frame.event_id)) {
				return ToString(ev->GetName());
			} else if (frame.maniac_event_id > 0) {
				return fmt::format("[(EV{:04d}) from another map..]", frame.maniac_event_id);
			}
			break;
		case InterpreterEventType::CommonEvent:
			if (auto* ce = lcf::ReaderUtil::GetElement(lcf::Data::commonevents, frame.maniac_event_id)) {
				return ToString(ce->name);
			}
			break;
		default:
			break;
	}
	return "";
}

std::string Debug::FormatEventName(Game_Character const& ch) {
	switch (ch.GetType()) {
		case Game_Character::Player:
			return "Player";
		case Game_Character::Vehicle:
		{
			int type = static_cast<Game_Vehicle const&>(ch).GetVehicleType();
			assert(type > Game_Vehicle::None && type <= Game_Vehicle::Airship);
			return Game_Vehicle::TypeNames[type];
		}
		case Game_Character::Event:
		{
			auto& ev = static_cast<Game_Event const&>(ch);
			if (ev.GetName().empty()) {
				return fmt::format("EV{:04d}", ev.GetId());
			}
			return fmt::format("EV{:04d} '{}'", ev.GetId(), ev.GetName());
		}
		default:
			assert(false);
	}
	return "";
}

std::string Debug::FormatEventName(Game_CommonEvent const& ce) {
	if (ce.GetName().empty()) {
		return fmt::format("CE{:04d}", ce.GetIndex());
	}
	return fmt::format("CE{:04d}: '{}'", ce.GetIndex(), ce.GetName());
}

void Debug::AssertBlockedMoves() {
	auto check = [](Game_Character& ev) {
		return ev.IsMoveRouteOverwritten() && !ev.IsMoveRouteFinished()
			&& ev.GetStopCount() != 0xFFFF && ev.GetStopCount() > ev.GetMaxStopCount();
	};
	auto assert_way = [](Game_Character& ev) {
		using Code = lcf::rpg::MoveCommand::Code;
		auto& move_command = ev.GetMoveRoute().move_commands[ev.GetMoveRouteIndex()];

		if (move_command.command_id >= static_cast<int>(Code::move_up)
			&& move_command.command_id <= static_cast<int>(Code::move_forward)) {

			const int dir = ev.GetDirection();
			const int from_x = ev.GetX(),
				from_y = ev.GetY(),
				to_x = from_x + ev.GetDxFromDirection(dir),
				to_y = from_y + ev.GetDyFromDirection(dir);

			if (from_x != to_x && from_y != to_y) {
				bool valid = Game_Map::AssertWay(ev, from_x, from_y, from_x, to_y);
				if (valid)
					valid = Game_Map::AssertWay(ev, from_x, to_y, to_x, to_y);
				if (valid)
					valid = Game_Map::AssertWay(ev, from_x, from_y, to_x, from_y);
				if (valid)
					valid = Game_Map::AssertWay(ev, to_x, from_y, to_x, to_y);
			} else {
				Game_Map::AssertWay(ev, from_x, from_y, to_x, to_y);
			}
		}
	};
	const auto map_id = Game_Map::GetMapId();
	if (auto& ch = *Main_Data::game_player; check(ch)) {
		assert_way(ch);
	}
	if (auto& vh = *Game_Map::GetVehicle(Game_Vehicle::Boat); vh.GetMapId() == map_id && check(vh)) {
		assert_way(vh);
	}
	if (auto& vh = *Game_Map::GetVehicle(Game_Vehicle::Ship); vh.GetMapId() == map_id && check(vh)) {
		assert_way(vh);
	}
	if (auto& vh = *Game_Map::GetVehicle(Game_Vehicle::Airship); vh.GetMapId() == map_id && check(vh)) {
		assert_way(vh);
	}
	for (auto& ev : Game_Map::GetEvents()) {
		if (check(ev)) {
			assert_way(ev);
		}
	}
}
