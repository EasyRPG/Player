#include "game_commonevent.h"
#include "game_switches.h"
#include "game_interpreter.h"
#include "main_data.h"

Game_CommonEvent::Game_CommonEvent(int _common_event_id) {
	common_event_id = _common_event_id;
	interpreter = NULL;
}

Game_CommonEvent::~Game_CommonEvent() {
	delete interpreter;
}

std::string Game_CommonEvent::Name() {
	return Main_Data::data_commonevents[common_event_id].name;
}

int Game_CommonEvent::SwitchId() {
	return Main_Data::data_commonevents[common_event_id].switch_id;
}

int Game_CommonEvent::Trigger() {
	return Main_Data::data_commonevents[common_event_id].trigger;
}

std::vector<RPG::EventCommand> Game_CommonEvent::List() {
	return Main_Data::data_commonevents[common_event_id].event_commands;
}

void Game_CommonEvent::Refresh() {
	if ( (Trigger() == 2) && ( (*Main_Data::game_switches)[SwitchId()] ) ) {
		if (interpreter == NULL) {
			interpreter = new Game_Interpreter();
		}
	} else {
		delete interpreter;
		interpreter = NULL;
	}
}

void Game_CommonEvent::Update() {

	if (interpreter != NULL) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(List(), 0);
		}
		interpreter->Update();
	}

}

