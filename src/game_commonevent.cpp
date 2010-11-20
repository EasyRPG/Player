/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "game_commonevent.h"
#include "game_switches.h"
#include "game_interpreter.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
Game_CommonEvent::Game_CommonEvent(int common_event_id) :
	common_event_id(common_event_id),
	interpreter(NULL) {
}

////////////////////////////////////////////////////////////
Game_CommonEvent::~Game_CommonEvent() {
	delete interpreter;
}

////////////////////////////////////////////////////////////
void Game_CommonEvent::Refresh() {
	if ( (GetTrigger() == 2) && ( Game_Switches[GetSwitchId()] ) ) {
		if (interpreter == NULL) {
			interpreter = new Game_Interpreter();
		}
	} else {
		delete interpreter;
		interpreter = NULL;
	}
}

////////////////////////////////////////////////////////////
void Game_CommonEvent::Update() {
	if (interpreter) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(GetList(), 0);
		}
		interpreter->Update();
	}
}

////////////////////////////////////////////////////////////
std::string Game_CommonEvent::GetName() const {
	return Main_Data::data_commonevents[common_event_id].name;
}

int Game_CommonEvent::GetSwitchId() const {
	return Main_Data::data_commonevents[common_event_id].switch_id;
}

int Game_CommonEvent::GetTrigger() const {
	return Main_Data::data_commonevents[common_event_id].trigger;
}

std::vector<RPG::EventCommand> Game_CommonEvent::GetList() const {
	return Main_Data::data_commonevents[common_event_id].event_commands;
}
