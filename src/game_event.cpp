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
#include "utils.h"
#include <cmath>

Game_Event::Game_Event(int map_id, const RPG::Event& event) :
	Game_Character(new RPG::SaveMapEvent()),
	_data_copy(this->data()),
	event(event),
	from_save(false) {

	SetMapId(map_id);
	MoveTo(event.x, event.y);
	Refresh();
}

Game_Event::Game_Event(int /* map_id */, const RPG::Event& event, const RPG::SaveMapEvent& orig_data) :
	Game_Character(new RPG::SaveMapEvent(orig_data)),
	_data_copy(this->data()),
	// FIXME unused int parameter
	event(event),
	from_save(true) {

	this->event.ID = data()->ID;

	MoveTo(data()->position_x, data()->position_y);

	if (!data()->event_data.commands.empty()) {
		interpreter.reset(new Game_Interpreter_Map());
		static_cast<Game_Interpreter_Map*>(interpreter.get())->SetupFromSave(data()->event_data.commands);
	}

	Refresh();
}

int Game_Event::GetMoveFrequency() const {
	return data()->move_frequency;
}

void Game_Event::SetMoveFrequency(int frequency) {
	data()->move_frequency = frequency;
	if (original_move_frequency == -1) {
		original_move_frequency = frequency;
	}
}

int Game_Event::GetOriginalMoveRouteIndex() const {
	return data()->original_move_route_index;
}

void Game_Event::SetOriginalMoveRouteIndex(int new_index) {
	data()->original_move_route_index = new_index;
}

bool Game_Event::GetThrough() const {
	return page == nullptr || data()->through;
}

void Game_Event::SetThrough(bool through) {
	data()->through = through;
}

void Game_Event::ClearStarting() {
	starting = false;
	started_by_decision_key = false;
}

void Game_Event::Setup(const RPG::EventPage* new_page) {
	bool from_null = page == nullptr;

	const RPG::EventPage* old_page = page;
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

	if (page == nullptr) {
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

	SetAnimFrame(AnimFrame(page->character_pattern));

	move_type = page->move_type;
	SetMoveSpeed(page->move_speed);
	SetMoveFrequency(page->move_frequency);
	if (!IsMoveRouteOverwritten()) {
		SetMaxStopCount((GetMoveFrequency() > 7) ? 0 : (int) pow(2.0, 8 - GetMoveFrequency()));
	}
	original_move_frequency = page->move_frequency;
	original_move_route = page->move_route;
	SetOriginalMoveRouteIndex(0);

	bool same_direction_as_on_old_page = old_page && old_page->character_direction == new_page->character_direction;
	SetAnimationType(RPG::EventPage::AnimType(page->animation_type));

	if (from_null || !(same_direction_as_on_old_page || IsMoving())) {
		SetSpriteDirection(page->character_direction);
		SetDirection(page->character_direction);
	}

	if (IsDirectionFixed()) {
		SetSpriteDirection(page->character_direction);
	}

	SetTransparency(page->translucent ? 3 : 0);
	SetLayer(page->layer);
	data()->overlap_forbidden = page->overlap_forbidden;
	trigger = page->trigger;
	list = page->event_commands;

	if (trigger == RPG::EventPage::Trigger_parallel) {
		interpreter.reset(new Game_Interpreter_Map());
	}
}

void Game_Event::SetupFromSave(const RPG::EventPage* new_page) {
	page = new_page;

	if (page == nullptr) {
		trigger = -1;
		list.clear();
		interpreter.reset();
		return;
	}

	move_type = page->move_type;
	original_move_route = page->move_route;
	trigger = page->trigger;
	list = page->event_commands;

	// Trigger parallel events when the interpreter wasn't already running
	// (because it was the middle of a parallel event while saving)
	if (!interpreter && trigger == RPG::EventPage::Trigger_parallel) {
		interpreter.reset(new Game_Interpreter_Map());
	}
}

void Game_Event::Refresh() {
	if (!data()->active) {
		if (from_save) {
			SetVisible(false);
			from_save = false;
		}
		return;
	}

	RPG::EventPage* new_page = nullptr;
	std::vector<RPG::EventPage>::reverse_iterator i;
	for (i = event.pages.rbegin(); i != event.pages.rend(); ++i) {
		// Loop in reverse order to see whether any page meets conditions...
		if (AreConditionsMet(*i)) {
			new_page = &(*i);
			SetVisible(true);
			// Stop looking for more...
			break;
		}
	}

	if (!new_page) {
		SetVisible(false);
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
	if (page.condition.flags.switch_a && !Game_Switches.Get(page.condition.switch_a_id)) {
		return false;
	}

	// Second switch (B)
	if (page.condition.flags.switch_b && !Game_Switches.Get(page.condition.switch_b_id)) {
		return false;
	}

	// Variable
	if (Player::IsRPG2k()) {
		if (page.condition.flags.variable && !(Game_Variables.Get(page.condition.variable_id) >= page.condition.variable_value)) {
			return false;
		}
	} else {
		if (page.condition.flags.variable) {
			switch (page.condition.compare_operator) {
			case 0: // ==
				if (!(Game_Variables.Get(page.condition.variable_id) == page.condition.variable_value))
					return false;
				break;
			case 1: // >=
				if (!(Game_Variables.Get(page.condition.variable_id) >= page.condition.variable_value))
					return false;
				break;
			case 2: // <=
				if (!(Game_Variables.Get(page.condition.variable_id) <= page.condition.variable_value))
					return false;
				break;
			case 3: // >
				if (!(Game_Variables.Get(page.condition.variable_id) > page.condition.variable_value))
					return false;
				break;
			case 4: // <
				if (!(Game_Variables.Get(page.condition.variable_id) < page.condition.variable_value))
					return false;
				break;
			case 5: // !=
				if (!(Game_Variables.Get(page.condition.variable_id) != page.condition.variable_value))
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
		int secs = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer1);
		if (secs > page.condition.timer_sec)
			return false;
	}

	// Timer2
	if (page.condition.flags.timer2) {
		int secs = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer2);
		if (secs > page.condition.timer2_sec)
			return false;
	}

	// All conditions met :D
	return true;
}

int Game_Event::GetId() const {
	return event.ID;
}

std::string Game_Event::GetName() const {
	return event.name;
}

bool Game_Event::GetStarting() const {
	return starting;
}

bool Game_Event::WasStartedByDecisionKey() const {
	return started_by_decision_key;
}

int Game_Event::GetTrigger() const {
	return trigger;
}

void Game_Event::SetActive(bool active) {
	data()->active = active;
	SetVisible(active);
}

bool Game_Event::GetActive() const {
	return data()->active;
}

void Game_Event::Start(bool by_decision_key) {
	// RGSS scripts consider list empty if size <= 1. Why?
	if (list.empty() || !data()->active)
		return;

	starting = true;
	started_by_decision_key = by_decision_key;
}

const std::vector<RPG::EventCommand>& Game_Event::GetList() const {
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

void Game_Event::CheckEventTriggers() {
	if (trigger == RPG::EventPage::Trigger_auto_start) {
		if (Player::GetFrames() == frame_count_at_last_auto_start_check) {
			// Delay the start to the next frame because the event was enabled
			// by an event with an higher ID than this event.
			return;
		}
		Start();
	} else if (trigger == RPG::EventPage::Trigger_collision) {
		CheckEventTriggerTouch(GetX(),GetY());
	}
}

bool Game_Event::CheckEventTriggerTouch(int x, int y) {
	if (Game_Map::GetInterpreter().IsRunning())
		return false;

	if (trigger == RPG::EventPage::Trigger_collision) {
		if (Main_Data::game_player->IsInPosition(GetX(), GetY()) && GetLayer() == RPG::EventPage::Layers_same) {
			return false;
		}

		if (Main_Data::game_player->IsInPosition(x, y)) {
			if (Main_Data::game_player->InAirship() && GetLayer() == RPG::EventPage::Layers_same) {
				return false;
			}

			Start();
			return true;
		}
	}

	return false;
}

void Game_Event::UpdateSelfMovement() {
	if (running)
		return;
	if (!Game_Message::GetContinueEvents() && Game_Map::GetInterpreter().IsRunning())
		return;
	if (!IsStopping())
		return;

	switch (move_type) {
	case RPG::EventPage::MoveType_random:
		MoveTypeRandom();
		break;
	case RPG::EventPage::MoveType_vertical:
		MoveTypeCycleUpDown();
		break;
	case RPG::EventPage::MoveType_horizontal:
		MoveTypeCycleLeftRight();
		break;
	case RPG::EventPage::MoveType_toward:
		MoveTypeTowardsPlayer();
		break;
	case RPG::EventPage::MoveType_away:
		MoveTypeAwayFromPlayer();
		break;
	case RPG::EventPage::MoveType_custom:
		MoveTypeCustom();
		break;
	}
}

void Game_Event::MoveTypeRandom() {
	int last_direction = GetDirection();
	switch (Utils::GetRandomNumber(0, 5)) {
	case 0:
		SetStopCount(GetStopCount() - Utils::GetRandomNumber(0, GetStopCount()));
		if (GetStopCount() < 0) {
			SetStopCount(0);
		}
		return;
	case 1:
		MoveForward();
		break;
	default:
		MoveRandom();
	}
	if (move_failed && !starting) {
		SetDirection(last_direction);
		if (!(IsDirectionFixed() || IsFacingLocked()))
			SetSpriteDirection(last_direction);
	} else {
		SetMaxStopCount(GetMaxStopCount() / 5 * Utils::GetRandomNumber(3, 6));
	}
}

void Game_Event::MoveTypeCycle(int default_dir) {
	SetMaxStopCount((GetMoveFrequency() > 7) ? 0 : (1 << (9 - GetMoveFrequency())));
	if (GetStopCount() < GetMaxStopCount()) return;

	int non_default_dir = ReverseDir(default_dir);
	int move_dir = GetDirection();
	if (!(move_dir == default_dir || move_dir == non_default_dir)) {
		move_dir = default_dir;
	}

	Move(move_dir, MoveOption::IgnoreIfCantMove);

	if (move_failed && GetStopCount() >= GetMaxStopCount() + 20) {
		if (GetStopCount() >= GetMaxStopCount() + 60) {
			Move(ReverseDir(move_dir));
			SetStopCount(0);
		} else {
			Move(ReverseDir(move_dir), MoveOption::IgnoreIfCantMove);
		}
	}
}

void Game_Event::MoveTypeCycleLeftRight() {
	MoveTypeCycle(Right);
}

void Game_Event::MoveTypeCycleUpDown() {
	MoveTypeCycle(Down);
}

void Game_Event::MoveTypeTowardsPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();
	int last_direction = GetDirection();

	if ( std::abs(sx) + std::abs(sy) >= 20 ) {
		MoveRandom();
	} else {
		switch (Utils::GetRandomNumber(0, 5)) {
		case 0:
			MoveRandom();
			break;
		case 1:
			MoveForward();
			break;
		default:
			MoveTowardsPlayer();
		}
	}

	if (move_failed && !starting) {
		if (GetStopCount() >= GetMaxStopCount() + 60) {
			SetStopCount(0);
		} else {
			SetDirection(last_direction);
			if (!(IsDirectionFixed() || IsFacingLocked()))
				SetSpriteDirection(last_direction);
		}
	}
}

void Game_Event::MoveTypeAwayFromPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();
	int last_direction = GetDirection();

	if ( std::abs(sx) + std::abs(sy) >= 20 ) {
		MoveRandom();
	} else {
		switch (Utils::GetRandomNumber(0, 5)) {
		case 0:
			MoveRandom();
			break;
		case 1:
			MoveForward();
			break;
		default:
			MoveAwayFromPlayer();
		}
	}

	if (move_failed && !starting) {
		if (GetStopCount() >= GetMaxStopCount() + 60) {
			SetStopCount(0);
		} else {
			SetDirection(last_direction);
			if (!(IsDirectionFixed() || IsFacingLocked()))
				SetSpriteDirection(last_direction);
		}
	}
}

void Game_Event::Update() {
	if (!data()->active || page == NULL) {
		return;
	}

	Game_Character::UpdateSprite();

	if (starting && !Game_Map::GetInterpreter().IsRunning()) {
		Game_Map::GetInterpreter().Setup(this);
		Game_Map::GetInterpreter().Update();
		running = true;
	}

	if (halting) {
		running = false;
		halting = false;
	}
}

void Game_Event::UpdateParallel() {
	int cur_frame_count = Player::GetFrames();

	if (trigger != RPG::EventPage::Trigger_auto_start) {
		// When this event becomes an auto-start event this frame through a
		// refresh don't allow to run it.
		// This prevents events  with lower IDs than the event doing the
		// refresh from starting this frame.
		frame_count_at_last_auto_start_check = cur_frame_count;
	}

	if (!data()->active || page == NULL || updating) {
		return;
	}

	updating = true;

	if (interpreter) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(this);
		}
		interpreter->Update();
	}

	// Placed after the interpreter update because multiple updates per frame are allowed.
	// This results in waits to finish quicker when an event collides with this event and
	// emulates a RPG Maker bug)
	// Only update the event once per frame
	if (cur_frame_count == frame_count_at_last_update_parallel) {
		updating = false;
		return;
	}
	frame_count_at_last_update_parallel = cur_frame_count;

	Game_Character::Update();
	updating = false;
}

const RPG::EventPage* Game_Event::GetPage(int page) const {
	if (page <= 0 || page - 1 >= static_cast<int>(event.pages.size())) {
		return nullptr;
	}
	return &event.pages[page - 1];
}

const RPG::EventPage *Game_Event::GetActivePage() const {
	return page;
}

const RPG::SaveMapEvent& Game_Event::GetSaveData() {
	if (interpreter) {
		data()->event_data.commands = static_cast<Game_Interpreter_Map*>(interpreter.get())->GetSaveData();
	}
	data()->ID = event.ID;

	return *data();
}
