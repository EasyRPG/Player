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
#include "game_commonevent.h"
#include "game_map.h"
#include "game_switches.h"
#include "game_interpreter_map.h"
#include "main_data.h"
#include "reader_util.h"

Game_CommonEvent::Game_CommonEvent(int common_event_id) :
	common_event_id(common_event_id) {

	if (GetTrigger() == RPG::EventPage::Trigger_parallel) {
		interpreter.reset(new Game_Interpreter_Map());
	}
}

void Game_CommonEvent::SetSaveData(const RPG::SaveEventExecState& data) {
	if (interpreter && !data.stack.empty()) {
		interpreter->SetState(data);
	}
}

void Game_CommonEvent::Update() {
	if (interpreter && IsWaitingBackgroundExecution()) {
		if (!interpreter->IsRunning()) {
			interpreter->Clear();
			interpreter->Push(this);
		}
		interpreter->Update();
	}
}

int Game_CommonEvent::GetIndex() const {
	return common_event_id;
}

// Game_Map ensures validity of Common Events

std::string Game_CommonEvent::GetName() const {
	return ReaderUtil::GetElement(Data::commonevents, common_event_id)->name;
}

bool Game_CommonEvent::GetSwitchFlag() const {
	return ReaderUtil::GetElement(Data::commonevents, common_event_id)->switch_flag;
}

int Game_CommonEvent::GetSwitchId() const {
	return ReaderUtil::GetElement(Data::commonevents, common_event_id)->switch_id;
}

int Game_CommonEvent::GetTrigger() const {
	return ReaderUtil::GetElement(Data::commonevents, common_event_id)->trigger;
}

std::vector<RPG::EventCommand>& Game_CommonEvent::GetList() {
	return ReaderUtil::GetElement(Data::commonevents, common_event_id)->event_commands;
}

RPG::SaveEventExecState Game_CommonEvent::GetSaveData() {
	RPG::SaveEventExecState state;
	if (interpreter) {
		state = interpreter->GetState();
	}
	return state;
}

bool Game_CommonEvent::IsWaitingExecution(RPG::EventPage::Trigger trigger) const {
	auto* ce = ReaderUtil::GetElement(Data::commonevents, common_event_id);
	return ce->trigger == trigger &&
		(!ce->switch_flag || Game_Switches.Get(ce->switch_id))
		&& !ce->event_commands.empty();
}

bool Game_CommonEvent::IsWaitingForegroundExecution() const {
	return IsWaitingExecution(RPG::EventPage::Trigger_auto_start);
}

bool Game_CommonEvent::IsWaitingBackgroundExecution() const {
	return IsWaitingExecution(RPG::EventPage::Trigger_parallel);
}
