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
#include "game_message.h"
#include "game_party.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_system.h"
#include "game_interpreter_map.h"
#include "main_data.h"
#include "player.h"
#include <cmath>

Game_Event::Game_Event(int map_id, const RPG::Event& event) :
	starting(false),
	running(false),
	halting(false),
	trigger(-1),
	event(event),
	page(NULL),
	from_save(false) {

	ID = event.ID;

	SetMapId(map_id);
	MoveTo(event.x, event.y);
	Refresh();
}

Game_Event::Game_Event(int /* map_id */, const RPG::Event& event, const RPG::SaveMapEvent& data) :
	// FIXME unused int parameter
	starting(false),
	running(false),
	halting(false),
	event(event),
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

int Game_Event::GetSpriteDirection() const {
	return data.sprite_direction;
}

void Game_Event::SetSpriteDirection(int new_direction) {
	data.sprite_direction = new_direction;
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
	if (original_move_frequency == -1) {
		original_move_frequency = frequency;
	}
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

bool Game_Event::IsMoveRouteRepeated() const {
	return data.move_route_repeated;
}

void Game_Event::SetMoveRouteRepeated(bool force) {
	data.move_route_repeated = force;
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
	return Color(data.flash_red, data.flash_green, data.flash_blue, 128);
}

void Game_Event::SetFlashColor(const Color& flash_color) {
	data.flash_red = flash_color.red;
	data.flash_blue = flash_color.blue;
	data.flash_green = flash_color.green;
}

double Game_Event::GetFlashLevel() const {
	return data.flash_current_level;
}

void Game_Event::SetFlashLevel(double flash_level) {
	data.flash_current_level = flash_level;
}

int Game_Event::GetFlashTimeLeft() const {
	return data.flash_time_left;
}

void Game_Event::SetFlashTimeLeft(int time_left) {
	data.flash_time_left = time_left;
}

bool Game_Event::IsMessageBlocking() const {
	return Game_Message::message_waiting &&
		!(Game_Message::GetContinueEvents() && Game_Message::owner_id != ID) &&
		!Game_Message::owner_parallel;
}

bool Game_Event::GetThrough() const {
	return page == NULL || Game_Character::GetThrough();
}

void Game_Event::ClearStarting() {
	starting = false;
}

void Game_Event::Setup(RPG::EventPage* new_page) {
	bool from_null = page == NULL;
	page = new_page;

	// Free resources if needed
	if (interpreter) {
		// If the new page is null and the interpreter is running, it should
		// carry on executing its command list during this frame
		if (page)
			interpreter->Clear();
		Game_Map::ReserveInterpreterDeletion(interpreter);
		interpreter.reset();
	}

	if (page == NULL) {
		tile_id = 0;
		SetSpriteName("");
		SetSpriteIndex(0);
		SetDirection(RPG::EventPage::Direction_down);
		//move_type = 0;
		trigger = -1;
		list.clear();
		return;
	}
	SetSpriteName(page->character_name);
	SetSpriteIndex(page->character_index);

	tile_id = page->character_name.empty() ? page->character_index : 0;

	if (original_pattern != page->character_pattern) {
		pattern = page->character_pattern;
		original_pattern = pattern;
	}
	
	move_type = page->move_type;
	SetMoveSpeed(page->move_speed);
	SetMoveFrequency(page->move_frequency);
	max_stop_count = (GetMoveFrequency() > 7) ? 0 : pow(2.0, 8 - GetMoveFrequency());
	original_move_frequency = page->move_frequency;
	original_move_route = page->move_route;
	SetOriginalMoveRouteIndex(0);

	bool last_direction_fixed = IsDirectionFixed() || IsFacingLocked();
	animation_type = page->animation_type;
	if (from_null || !last_direction_fixed || IsDirectionFixed()) {
		SetDirection(page->character_direction);
		SetSpriteDirection(page->character_direction);
	}

	SetOpacity(page->translucent ? 160 : 255);
	SetLayer(page->layer);
	trigger = page->trigger;
	list = page->event_commands;

	if (trigger == RPG::EventPage::Trigger_parallel) {
		interpreter.reset(new Game_Interpreter_Map());
	}
	CheckEventTriggerAuto();
}

void Game_Event::SetupFromSave(RPG::EventPage* new_page) {
	page = new_page;

	if (page == NULL) {
		tile_id = 0;
		through = true;
		trigger = -1;
		list.clear();
		interpreter.reset();
		return;
	}

	data.Fixup(*new_page);

	tile_id = page->character_name.empty() ? page->character_index : 0;

	pattern = page->character_pattern;
	original_pattern = pattern;

	move_type = page->move_type;
	original_move_route = page->move_route;
	animation_type = page->animation_type;
	trigger = page->trigger;
	list = page->event_commands;

	// FIXME: transparency gets not restored otherwise
	SetOpacity(page->translucent ? 160 : 255);

	// Trigger parallel events when the interpreter wasn't already running
	// (because it was the middle of a parallel event while saving)
	if (!interpreter && trigger == RPG::EventPage::Trigger_parallel) {
		interpreter.reset(new Game_Interpreter_Map());
	}
	CheckEventTriggerAuto();
}

void Game_Event::Refresh() {
	if (!data.active) {
		if (from_save) {
			SetVisible(false);
			from_save = false;
		}
		return;
	}

	RPG::EventPage* new_page = NULL;
	std::vector<RPG::EventPage>::reverse_iterator i;
	for (i = event.pages.rbegin(); i != event.pages.rend(); ++i) {
		// Loop in reverse order to see whether any page meets conditions...
		if (AreConditionsMet(*i)) {
			new_page = &(*i);
			// Stop looking for more...
			break;
		}
	}

	// Only update the page pointer when game is loaded,
	// don't setup event, already done
	if (from_save) {
		SetupFromSave(new_page);
		from_save = false;
	}
	else if (new_page != this->page) {
		ClearStarting();
		Setup(new_page);
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
	if (Player::IsRPG2k()) {
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
	if (page.condition.flags.item && !(Main_Data::game_party->GetItemCount(page.condition.item_id)
		+ Main_Data::game_party->GetItemCount(page.condition.item_id, true))) {
			return false;
	}

	// Actor in party?
	if (page.condition.flags.actor) {
		if (!Main_Data::game_party->IsActorInParty(page.condition.actor_id)) {
			return false;
		}
	}

	// Timer
	if (page.condition.flags.timer) {
		int frames = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer1);
		if (frames > page.condition.timer_sec * DEFAULT_FPS)
			return false;
	}

	// Timer2
	if (page.condition.flags.timer2) {
		int frames = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer2);
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

void Game_Event::SetActive(bool active) {
	data.active = active;
	SetVisible(active);
}

bool Game_Event::GetActive() const {
	return data.active;
}

void Game_Event::Start() {
	// RGSS scripts consider list empty if size <= 1. Why?
	if (list.empty() || !data.active || running)
		return;

	starting = true;
}

void Game_Event::CheckEventTriggerAuto() {
	if (trigger == RPG::EventPage::Trigger_auto_start && Game_Map::GetReady()) {
		Start();
	}
}

std::vector<RPG::EventCommand>& Game_Event::GetList() {
	return list;
}

void Game_Event::StartTalkToHero() {
	if (!(IsDirectionFixed() || IsFacingLocked())) {
		int prelock_dir = GetDirection();
		TurnTowardHero();
		SetDirection(prelock_dir);
	}
}

void Game_Event::StopTalkToHero() {
	if (!(IsDirectionFixed() || IsFacingLocked())) {
		SetSpriteDirection(GetDirection());
	}

	halting = true;
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
	if (!data.active || page == NULL) {
		return;
	}

	CheckEventTriggerAuto();

	if (interpreter) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(list, event.ID, -event.x, event.y);
		} else {
			interpreter->Update();
		}
	} else if (starting && !Game_Map::GetInterpreter().IsRunning()) {
		Game_Map::GetInterpreter().SetupStartingEvent(this);
		running = true;
	}

	Game_Character::Update();

	if (halting) {
		running = false;
		halting = false;
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
