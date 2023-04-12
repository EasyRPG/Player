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
#include "rand.h"
#include "output.h"
#include <cmath>
#include <cassert>

Game_Event::Game_Event(int map_id, const lcf::rpg::Event* event) :
	Game_EventBase(Event),
	event(event)
{
	data()->ID = event->ID;
	SetMapId(map_id);
	SetX(event->x);
	SetY(event->y);

	RefreshPage();
}

void Game_Event::SanitizeData() {
	StringView name = event->name;
	Game_Character::SanitizeData(name);
	if (page != nullptr) {
		SanitizeMoveRoute(name, page->move_route, data()->original_move_route_index, "original_move_route_index");
	}
}

void Game_Event::SetSaveData(lcf::rpg::SaveMapEvent save)
{
	// 2k Savegames have 0 for the mapid for compatibility with RPG_RT.
	auto map_id = GetMapId();
	*data() = std::move(save);

	data()->ID = event->ID;
	SetMapId(map_id);

	SanitizeData();

	if (!data()->active || page == nullptr) {
		return;
	}

	if (interpreter) {
		interpreter->Clear();
	}

	if (GetTrigger() == lcf::rpg::EventPage::Trigger_parallel) {
		auto& state = data()->parallel_event_execstate;
		// RPG_RT Savegames have empty stacks for parallel events.
		// We are LSD compatible but don't load these into interpreter.
		bool has_state = (!state.stack.empty() && !state.stack.front().commands.empty());
		// If the page changed before save but the event never updated,
		// there will be not stack but we still need to create an interpreter
		// for the event page commands.
		if (has_state || !page->event_commands.empty()) {
			if (!interpreter) {
				interpreter.reset(new Game_Interpreter_Map());
			}
		}

		if (has_state) {
			interpreter->SetState(state);
		}
	}
}

lcf::rpg::SaveMapEvent Game_Event::GetSaveData() const {
	auto save = *data();

	lcf::rpg::SaveEventExecState state;
	if (page && page->trigger == lcf::rpg::EventPage::Trigger_parallel) {
		if (interpreter) {
			state = interpreter->GetState();
		}

		if (state.stack.empty() && page->event_commands.empty()) {
			// RPG_RT always stores an empty stack frame for empty parallel events.
			lcf::rpg::SaveEventExecFrame frame;
			frame.event_id = GetId();
			state.stack.push_back(std::move(frame));
		}
	}
	save.parallel_event_execstate = std::move(state);

	return save;
}

Drawable::Z_t Game_Event::GetScreenZ(bool apply_shift) const {
	// Lowest 16 bit are reserved for the ID
	// See base function for full explanation
	return Game_Character::GetScreenZ(apply_shift) + GetId();
}

int Game_Event::GetOriginalMoveRouteIndex() const {
	return data()->original_move_route_index;
}

void Game_Event::SetOriginalMoveRouteIndex(int new_index) {
	data()->original_move_route_index = new_index;
}

void Game_Event::ClearWaitingForegroundExecution() {
	data()->waiting_execution = false;
}

static bool CompareMoveRouteCommandCodes(const lcf::rpg::MoveRoute& l, const lcf::rpg::MoveRoute& r) {
	auto& lmc = l.move_commands;
	auto& rmc = r.move_commands;
	if (lmc.size() != rmc.size()) {
		return false;
	}

	for (size_t i = 0; i < lmc.size(); ++i) {
		if (lmc[i].command_id != rmc[i].command_id) {
			return false;
		}
	}
	return true;
}

void Game_Event::RefreshPage() {
	const lcf::rpg::EventPage* new_page = nullptr;
	for (auto i = event->pages.crbegin(); i != event->pages.crend(); ++i) {
		// Loop in reverse order to see whether any page meets conditions...
		if (AreConditionsMet(*i)) {
			new_page = &(*i);
			// Stop looking for more...
			break;
		}
	}

	if (!new_page) {
		ClearWaitingForegroundExecution();
		SetPaused(false);
		SetThrough(true);
		this->page = new_page;
		return;
	}

	if (new_page == this->page) {
		return;
	}

	ClearWaitingForegroundExecution();
	SetPaused(false);
	const auto* old_page = page;
	page = new_page;

	SetSpriteGraphic(ToString(page->character_name), page->character_index);

	if (IsStopping()
			&& (!old_page
				|| old_page->character_direction != new_page->character_direction
				|| old_page->character_pattern != new_page->character_pattern))
	{
		SetFacing(page->character_direction);
		SetDirection(page->character_direction);
	}

	// This fixes a load game bug in RPG_RT where if you save and load while an event
	// has an active move route, it's frequency gets set to zero after the move route ends.
	original_move_frequency = page->move_frequency;

	SetTransparency(page->translucent ? 3 : 0);
	SetMoveFrequency(page->move_frequency);
	SetLayer(page->layer);
	data()->overlap_forbidden = page->overlap_forbidden;
	SetAnimationType(static_cast<lcf::rpg::EventPage::AnimType>(page->animation_type));
	SetMoveSpeed(page->move_speed);

	if (IsFacingLocked()) {
		SetFacing(page->character_direction);
	}

	if (GetAnimationType() == lcf::rpg::EventPage::AnimType_fixed_graphic
			|| GetAnimationType() == lcf::rpg::EventPage::AnimType_spin) {
		SetAnimFrame(page->character_pattern);
	}

	if (page->move_type == lcf::rpg::EventPage::MoveType_random) {
		SetMaxStopCountForRandom();
	} else if (page->move_type == lcf::rpg::EventPage::MoveType_custom) {
		SetMaxStopCountForTurn();
	} else {
		SetMaxStopCountForStep();
	}

	// When the page is being changed, RPG_RT will not reset the original move route index
	// if the move routes of the old page and the new page have the same size and all their command codes are the same.
	// To clarify, other move route command parameters such as switch_id etc... are not considered in the comparison!
	if (!old_page || !CompareMoveRouteCommandCodes(old_page->move_route, new_page->move_route)) {
		SetOriginalMoveRouteIndex(0);
	}

	ResetThrough();

	if (GetTrigger() == lcf::rpg::EventPage::Trigger_parallel) {
		if (!page->event_commands.empty()) {
			if (!interpreter) {
				interpreter.reset(new Game_Interpreter_Map());
			}
			// RPG_RT will wait until the next call to Update() to push the interpreter code.
			// This forces the interpreter to yield when it changes it's own page.
		}
	}

	if (interpreter) {
		interpreter->Clear();
	}
}

bool Game_Event::AreConditionsMet(const lcf::rpg::EventPage& page) {
	// First switch (A)
	if (page.condition.flags.switch_a && !Main_Data::game_switches->Get(page.condition.switch_a_id)) {
		return false;
	}

	// Second switch (B)
	if (page.condition.flags.switch_b && !Main_Data::game_switches->Get(page.condition.switch_b_id)) {
		return false;
	}

	// Variable
	if (Player::IsRPG2k()) {
		if (page.condition.flags.variable && !(Main_Data::game_variables->Get(page.condition.variable_id) >= page.condition.variable_value)) {
			return false;
		}
	} else {
		if (page.condition.flags.variable) {
			switch (page.condition.compare_operator) {
			case 0: // ==
				if (!(Main_Data::game_variables->Get(page.condition.variable_id) == page.condition.variable_value))
					return false;
				break;
			case 1: // >=
				if (!(Main_Data::game_variables->Get(page.condition.variable_id) >= page.condition.variable_value))
					return false;
				break;
			case 2: // <=
				if (!(Main_Data::game_variables->Get(page.condition.variable_id) <= page.condition.variable_value))
					return false;
				break;
			case 3: // >
				if (!(Main_Data::game_variables->Get(page.condition.variable_id) > page.condition.variable_value))
					return false;
				break;
			case 4: // <
				if (!(Main_Data::game_variables->Get(page.condition.variable_id) < page.condition.variable_value))
					return false;
				break;
			case 5: // !=
				if (!(Main_Data::game_variables->Get(page.condition.variable_id) != page.condition.variable_value))
					return false;
				break;
			}
		}
	}

	// Item in possession?
	if (page.condition.flags.item && !(Main_Data::game_party->GetItemCount(page.condition.item_id)
		+ Main_Data::game_party->GetEquippedItemCount(page.condition.item_id))) {
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
	if (page.condition.flags.timer2 && Player::IsRPG2k3Commands()) {
		int secs = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer2);
		if (secs > page.condition.timer2_sec)
			return false;
	}

	// All conditions met :D
	return true;
}

int Game_Event::GetId() const {
	return data()->ID;
}

StringView Game_Event::GetName() const {
	return event->name;
}

bool Game_Event::IsWaitingForegroundExecution() const {
	return data()->waiting_execution;
}

bool Game_Event::WasStartedByDecisionKey() const {
	return data()->triggered_by_decision_key;
}

lcf::rpg::EventPage::Trigger Game_Event::GetTrigger() const {
	int trigger = page ? page->trigger : -1;
	return static_cast<lcf::rpg::EventPage::Trigger>(trigger);
}


bool Game_Event::ScheduleForegroundExecution(bool by_decision_key, bool face_player) {
	// RPG_RT always resets this everytime this function is called, whether successful or not
	data()->triggered_by_decision_key = by_decision_key;

	auto& list = GetList();
	if (!IsActive() || IsWaitingForegroundExecution() || list.empty()) {
		return false;
	}

	if (face_player && !(IsFacingLocked() || IsSpinning())) {
		SetFacing(GetDirectionToHero());
	}

	data()->waiting_execution = true;
	SetPaused(true);

	return true;
}

static std::vector<lcf::rpg::EventCommand> _empty_list = {};

const std::vector<lcf::rpg::EventCommand>& Game_Event::GetList() const {
	return page ? page->event_commands : _empty_list;
}

void Game_Event::OnFinishForegroundEvent() {
	UpdateFacing();
	SetPaused(false);
}

bool Game_Event::CheckEventAutostart() {
	if (GetTrigger() == lcf::rpg::EventPage::Trigger_auto_start) {
		ScheduleForegroundExecution(false, false);
		return true;
	}
	return false;
}

bool Game_Event::CheckEventCollision() {
	if (GetTrigger() == lcf::rpg::EventPage::Trigger_collision
			&& GetLayer() != lcf::rpg::EventPage::Layers_same
			&& !Main_Data::game_player->IsMoveRouteOverwritten()
			&& !Game_Map::GetInterpreter().IsRunning()
			&& Main_Data::game_player->GetX() == GetX()
			&& Main_Data::game_player->GetY() == GetY())
	{
		ScheduleForegroundExecution(false, true);
		SetStopCount(0);
		return true;
	}
	return false;
}

void Game_Event::CheckCollisonOnMoveFailure() {
	if (Game_Map::GetInterpreter().IsRunning()) {
		return;
	}

	const auto front_x = Game_Map::XwithDirection(GetX(), GetDirection());
	const auto front_y = Game_Map::YwithDirection(GetY(), GetDirection());

	if (Main_Data::game_player->GetX() == front_x
			&& Main_Data::game_player->GetY() == front_y
			&& GetLayer() == lcf::rpg::EventPage::Layers_same
			&& GetTrigger() == lcf::rpg::EventPage::Trigger_collision)
	{
		ScheduleForegroundExecution(false, true);
		// Events with trigger collision and layer same always reset their
		// stop_count when they fail movement to a tile that the player inhabits.
		SetStopCount(0);
	}
}

bool Game_Event::Move(int dir) {
	Game_Character::Move(dir);
	if (IsStopping()) {
		CheckCollisonOnMoveFailure();
		return false;
	}
	return true;
}

void Game_Event::UpdateNextMovementAction() {
	if (!page) {
		return;
	}

	UpdateMoveRoute(data()->move_route_index, data()->move_route, true);

	CheckEventAutostart();

	if (!IsStopping()) {
		return;
	}

	CheckEventCollision();

	if (!page
			|| IsPaused()
			|| IsMoveRouteOverwritten()
			|| IsStopCountActive()
			|| (!Main_Data::game_system->GetMessageContinueEvents() && Game_Map::GetInterpreter().IsRunning()))
	{
		return;
	}

	switch (page->move_type) {
	case lcf::rpg::EventPage::MoveType_stationary:
		break;
	case lcf::rpg::EventPage::MoveType_random:
		MoveTypeRandom();
		break;
	case lcf::rpg::EventPage::MoveType_vertical:
		MoveTypeCycleUpDown();
		break;
	case lcf::rpg::EventPage::MoveType_horizontal:
		MoveTypeCycleLeftRight();
		break;
	case lcf::rpg::EventPage::MoveType_toward:
		MoveTypeTowardsPlayer();
		break;
	case lcf::rpg::EventPage::MoveType_away:
		MoveTypeAwayFromPlayer();
		break;
	case lcf::rpg::EventPage::MoveType_custom:
		UpdateMoveRoute(data()->original_move_route_index, page->move_route, false);
		break;
	}
}

void Game_Event::SetMaxStopCountForRandom() {
	auto st = GetMaxStopCountForStep(GetMoveFrequency());
	st *= Rand::GetRandomNumber(0, 3) + 3;
	st /= 5;
	SetMaxStopCount(st);
}

void Game_Event::MoveTypeRandom() {
	int draw = Rand::GetRandomNumber(0, 9);

	const auto prev_dir = GetDirection();

	if (draw < 3) {
	} else if (draw < 5) {
		Turn90DegreeLeft();
	} else if (draw < 7) {
		Turn90DegreeRight();
	} else if (draw < 8) {
		Turn180Degree();
	} else {
		SetStopCount(Rand::GetRandomNumber(0, GetMaxStopCount()));
		return;
	}

	Move(GetDirection());

	if (IsStopping()) {
		if (IsWaitingForegroundExecution() || (GetStopCount() >= GetMaxStopCount() + 60)) {
			SetStopCount(0);
		} else {
			SetDirection(prev_dir);
			if (!IsFacingLocked()) {
				SetFacing(prev_dir);
			}
		}
	}

	SetMaxStopCountForRandom();
}

void Game_Event::MoveTypeCycle(int default_dir) {
	if (GetStopCount() < GetMaxStopCount()) return;

	const auto prev_dir = GetDirection();

	const auto reverse_dir = ReverseDir(default_dir);
	int move_dir = GetDirection();
	if (move_dir != reverse_dir) {
		move_dir = default_dir;
	}

	Move(move_dir);

	if (IsStopping() && GetStopCount() >= GetMaxStopCount() + 20) {
		Move(ReverseDir(move_dir));
	}

	if (IsStopping()) {
		if (IsWaitingForegroundExecution() || (GetStopCount() >= GetMaxStopCount() + 60)) {
			SetStopCount(0);
		} else {
			SetDirection(prev_dir);
			if (!IsFacingLocked()) {
				SetFacing(prev_dir);
			}
		}
	}

	SetMaxStopCountForStep();
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

	const bool in_sight = (sx >= -offset && sx <= Player::screen_width + offset
			&& sy >= -offset && sy <= Player::screen_height + offset);

	const auto prev_dir = GetDirection();

	int dir = 0;
	if (!in_sight) {
		dir = Rand::GetRandomNumber(0, 3);
	} else {
		int draw = Rand::GetRandomNumber(0, 9);
		if (draw == 0) {
			dir = GetDirection();
		} else if(draw == 1) {
			dir = Rand::GetRandomNumber(0, 3);
		} else {
			dir = towards
				? GetDirectionToHero()
				: GetDirectionAwayHero();
		}
	}

	Move(dir);

	if (IsStopping()) {
		if (IsWaitingForegroundExecution() || (GetStopCount() >= GetMaxStopCount() + 60)) {
			SetStopCount(0);
		} else {
			SetDirection(prev_dir);
			if (!IsFacingLocked()) {
				SetFacing(prev_dir);
			}
		}
	}

	SetMaxStopCountForStep();
}

void Game_Event::MoveTypeTowardsPlayer() {
	MoveTypeTowardsOrAwayPlayer(true);
}

void Game_Event::MoveTypeAwayFromPlayer() {
	MoveTypeTowardsOrAwayPlayer(false);
}

AsyncOp Game_Event::Update(bool resume_async) {
	if (!data()->active || (!resume_async && page == NULL)) {
		return {};
	}

	// RPG_RT runs the parallel interpreter everytime Update is called.
	// That means if the event updates multiple times due to makeway,
	// the interpreter will run multiple times per frame.
	// This results in event waits to finish quicker during collisions as
	// the wait will tick by 1 each time the interpreter is invoked.
	if ((resume_async || GetTrigger() == lcf::rpg::EventPage::Trigger_parallel) && interpreter) {
		if (!interpreter->IsRunning() && page && !page->event_commands.empty()) {
			interpreter->Push(this);
		}
		interpreter->Update(!resume_async);

		// Suspend due to async op ...
		if (interpreter->IsAsyncPending()) {
			return interpreter->GetAsyncOp();
		}

		// RPG_RT only exits if active is false here, but not if there is
		// no active page...
		if (!data()->active) {
			return {};
		}
	}

	Game_Character::Update();

	return {};
}

const lcf::rpg::EventPage* Game_Event::GetPage(int page) const {
	if (page <= 0 || page - 1 >= static_cast<int>(event->pages.size())) {
		return nullptr;
	}
	return &event->pages[page - 1];
}

const lcf::rpg::EventPage *Game_Event::GetActivePage() const {
	return page;
}

