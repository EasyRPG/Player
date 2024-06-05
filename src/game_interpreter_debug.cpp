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
	std::vector<int> ev_ids;
	std::vector<int> ce_ids;

	std::vector<lcf::rpg::SaveEventExecState> state_ev;
	std::vector<lcf::rpg::SaveEventExecState> state_ce;

	if (Game_Map::GetMapId() > 0) {
		for (auto& ev : Game_Map::GetEvents()) {
			if (ev.GetTrigger() != lcf::rpg::EventPage::Trigger_parallel || !ev.interpreter)
				continue;
			ev_ids.emplace_back(ev.GetId());
			state_ev.emplace_back(ev.interpreter->GetState());
		}
		for (auto& ce : Game_Map::GetCommonEvents()) {
			if (ce.IsWaitingBackgroundExecution(false)) {
				ce_ids.emplace_back(ce.common_event_id);
				state_ce.emplace_back(ce.interpreter->GetState());
			}
		}
	} else if (Game_Battle::IsBattleRunning() && Player::IsPatchManiac()) {
		//FIXME: Not implemented: battle common events
	}

	return { ev_ids, ce_ids, state_ev, state_ce };
}

std::vector<Debug::CallStackItem> Debug::CreateCallStack(const int owner_evt_id, const lcf::rpg::SaveEventExecState& state) {
	std::vector<CallStackItem> items;

	for (int i = state.stack.size() - 1; i >= 0; i--) {
		int evt_id = state.stack[i].event_id;
		int page_id = 0;
		if (state.stack[i].maniac_event_id > 0) {
			evt_id = state.stack[i].maniac_event_id;
			page_id = state.stack[i].maniac_event_page_id;
		}
		if (evt_id == 0 && i == 0)
			evt_id = owner_evt_id;

		bool is_calling_ev_ce = false;

		//FIXME: There are some currently unimplemented SaveEventExecFrame fields introduced via the ManiacPatch which should be used to properly get event state information
		if (evt_id == 0 && i > 0) {
			auto& prev_frame = state.stack[i - 1];
			auto& com = prev_frame.commands[prev_frame.current_command - 1];
			if (com.code == 12330) { // CallEvent
				if (com.parameters[0] == 0) {
					is_calling_ev_ce = true;
					evt_id = com.parameters[1];
				} else if (com.parameters[0] == 3 && Player::IsPatchManiac()) {
					is_calling_ev_ce = true;
					evt_id = Main_Data::game_variables->Get(com.parameters[1]);
				} else if (com.parameters[0] == 4 && Player::IsPatchManiac()) {
					is_calling_ev_ce = true;
					evt_id = Main_Data::game_variables->GetIndirect(com.parameters[1]);
				}
			}
		}

		auto item = Debug::CallStackItem();
		item.stack_item_no = i + 1;
		item.is_ce = is_calling_ev_ce;
		item.evt_id = evt_id;
		item.page_id = page_id;
		item.name = "";
		item.cmd_current = state.stack[i].current_command;
		item.cmd_count = state.stack[i].commands.size();

		if (item.is_ce) {
			auto* ce = lcf::ReaderUtil::GetElement(lcf::Data::commonevents, item.evt_id);
			if (ce) {
				item.name = ToString(ce->name);
			}
		} else {
			auto* ev = Game_Map::GetEvent(evt_id);
			if (ev) {
				//FIXME: map could have changed, but map_id isn't available
				item.name = ToString(ev->GetName());
			}
		}

		items.push_back(item);
	}

	return items;
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
