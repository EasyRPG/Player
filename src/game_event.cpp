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
	Game_Character(Event, new RPG::SaveMapEvent()),
	_data_copy(this->data()),
	event(event),
	from_save(false)
{
	SetMapId(map_id);
	SetMoveSpeed(3);
	MoveTo(event.x, event.y);
	Refresh();
}

Game_Event::Game_Event(int map_id, const RPG::Event& event, const RPG::SaveMapEvent& orig_data) :
	Game_Character(Event, new RPG::SaveMapEvent(orig_data)),
	_data_copy(this->data()),
	event(event),
	from_save(true)
{
	// Savegames have 0 for the mapid for compatibility with RPG_RT.
	SetMapId(map_id);

	this->event.ID = data()->ID;

	if (!data()->parallel_event_execstate.stack.empty()) {
		interpreter.reset(new Game_Interpreter_Map());
		static_cast<Game_Interpreter_Map*>(interpreter.get())->SetupFromSave(data()->parallel_event_execstate.stack);
	}

	Refresh();
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

void Game_Event::ClearWaitingForegroundExecution() {
	data()->waiting_execution = false;
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

	SetPaused(false);

	if (page == nullptr) {
		SetSpriteName("");
		SetSpriteIndex(0);
		SetDirection(RPG::EventPage::Direction_down);
		trigger = -1;
		list.clear();
		return;
	}

	SetSpriteName(page->character_name);
	SetSpriteIndex(page->character_index);

	SetMoveSpeed(page->move_speed);
	SetMoveFrequency(page->move_frequency);
	if (page->move_type == RPG::EventPage::MoveType_custom) {
		SetMaxStopCountForTurn();
	} else {
		SetMaxStopCountForStep();
	}
	original_move_frequency = page->move_frequency;
	SetOriginalMoveRouteIndex(0);

	bool same_direction_as_on_old_page = old_page && old_page->character_direction == new_page->character_direction;
	SetAnimationType(RPG::EventPage::AnimType(page->animation_type));

	if (GetAnimationType() == RPG::EventPage::AnimType_fixed_graphic
			|| GetAnimationType() == RPG::EventPage::AnimType_spin) {
		SetAnimFrame(page->character_pattern);
	}

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

	original_move_frequency = page->move_frequency;
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
		ClearWaitingForegroundExecution();
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
		int secs = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer1);
		if (secs > page.condition.timer_sec)
			return false;
	}

	// Timer2
	if (page.condition.flags.timer2) {
		int secs = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer2);
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

bool Game_Event::IsWaitingForegroundExecution() const {
	return data()->waiting_execution;
}

bool Game_Event::WasStartedByDecisionKey() const {
	return data()->triggered_by_decision_key;
}

RPG::EventPage::Trigger Game_Event::GetTrigger() const {
	return static_cast<RPG::EventPage::Trigger>(trigger);
}


bool Game_Event::SetAsWaitingForegroundExecution(bool face_hero, bool by_decision_key) {
	// RGSS scripts consider list empty if size <= 1. Why?
	if (list.empty() || !data()->active) {
		return false;
	}

	if (face_hero && !(IsDirectionFixed() || IsFacingLocked() || IsSpinning())) {
		SetSpriteDirection(GetDirectionToHero());
	}

	data()->waiting_execution = true;
	data()->triggered_by_decision_key = by_decision_key;
	SetPaused(true);

	return true;
}

const std::vector<RPG::EventCommand>& Game_Event::GetList() const {
	return list;
}

void Game_Event::OnFinishForegroundEvent() {
	if (!(IsDirectionFixed() || IsFacingLocked() || IsSpinning())) {
		SetSpriteDirection(GetDirection());
	}
	SetPaused(false);
}

void Game_Event::CheckEventAutostart() {
	if (trigger == RPG::EventPage::Trigger_auto_start
			&& GetRemainingStep() == 0) {
		SetAsWaitingForegroundExecution(false, false);
		return;
	}
}

void Game_Event::CheckEventCollision() {
	if (trigger == RPG::EventPage::Trigger_collision
			&& GetLayer() != RPG::EventPage::Layers_same
			&& !Main_Data::game_player->IsMoveRouteOverwritten()
			&& !Game_Map::GetInterpreter().IsRunning()
			&& !Main_Data::game_player->InAirship()
			&& Main_Data::game_player->IsInPosition(GetX(), GetY())) {
		SetAsWaitingForegroundExecution(true, false);
		return;
	}
}

void Game_Event::OnMoveFailed(int x, int y) {
	if (Main_Data::game_player->InAirship()
			|| GetLayer() != RPG::EventPage::Layers_same
			|| trigger != RPG::EventPage::Trigger_collision) {
		return;
	}

	if (Main_Data::game_player->IsInPosition(x, y)) {
		SetAsWaitingForegroundExecution(false, false);
		// Events with trigger collision and layer same always reset their
		// stop_count when they fail movement to a tile that the player inhabits.
		SetStopCount(0);
		return;
	}
}

void Game_Event::UpdateSelfMovement() {
	if (IsPaused())
		return;
	if (IsMoveRouteOverwritten()) {
		return;
	}
	if (!Game_Message::GetContinueEvents() && Game_Map::GetInterpreter().IsRunning())
		return;
	if (!IsStopping())
		return;
	if (page == nullptr) {
		return;
	}
	if (IsStopCountActive()) {
		return;
	}

	switch (page->move_type) {
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
		UpdateMoveRoute(data()->original_move_route_index, page->move_route);
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
	if (move_failed) {
		SetDirection(last_direction);
		if (!(IsDirectionFixed() || IsFacingLocked()))
			SetSpriteDirection(last_direction);
	} else {
		SetMaxStopCount(GetMaxStopCount() / 5 * Utils::GetRandomNumber(3, 6));
	}
}

void Game_Event::MoveTypeCycle(int default_dir) {
	SetMaxStopCountForStep();
	if (GetStopCount() < GetMaxStopCount()) return;

	const int reverse_dir = ReverseDir(default_dir);
	int move_dir = GetDirection();
	if (move_dir != reverse_dir) {
		move_dir = default_dir;
	}

	Move(move_dir, MoveOption::IgnoreIfCantMove);

	if (move_failed) {
		if (GetStopCount() >= GetMaxStopCount() + 20) {
			if (GetStopCount() >= GetMaxStopCount() + 60) {
				Move(ReverseDir(move_dir));
				SetStopCount(0);
			} else {
				Move(ReverseDir(move_dir), MoveOption::IgnoreIfCantMove);
			}
		}
	}
}

void Game_Event::MoveTypeCycleLeftRight() {
	MoveTypeCycle(Right);
}

void Game_Event::MoveTypeCycleUpDown() {
	MoveTypeCycle(Down);
}

void Game_Event::MoveTypeTowardsOrAwayPlayer(bool towards) {
	int sx = GetScreenX();
	int sy = GetScreenY();

	constexpr int offset = TILE_SIZE * 2;

	const bool in_sight = (sx >= -offset && sx <= SCREEN_TARGET_WIDTH + offset
			&& sy >= -offset && sy <= SCREEN_TARGET_HEIGHT + offset);

	int dir = 0;
	if (!in_sight) {
		dir = Utils::GetRandomNumber(0, 3);
	} else {
		int draw = Utils::GetRandomNumber(0, 9);
		if (draw == 0) {
			dir = GetDirection();
		} else if(draw == 1) {
			dir = Utils::GetRandomNumber(0, 3);
		} else {
			dir = towards
				? GetDirectionToHero()
				: GetDirectionAwayHero();
		}
	}

	const bool stop_limit = (GetStopCount() >= 60);

	const auto move_opt = stop_limit
		? MoveOption::Normal
		: MoveOption::IgnoreIfCantMove;

	Move(dir, move_opt);

	if (move_failed && stop_limit) {
		SetStopCount(0);
	}
}

void Game_Event::MoveTypeTowardsPlayer() {
	MoveTypeTowardsOrAwayPlayer(true);
}

void Game_Event::MoveTypeAwayFromPlayer() {
	MoveTypeTowardsOrAwayPlayer(false);
}

void Game_Event::Update() {
	if (!data()->active || page == NULL) {
		return;
	}

	// RPG_RT runs the parallel interpreter everytime Update is called.
	// That means if the event updates multiple times due to makeway,
	// the interpreter will run multiple times per frame.
	// This results in event waits to finish quicker during collisions as
	// the wait will tick by 1 each time the interpreter is invoked.
	if (trigger == RPG::EventPage::Trigger_parallel && interpreter) {
		if (!interpreter->IsRunning()) {
			interpreter->Setup(this);
		}
		interpreter->Update();

		// RPG_RT only exits if active is false here, but not if there is
		// no active page...
		if (!data()->active) {
			return;
		}
	}

	if (IsProcessed()) {
		return;
	}
	SetProcessed(true);

	CheckEventAutostart();

	if (!IsMoveRouteOverwritten() || IsMoving()) {
		CheckEventCollision();
	}

	auto was_moving = !IsStopping();
	Game_Character::UpdateMovement();
	Game_Character::UpdateAnimation(was_moving);
	Game_Character::UpdateFlash();

	if (IsStopping()) {
		CheckEventCollision();
	}
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
		data()->parallel_event_execstate.stack = static_cast<Game_Interpreter_Map*>(interpreter.get())->GetSaveData();
	}
	data()->ID = event.ID;

	return *data();
}

bool Game_Event::IsMoveRouteActive() const {
	return true;
}
