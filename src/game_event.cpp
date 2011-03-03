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
#include "game_event.h"
#include "game_actor.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_party.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_system.h"
#include "game_interpreter_map.h"
#include "main_data.h"
#include "player.h"

////////////////////////////////////////////////////////////
Game_Event::Game_Event(int map_id, const RPG::Event& event) :
	starting(false),
	map_id(map_id),
	event(event),
	erased(false),
	through(true),
	page(NULL),
	interpreter(NULL) {

	ID = event.ID;
	
	MoveTo(event.x, event.y);
	Refresh();
}

////////////////////////////////////////////////////////////
Game_Event::~Game_Event() {
	delete interpreter;
}

////////////////////////////////////////////////////////////
void Game_Event::ClearStarting() {
	starting = false;
}

////////////////////////////////////////////////////////////
void Game_Event::Setup(RPG::EventPage* new_page) {
	page = new_page;

	if (page == NULL) {
		tile_id = 0;
		character_name = "";
		character_index = 0;
		direction = 2;
		//move_type = 0;
		through = true;
		trigger = -1;
		list.clear();
		delete interpreter;
		interpreter = NULL;
		return;
	}
	character_name = page->character_name;
	character_index = page->character_index;

	tile_id = page->character_name.empty() ? page->character_index : 0;

	if (original_direction != page->character_direction) {
		switch (page->character_direction) {
		case 0: direction = 8; break;
		case 1: direction = 6; break;
		case 2: direction = 2; break;
		case 3: direction = 4; break;
		};

		original_direction = direction;
		prelock_direction = 0;
	}

	if (original_pattern != page->character_pattern) {
		pattern = page->character_pattern;
		original_pattern = pattern;
	}
	//opacity = page.opacity;
	//opacity = page.translucent ? 192 : 255;
	//blend_type = page.blend_type;
	move_type = page->move_type;
	move_speed = page->move_speed;
	move_frequency = page->move_frequency;
	move_route = &page->move_route;
	move_route_index = 0;
	move_route_forcing = false;
	//animation_type = page.animation_type;
	//through = page;
	//always_on_top = page.overlap;
	priority_type = page->priority_type;
	trigger = page->trigger;
	list = page->event_commands;
	
	// Free resources if needed
	delete interpreter;
	interpreter = NULL;
	if (trigger == TriggerParallelProcess) {
		interpreter = new Game_Interpreter_Map();
	}
	CheckEventTriggerAuto();
}

void Game_Event::Refresh() {
	RPG::EventPage* new_page = NULL;
	if (!erased) {
		std::vector<RPG::EventPage>::reverse_iterator i;
		for (i = event.pages.rbegin(); i != event.pages.rend(); i++) {
			// Loop in reverse order to see whether any page meets conditions...
			if (AreConditionsMet(*i)) {
				new_page = &(*i);
				// Stop looking for more...
				break;
			}
		}
	}

	if (new_page != this->page) {
		ClearStarting();
		Setup(new_page);
		CheckEventTriggerAuto();
	}
}

bool Game_Event::AreConditionsMet(const RPG::EventPage& page) {
	// First switch (A)
	if (page.condition.flags.switch_a && !Game_Switches[page.condition.switch_a_id]) {
		return false;
	}

	// Second switch (B)
	if (page.condition.flags.switch_b && !Game_Switches[page.condition.switch_b_id]) {
		return false;
	}

	// Variable
	if (Player::engine == Player::EngineRpg2k) {
		if (page.condition.flags.variable && !(Game_Variables[page.condition.variable_id] >= page.condition.variable_value)) {
			return false;
		}
	} else {
		if (page.condition.flags.variable) {
			switch (page.condition.compare_operator) {
			case 0: // ==
				if (!(Game_Variables[page.condition.variable_id] == page.condition.variable_value))
					return false;
				break;
			case 1: // >=
				if (!(Game_Variables[page.condition.variable_id] >= page.condition.variable_value))
					return false;
				break;
			case 2: // <=
				if (!(Game_Variables[page.condition.variable_id] <= page.condition.variable_value))
					return false;
				break;
			case 3: // >
				if (!(Game_Variables[page.condition.variable_id] > page.condition.variable_value))
					return false;
				break;
			case 4: // <
				if (!(Game_Variables[page.condition.variable_id] < page.condition.variable_value))
					return false;
				break;
			case 5: // !=
				if (!(Game_Variables[page.condition.variable_id] != page.condition.variable_value))
					return false;
				break;
			}
		}
	}

	// Item in possession?
	if (page.condition.flags.item && !Game_Party::ItemNumber(page.condition.item_id)) {
		return false;
	}

	// Actor in party?
	if (page.condition.flags.actor) {
		if (!Game_Party::IsActorInParty(page.condition.actor_id)) {
			return false;
		}
	}

	// Timer
	if (page.condition.flags.timer) {
		int frames = Game_Party::ReadTimer(Game_Party::Timer1);
		if (frames > page.condition.timer_sec * DEFAULT_FPS)
			return false;
	}

	// Timer2
	if (page.condition.flags.timer2) {
		int frames = Game_Party::ReadTimer(Game_Party::Timer2);
		if (frames > page.condition.timer2_sec * DEFAULT_FPS)
			return false;
	}

	// All conditions met :D
	return true;
}

////////////////////////////////////////////////////////////
int Game_Event::GetId() const {
	return ID;
}

bool Game_Event::GetStarting() const {
	return starting;
}

int Game_Event::GetTrigger() const {
	return trigger;
}

////////////////////////////////////////////////////////////
void Game_Event::SetDisabled(bool dis_flag) {
	erased = dis_flag;
}

bool Game_Event::GetDisabled() const {
	return erased;
}
////////////////////////////////////////////////////////////

void Game_Event::Start() {
	// RGSS scripts consider list empty if size <= 1. Why?
	if (list.empty() || erased) 
		return;

	starting = true;

	if (trigger < 3)
		Lock();

	if (!Game_Map::GetInterpreter().IsRunning()) {
		Game_Map::GetInterpreter().SetupStartingEvent(this);
	}
}

void Game_Event::CheckEventTriggerAuto() {
	if (trigger == TriggerAutoStart)
	{
		Start();
	}
}

std::vector<RPG::EventCommand>& Game_Event::GetList() {
	return list;
}

bool Game_Event::CheckEventTriggerTouch(int x, int y) {
	if (Game_Map::GetInterpreter().IsRunning())
		return false;

	if ((trigger == TriggerHeroOrEventTouch) && (Main_Data::game_player->IsInPosition(x, y))) {

		// TODO check over trigger VX differs from XP here
		if (!IsJumping()) {
			Start();
		}
	}

	return true;
}



void Game_Event::Update() {
	Game_Character::Update();

	CheckEventTriggerAuto();

	if (interpreter != NULL) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(list, event.ID, event.x, event.y);
		}
		interpreter->Update();
	}
	
}

RPG::Event& Game_Event::GetEvent() {
	return event;
}

