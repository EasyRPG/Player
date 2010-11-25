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
#include "game_switches.h"
#include "game_variables.h"

////////////////////////////////////////////////////////////
Game_Event::Game_Event(int map_id, RPG::Event event) :
	starting(false),
	map_id(map_id),
	event(event),
	erased(false),
	through(true) {

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
void Game_Event::Refresh() {
	RPG::EventPage new_page;
	if (!erased) {
		RPG::Event a = event;
		for (int i = (int)event.pages.size() - 1; i >= 0; i--) {
			if	((event.pages[i].condition.switch_a && !Game_Switches[event.pages[i].condition.switch_a_id]) ||
				(event.pages[i].condition.switch_b && !Game_Switches[event.pages[i].condition.switch_b_id]) ||
				(event.pages[i].condition.variable && Game_Variables[event.pages[i].condition.variable_id] < event.pages[i].condition.variable_value)) {
				continue;
			}
			new_page = event.pages[i];
			break;
		}
	}

	if (new_page.ID == page.ID)
		return;

	page = new_page;

	ClearStarting();

	if (page.ID == 0) {
		tile_id = 0;
		character_name = "";
		character_index = 0;
		//move_type = 0;
		through = true;
		trigger = -1;
		list.clear();
		interpreter = NULL;
		return;
	}
	character_name = page.character_name;
	character_index = page.character_index;
	pattern = page.character_pattern;
	/*if (original_direction != page.character_dir) {
		direction = page.character_dir;
		original_direction = direction;
		prelock_direction = 0;
	}
	if (original_pattern != page.character_pattern) {
		pattern = page.character_pattern;
		original_pattern = pattern;
	}*/
	//opacity = page.opacity;
	//opacity = page.translucent ? 192 : 255;
	//blend_type = page.blend_type;
	//move_type = page.move_type;
	move_speed = page.move_speed;
	move_frequency = page.move_frequency;
	//move_route = page.move_route;
	//move_route_index = 0;
	move_route_forcing = false;
	//animation_type = page.animation_type;
	//through = page;
	//always_on_top = page.overlap;
	trigger = page.trigger;
	list = page.event_commands;
	interpreter = NULL;
	//if (trigger == 4)
	//	interpreter = new Game_Interpreter();
	//CheckEventTriggerAuto();
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

std::vector<RPG::EventCommand> Game_Event::GetList() {
	return list;
}
