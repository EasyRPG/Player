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

Game_Event::Game_Event(int map_id, const RPG::Event& event) :
	starting(false),
	event(event),
	erased(false),
	page(NULL),
	from_save(false) {

	ID = event.ID;
	through = true;

	SetMapId(map_id);
	MoveTo(event.x, event.y);
	Refresh();
}

Game_Event::Game_Event(int map_id, const RPG::Event& event, const RPG::SaveMapEvent& data) :
	starting(false),
	event(event),
	erased(false),
	page(NULL),
	from_save(true) {

	ID = data.ID;

	this->data = data;
	MoveTo(data.position_x, data.position_y);
	
	if (!data.event_data.commands.empty()) {
		interpreter.reset(new Game_Interpreter_Map());
		static_cast<Game_Interpreter_Map*>(interpreter.get())->SetupFromSave(data.event_data.commands, event.ID);
	}

	Refresh();
}

int Game_Event::GetX() const {
	return data.position_x;
}

void Game_Event::SetX(int new_x) {
	data.position_x = new_x;
}

int Game_Event::GetY() const {
	return data.position_y;
}

void Game_Event::SetY(int new_y) {
	data.position_y = new_y;
}

int Game_Event::GetMapId() const {
	return data.map_id;
}

void Game_Event::SetMapId(int new_map_id) {
	data.map_id = new_map_id;
}

int Game_Event::GetDirection() const {
	return data.direction;
}

void Game_Event::SetDirection(int new_direction) {
	data.direction = new_direction;
}

int Game_Event::GetPrelockDirection() const {
	return data.prelock_direction;
}

void Game_Event::SetPrelockDirection(int new_direction) {
	data.prelock_direction = new_direction;
}

bool Game_Event::IsFacingLocked() const {
	return data.lock_facing;
}

void Game_Event::SetFacingLocked(bool locked) {
	data.lock_facing = locked;
}

int Game_Event::GetLayer() const {
	return data.layer;
}

void Game_Event::SetLayer(int new_layer) {
	data.layer = new_layer;
}

int Game_Event::GetMoveSpeed() const {
	return data.move_speed;
}

void Game_Event::SetMoveSpeed(int speed) {
	data.move_speed = speed;
}

int Game_Event::GetMoveFrequency() const {
	return data.move_frequency;
}

void Game_Event::SetMoveFrequency(int frequency) {
	data.move_frequency = frequency;
}

const RPG::MoveRoute& Game_Event::GetMoveRoute() const {
	return data.move_route;
}

void Game_Event::SetMoveRoute(const RPG::MoveRoute& move_route) {
	data.move_route = move_route;
}

int Game_Event::GetOriginalMoveRouteIndex() const {
	return data.original_move_route_index;
}

void Game_Event::SetOriginalMoveRouteIndex(int new_index) {
	data.original_move_route_index = new_index;
}

int Game_Event::GetMoveRouteIndex() const {
	return data.move_route_index;
}

void Game_Event::SetMoveRouteIndex(int new_index) {
	data.move_route_index = new_index;
}

bool Game_Event::IsMoveRouteOverwritten() const {
	return data.move_route_overwrite;
}

void Game_Event::SetMoveRouteOverwritten(bool force) {
	data.move_route_overwrite = force;
}

const std::string& Game_Event::GetSpriteName() const {
	return data.sprite_name;
}

void Game_Event::SetSpriteName(const std::string& sprite_name) {
	data.sprite_name = sprite_name;
}

int Game_Event::GetSpriteIndex() const {
	return data.sprite_id;
}

void Game_Event::SetSpriteIndex(int index) {
	data.sprite_id = index;
}

Color Game_Event::GetFlashColor() const {
	return Color(data.flash_red, data.flash_green, data.flash_blue, 0);
}

void Game_Event::SetFlashColor(const Color& flash_color) {
	data.flash_red = flash_color.red;
	data.flash_blue = flash_color.blue;
	data.flash_green = flash_color.green;
}

int Game_Event::GetFlashLevel() const {
	return data.flash_current_level;
}

void Game_Event::SetFlashLevel(int flash_level) {
	data.flash_current_level = flash_level;
}

int Game_Event::GetFlashTimeLeft() const {
	return data.flash_time_left;
}

void Game_Event::SetFlashTimeLeft(int time_left) {
	data.flash_time_left = time_left;
}

void Game_Event::ClearStarting() {
	starting = false;
}

void Game_Event::Setup(RPG::EventPage* new_page) {
	page = new_page;

	if (page == NULL) {
		tile_id = 0;
		SetSpriteName("");
		SetSpriteIndex(0);
		SetFacingDirection(RPG::EventPage::Direction_down);
		//move_type = 0;
		through = true;
		trigger = -1;
		list.clear();
		interpreter.reset();
		return;
	}
	SetSpriteName(page->character_name);
	SetSpriteIndex(page->character_index);

	tile_id = page->character_name.empty() ? page->character_index : 0;

	if (GetDirection() != page->character_direction) {
		SetFacingDirection(page->character_direction);
		SetPrelockDirection(-1);
	}

	if (original_pattern != page->character_pattern) {
		pattern = page->character_pattern;
		original_pattern = pattern;
	}
	//opacity = page.opacity;
	//opacity = page.translucent ? 192 : 255;
	//blend_type = page.blend_type;
	move_type = page->move_type;
	SetMoveSpeed(page->move_speed);
	SetMoveFrequency(page->move_frequency);
	original_move_route = page->move_route;
	SetOriginalMoveRouteIndex(0);
	SetMoveRouteOverwritten(false);
	animation_type = page->animation_type;

	SetLayer(page->layer);
	trigger = page->trigger;
	list = page->event_commands;
	through = false;

	// Free resources if needed
	interpreter.reset();
	if (trigger == RPG::EventPage::Trigger_parallel) {
		interpreter.reset(new Game_Interpreter_Map());
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

	// Only update the page pointer when game is loaded,
	// don't setup event, already done
	if (from_save) {
		page = new_page;
		original_move_route = page->move_route;
		original_move_frequency = page->move_frequency;
		data.Fixup(*page);
		from_save = false;
	}
	else if (new_page != this->page) {
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

int Game_Event::GetId() const {
	return ID;
}

bool Game_Event::GetStarting() const {
	return starting;
}

int Game_Event::GetTrigger() const {
	return trigger;
}

void Game_Event::SetDisabled(bool dis_flag) {
	erased = dis_flag;
}

bool Game_Event::GetDisabled() const {
	return erased;
}

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
	if (trigger == RPG::EventPage::Trigger_auto_start)
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

	if ((trigger == RPG::EventPage::Trigger_collision) && (Main_Data::game_player->IsInPosition(x, y))) {

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

	if (interpreter) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(list, event.ID, -event.x, event.y);
		}
		interpreter->Update();
	}

}

RPG::Event& Game_Event::GetEvent() {
	return event;
}

const RPG::SaveMapEvent& Game_Event::GetSaveData() {
	if (interpreter) {
		data.event_data.commands = static_cast<Game_Interpreter_Map*>(interpreter.get())->GetSaveData();
	}
	data.ID = ID;

	return data;
}
