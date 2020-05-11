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
#include "game_player.h"
#include "async_handler.h"
#include "game_actor.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_system.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "input.h"
#include "main_data.h"
#include "player.h"
#include "util_macro.h"
#include "game_switches.h"
#include "output.h"
#include <lcf/reader_util.h>
#include <lcf/scope_guard.h>
#include "scene_battle.h"
#include "scene_menu.h"
#include <algorithm>
#include <cmath>

Game_Player::Game_Player(): Game_PlayerBase(Player)
{
	SetDirection(lcf::rpg::EventPage::Direction_down);
	SetMoveSpeed(4);
	SetAnimationType(lcf::rpg::EventPage::AnimType_non_continuous);
}

void Game_Player::SetSaveData(lcf::rpg::SavePartyLocation save)
{
	*data() = std::move(save);
}

lcf::rpg::SavePartyLocation Game_Player::GetSaveData() const {
	return *data();
}

int Game_Player::GetScreenZ(bool apply_shift) const {
	// Player is always slightly above events
	// (and always on "same layer as hero" obviously)
	return Game_Character::GetScreenZ(apply_shift) + 1;
}

bool Game_Player::GetVisible() const {
	return visible && !data()->aboard;
}

void Game_Player::ReserveTeleport(int map_id, int x, int y, int direction, TeleportTarget::Type tt) {
	teleport_target = TeleportTarget(map_id, x, y, direction, tt);

	FileRequestAsync* request = Game_Map::RequestMap(map_id);
	request->SetImportantFile(true);
	request->Start();
}

void Game_Player::ReserveTeleport(const lcf::rpg::SaveTarget& target) {
	int map_id = target.map_id;

	if (Game_Map::GetMapType(target.map_id) == lcf::rpg::TreeMap::MapType_area) {
		// Area: Obtain the map the area belongs to
		map_id = Game_Map::GetParentId(target.map_id);
	}

	ReserveTeleport(map_id, target.map_x, target.map_y, Down, TeleportTarget::eSkillTeleport);

	if (target.switch_on) {
		Main_Data::game_switches->Set(target.switch_id, true);
		Game_Map::SetNeedRefresh(true);
	}
}

void Game_Player::PerformTeleport() {
	assert(IsPendingTeleport());
	if (!IsPendingTeleport()) {
		return;
	}

	if (teleport_target.GetMapId() <= 0) {
		Output::Error("Invalid Teleport map id! mapid={} x={} y={} d={}", teleport_target.GetMapId(),
				teleport_target.GetX(), teleport_target.GetY(), teleport_target.GetDirection());
	}

	const auto map_changed = (GetMapId() != teleport_target.GetMapId());
	MoveTo(teleport_target.GetMapId(), teleport_target.GetX(), teleport_target.GetY());


	// FIXME: Direction lock on hero?
	if (teleport_target.GetDirection() >= 0) {
		SetDirection(teleport_target.GetDirection());
		if (!IsFacingLocked()) {
			SetSpriteDirection(teleport_target.GetDirection());
		}
	}

	if (teleport_target.GetType() != TeleportTarget::eAsyncQuickTeleport) {
		Main_Data::game_screen->OnMapChange();
		Main_Data::game_pictures->OnMapChange();
		Game_Map::GetInterpreter().OnMapChange();
	}

	ResetTeleportTarget();
}

void Game_Player::MoveTo(int map_id, int x, int y) {
	const auto map_changed = (GetMapId() != map_id);

	Game_Character::MoveTo(map_id, x, y);
	SetEncounterSteps(0);
	SetMenuCalling(false);

	auto* vehicle = GetVehicle();
	if (vehicle) {
		vehicle->MoveTo(map_id, x, y);
	}

	if (map_changed) {
		// FIXME: Assert map pre-loaded in cache.

		// pan_state does not reset when you change maps.
		data()->pan_speed = lcf::rpg::SavePartyLocation::kPanSpeedDefault;
		data()->pan_finish_x = lcf::rpg::SavePartyLocation::kPanXDefault;
		data()->pan_finish_y = lcf::rpg::SavePartyLocation::kPanYDefault;
		data()->pan_current_x = lcf::rpg::SavePartyLocation::kPanXDefault;
		data()->pan_current_y = lcf::rpg::SavePartyLocation::kPanYDefault;

		Game_Map::Setup();
		Game_Map::PlayBgm();
	} else {
		Game_Map::SetPositionX(GetSpriteX() - GetPanX());
		Game_Map::SetPositionY(GetSpriteY() - GetPanY());
	}

	ResetAnimation();

	Refresh();
}

bool Game_Player::MakeWay(int from_x, int from_y, int to_x, int to_y) {
	if (data()->aboard) {
		return GetVehicle()->MakeWay(from_x, from_y, to_x, to_y);
	}

	return Game_Character::MakeWay(from_x, from_y, to_x, to_y);
}


void Game_Player::UpdateScroll(int old_x, int old_y) {
	if (IsPanLocked()) {
		return;
	}

	int screen_x = Game_Map::GetPositionX();
	int screen_y = Game_Map::GetPositionY();

	int old_panx = old_x - screen_x;
	int old_pany = old_y - screen_y;

	int new_x = GetSpriteX();
	int new_y = GetSpriteY();

	int dx = new_x - old_x;
	int dy = new_y - old_y;

	int new_panx = new_x - screen_x;
	int new_pany = new_y - screen_y;

	// Detect whether we crossed map boundary.
	// We need to scale down dx/dy to a single step
	// to not message up further calculations.
	// FIXME: This logic will break if something moves so fast
	// as to cross half the map in 1 frame.
	if (Game_Map::LoopHorizontal()) {
		auto w = Game_Map::GetWidth() * SCREEN_TILE_SIZE;
		if (std::abs(dx) > w / 2) {
			dx = (w - std::abs(dx)) % w;
			if (new_x > old_x) {
				dx = -dx;
			}
		}
	}
	if (Game_Map::LoopVertical()) {
		auto h = Game_Map::GetHeight() * SCREEN_TILE_SIZE;
		if (std::abs(dy) > h / 2) {
			dy = (h - std::abs(dy)) % h;
			if (new_y > old_y) {
				dy = -dy;
			}
		}
	}

	int scroll_dx = 0;
	int scroll_dy = 0;
	if (Game_Map::LoopHorizontal() ||
			std::abs(data()->pan_current_x - new_panx) >=
			std::abs(data()->pan_current_x - old_panx)) {
		scroll_dx = dx;
	}

	if (Game_Map::LoopVertical() ||
			std::abs(data()->pan_current_y - new_pany) >=
			std::abs(data()->pan_current_y - old_pany)) {
		scroll_dy = dy;
	}

	if (scroll_dx || scroll_dy) {
		Game_Map::Scroll(scroll_dx, scroll_dy);
	}
}

void Game_Player::UpdateVehicleActions() {
	if (IsAboard()) {
		auto* vehicle = GetVehicle();
		if (vehicle) {
			vehicle->SyncWithPlayer();
			if (IsStopping()) {
				vehicle->AnimateAscentDescent();
			}
		}
	}

	if (IsStopping()) {
		if (data()->boarding) {
			// Boarding completed
			data()->aboard = true;
			data()->boarding = false;
			auto* vehicle = GetVehicle();
			if (vehicle->IsMoveRouteOverwritten()) {
				vehicle->CancelMoveRoute();
			}
			SetMoveSpeed(vehicle->GetMoveSpeed());
			vehicle->SetDirection(GetDirection());
			vehicle->SetSpriteDirection(Left);
			// Note: RPG_RT ignores the lock_facing flag here!
			SetSpriteDirection(Left);
			vehicle->SetX(GetX());
			vehicle->SetY(GetY());
		}

		if (data()->unboarding) {
			// Unboarding completed
			data()->unboarding = false;
		}
	}
}

void Game_Player::UpdateSelfMovement() {
	bool did_call_encounter = false;
	bool did_call_menu = false;

	auto* vehicle = GetVehicle();

	bool is_boarding = IsBoardingOrUnboarding()
		|| (vehicle && vehicle->IsAscendingOrDescending());

	if (!is_boarding
			&& IsStopping()
			&& !IsMoveRouteOverwritten()) {

		if (IsEncounterCalling()) {
			BattleArgs args;
			if (Game_Map::PrepareEncounter(args)) {
				Scene::instance->SetRequestedScene(Scene_Battle::Create(std::move(args)));
			}
			SetEncounterCalling(false);
			did_call_encounter = true;
		}

		if (IsMenuCalling()) {
			if (!did_call_encounter) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				Scene::instance->SetRequestedScene(std::make_shared<Scene_Menu>());
				did_call_menu = true;
			}
			SetMenuCalling(false);
		}
	}

	if (!is_boarding
			&& !Game_Map::GetInterpreter().IsRunning()
			&& !Game_Message::IsMessageActive()
			&& !IsMoveRouteOverwritten()
			&& !IsPaused() // RPG_RT compatible logic, but impossible to set pause on player
			&& !did_call_encounter
			&& !did_call_menu
			&& !Game_Map::IsAnyEventStarting())
	{
		if (IsStopping()) {
			const bool force_through = (Player::debug_flag
					&& Input::IsPressed(Input::DEBUG_THROUGH)
					&& !GetThrough());
			if (force_through) {
				SetThrough(true);
			}
			auto sg = lcf::makeScopeGuard([&](){
					if (force_through) { SetThrough(false); }
					});

			bool tried_move = false;
			switch (Input::dir4) {
				case 2:
					tried_move = true;
					Move(Down);
					break;
				case 4:
					tried_move = true;
					Move(Left);
					break;
				case 6:
					tried_move = true;
					Move(Right);
					break;
				case 8:
					tried_move = true;
					Move(Up);
					break;
			}
			if (tried_move && IsStopping()) {
				Main_Data::game_party->IncSteps();
				if (UpdateEncounterSteps()) {
					SetEncounterCalling(true);
				}
				if (Main_Data::game_party->ApplyStateDamage()) {
					Main_Data::game_screen->FlashMapStepDamage();
				}
			}
		}

		if (IsStopping()) {
			if (Input::IsTriggered(Input::DECISION)) {
				if (!GetOnOffVehicle()) {
					CheckActionEvent();
				}
			}
		}
	}

	// ESC-Menu calling
	if (Game_System::GetAllowMenu()
			&& !Game_Message::IsMessageActive()
			&& !Game_Map::GetInterpreter().IsRunning())
	{
		if (Input::IsTriggered(Input::CANCEL)) {
			SetMenuCalling(true);
		}
	}
}

void Game_Player::Update() {
	if (IsProcessed()) {
		return;
	}
	SetProcessed(true);

	const auto old_sprite_x = GetSpriteX();
	const auto old_sprite_y = GetSpriteY();

	auto was_moving = !IsStopping();
	auto was_move_route_overriden = IsMoveRouteOverwritten();

	Game_Character::UpdateMovement();
	Game_Character::UpdateAnimation(was_moving);
	Game_Character::UpdateFlash();

	UpdateScroll(old_sprite_x, old_sprite_y);
	UpdatePan();

	UpdateVehicleActions();

	if (!InAirship()
			&& IsStopping()
			&& !IsMoveRouteOverwritten()
			) {
		TriggerSet triggers;

		if (!Game_Map::GetInterpreter().IsRunning()) {
			triggers[lcf::rpg::EventPage::Trigger_collision] = true;
		}

		// When the last command of a move route is a move command, there is special
		// logic to reset the move route index to 0. We leverage this here because
		// we only do touch checks if the last move command was a move.
		// Checking was_moving is not enough, because there could have been 0 frame
		// commands after the move in the move route, in which case index would be > 0.
		if (was_moving && (!was_move_route_overriden || GetMoveRouteIndex() == 0)) {
			triggers[lcf::rpg::EventPage::Trigger_touched] = true;
			triggers[lcf::rpg::EventPage::Trigger_collision] = true;
		}

		if (triggers.count() > 0 && CheckEventTriggerHere(triggers, true, false)) {
			return;
		}
	}
}

bool Game_Player::CheckActionEvent() {
	if (IsFlying()) {
		return false;
	}

	bool result = CheckEventTriggerHere({lcf::rpg::EventPage::Trigger_action}, true, true);

	int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
	int front_y = Game_Map::YwithDirection(GetY(), GetDirection());

	result |= CheckEventTriggerThere({lcf::rpg::EventPage::Trigger_touched, lcf::rpg::EventPage::Trigger_collision}, front_x, front_y, true, true);

	// Counter tile loop stops only if you talk to an action event.
	bool got_action = CheckEventTriggerThere({lcf::rpg::EventPage::Trigger_action}, front_x, front_y, true, true);
	// RPG_RT allows maximum of 3 counter tiles
	for (int i = 0; !got_action && i < 3; ++i) {
		if (!Game_Map::IsCounter(front_x, front_y)) {
			break;
		}

		front_x = Game_Map::XwithDirection(front_x, GetDirection());
		front_y = Game_Map::YwithDirection(front_y, GetDirection());

		got_action |= CheckEventTriggerThere({lcf::rpg::EventPage::Trigger_action}, front_x, front_y, true, true);
	}
	return result || got_action;
}

bool Game_Player::CheckEventTriggerHere(TriggerSet triggers, bool face_hero, bool triggered_by_decision_key) {
	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, GetX(), GetY());

	for (auto* ev: events) {
		const auto trigger = ev->GetTrigger();
		if (ev->GetLayer() != lcf::rpg::EventPage::Layers_same
				&& trigger >= 0
				&& triggers[trigger]) {
			result |= ev->SetAsWaitingForegroundExecution(face_hero, triggered_by_decision_key);
		}
	}
	return result;
}

bool Game_Player::CheckEventTriggerThere(TriggerSet triggers, int x, int y, bool face_hero, bool triggered_by_decision_key) {
	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);

	for (const auto& ev : events) {
		const auto trigger = ev->GetTrigger();
		if ( ev->GetLayer() == lcf::rpg::EventPage::Layers_same
				&& trigger >= 0
				&& triggers[trigger]) {
			result |= ev->SetAsWaitingForegroundExecution(face_hero, triggered_by_decision_key);
		}
	}
	return result;
}

void Game_Player::Refresh() {

	auto* actor = Main_Data::game_party->GetActor(0);
	if (actor == nullptr) {
		SetSpriteGraphic("", 0);
		SetTransparency(0);
		return;
	}

	SetSpriteGraphic(ToString(actor->GetSpriteName()), actor->GetSpriteIndex());
	SetTransparency(actor->GetSpriteTransparency());

	if (data()->aboard)
		GetVehicle()->SyncWithPlayer();
}

bool Game_Player::GetOnOffVehicle() {
	if (IsBoardingOrUnboarding()) {
		return false;
	}

	if (InVehicle())
		return GetOffVehicle();
	return GetOnVehicle();
}

bool Game_Player::GetOnVehicle() {
	int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
	int front_y = Game_Map::YwithDirection(GetY(), GetDirection());
	Game_Vehicle::Type type;

	if (Game_Map::GetVehicle(Game_Vehicle::Airship)->IsInPosition(GetX(), GetY()))
		type = Game_Vehicle::Airship;
	else if (Game_Map::GetVehicle(Game_Vehicle::Ship)->IsInPosition(front_x, front_y))
		type = Game_Vehicle::Ship;
	else if (Game_Map::GetVehicle(Game_Vehicle::Boat)->IsInPosition(front_x, front_y))
		type = Game_Vehicle::Boat;
	else
		return false;

	auto* vehicle = Game_Map::GetVehicle(type);

	if (vehicle->IsAscendingOrDescending()) {
		return false;
	}

	if (type == Game_Vehicle::Airship && (IsMoving() || IsJumping())) {
		return false;
	}

	if (type != Game_Vehicle::Airship && !Game_Map::CanEmbarkShip(*this, front_x, front_y)) {
		return false;
	}

	data()->vehicle = type;
	data()->preboard_move_speed = GetMoveSpeed();
	if (type != Game_Vehicle::Airship) {
		data()->boarding = true;
		if (!IsMoving() && !IsJumping()) {
			if (!GetThrough()) {
				SetThrough(true);
				Move(GetDirection());
				SetThrough(false);
			} else {
				Move(GetDirection());
			}
		}
	} else {
		data()->aboard = true;
		if (vehicle->IsMoveRouteOverwritten()) {
			vehicle->CancelMoveRoute();
		}
		SetMoveSpeed(vehicle->GetMoveSpeed());
		SetDirection(lcf::rpg::EventPage::Direction_left);
		// Note: RPG_RT ignores the lock_facing flag here!
		SetSpriteDirection(lcf::rpg::EventPage::Direction_left);
		vehicle->SetX(GetX());
		vehicle->SetY(GetY());
	}

	Game_System::SetBeforeVehicleMusic(Game_System::GetCurrentBGM());
	GetVehicle()->GetOn();
	return true;
}

bool Game_Player::GetOffVehicle() {
	if (!InAirship()) {
		int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
		int front_y = Game_Map::YwithDirection(GetY(), GetDirection());
		if (!Game_Map::CanDisembarkShip(*this, front_x, front_y)) {
			return false;
		}
	}
	auto* vehicle = GetVehicle();

	if (vehicle->IsAscendingOrDescending()) {
		return false;
	}

	vehicle->GetOff();
	return true;
}

void Game_Player::UpdateMoveRoute(int32_t& current_index, const RPG::MoveRoute& current_route, bool is_overwrite) {
	auto* vehicle = GetVehicle();
	if (vehicle && vehicle->IsAscendingOrDescending()) {
		return;
	}
	Game_Character::UpdateMoveRoute(current_index, current_route, is_overwrite);
}

bool Game_Player::InVehicle() const {
	return data()->vehicle > 0;
}

bool Game_Player::InAirship() const {
	return data()->vehicle == Game_Vehicle::Airship;
}

Game_Vehicle* Game_Player::GetVehicle() const {
	return Game_Map::GetVehicle((Game_Vehicle::Type) data()->vehicle);
}

void Game_Player::Move(int dir) {
	if (!IsStopping()) {
		return;
	}

	Game_Character::Move(dir);

	if (IsStopping()) {
		// FIXME: Does this really happen? Not seen in code. Does this belong in makeway?
		if (!IsMoveRouteOverwritten()) {
			int nx = GetX() + GetDxFromDirection(dir);
			int ny = GetY() + GetDyFromDirection(dir);
			CheckEventTriggerThere({RPG::EventPage::Trigger_touched, RPG::EventPage::Trigger_collision}, nx, ny, true, false);
		}
		return;
	}

	if (InAirship()) {
		return;
	}

	int terrain_id = Game_Map::GetTerrainTag(GetX(), GetY());
	const lcf::rpg::Terrain* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, terrain_id);
	bool red_flash = false;

	if (terrain) {
		if (!terrain->on_damage_se || (terrain->on_damage_se && (terrain->damage > 0))) {
			Game_System::SePlay(terrain->footstep);
		}
		if (terrain->damage > 0) {
			for (auto hero : Main_Data::game_party->GetActors()) {
				if (!hero->PreventsTerrainDamage()) {
					red_flash = true;
					hero->ChangeHp(-std::max<int>(0, std::min<int>(terrain->damage, hero->GetHp() - 1)));
				}
			}
		}
	} else {
		Output::Warning("Player BeginMove: Invalid terrain ID {} at ({}, {})", terrain_id, GetX(), GetY());
	}

	if (red_flash) {
		Main_Data::game_screen->FlashMapStepDamage();
	}
}

void Game_Player::Unboard() {
	data()->aboard = false;
	SetMoveSpeed(data()->preboard_move_speed);

	Game_System::BgmPlay(Game_System::GetBeforeVehicleMusic());
}

bool Game_Player::IsAboard() const {
	return data()->aboard;
}

bool Game_Player::IsBoardingOrUnboarding() const {
	return data()->boarding || data()->unboarding;
}

void Game_Player::UnboardingFinished() {
	Unboard();
	if (InAirship()) {
		SetDirection(lcf::rpg::EventPage::Direction_down);
		// Note: RPG_RT ignores the lock_facing flag here!
		SetSpriteDirection(lcf::rpg::EventPage::Direction_down);
	} else {
		data()->unboarding = true;
		if (!IsMoving() && !IsJumping()) {
			if (!GetThrough()) {
				SetThrough(true);
				Move(GetDirection());
				SetThrough(false);
			} else {
				Move(GetDirection());
			}
		}
	}
	data()->vehicle = Game_Vehicle::None;
}

int Game_Player::GetVehicleType() const {
	return data()->vehicle;
}

bool Game_Player::UpdateEncounterSteps() {
	if (Player::debug_flag &&
		Input::IsPressed(Input::DEBUG_THROUGH)) {
			return false;
	}

	if(Main_Data::game_player->InAirship()) {
		return false;
	}

	const auto encounter_rate = Game_Map::GetEncounterRate();

	if (encounter_rate <= 0) {
		SetEncounterSteps(0);
		return false;
	}

	int x = GetX();
	int y = GetY();

	const auto* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, Game_Map::GetTerrainTag(x,y));
	if (!terrain) {
		Output::Warning("UpdateEncounterSteps: Invalid terrain at ({}, {})", x, y);
		return false;
	}

	data()->encounter_steps += terrain->encounter_rate;

	struct Row {
		int ratio;
		float pmod;
	};

#if 1
	static constexpr Row enc_table[] = {
		{ 0, 0.0625},
		{ 20, 0.125 },
		{ 40, 0.25 },
		{ 60, 0.5 },
		{ 100, 2.0 },
		{ 140, 4.0 },
		{ 160, 8.0 },
		{ 180, 16.0 },
		{ INT_MAX, 16.0 }
	};
#else
	//Old versions of RM2k used this table.
	//Left here for posterity.
	static constexpr Row enc_table[] = {
		{ 0, 0.5 },
		{ 20, 2.0 / 3.0 },
		{ 50, 5.0 / 6.0 },
		{ 100, 6.0 / 5.0 },
		{ 200, 3.0 / 2.0 },
		{ INT_MAX, 3.0 / 2.0 }
	};
#endif
	const auto ratio = GetEncounterSteps() / encounter_rate;

	auto& idx = last_encounter_idx;
	while (ratio > enc_table[idx+1].ratio) {
		++idx;
	}
	const auto& row = enc_table[idx];

	const auto pmod = row.pmod;
	const auto p = (1.0f / float(encounter_rate)) * pmod * (float(terrain->encounter_rate) / 100.0f);

	if (Utils::PercentChance(p)) {
		SetEncounterSteps(0);
		return true;
	}

	return false;
}

void Game_Player::SetEncounterSteps(int steps) {
	last_encounter_idx = 0;
	data()->encounter_steps = steps;
}

void Game_Player::LockPan() {
	data()->pan_state = lcf::rpg::SavePartyLocation::PanState_fixed;
}

void Game_Player::UnlockPan() {
	data()->pan_state = lcf::rpg::SavePartyLocation::PanState_follow;
}

void Game_Player::StartPan(int direction, int distance, int speed) {
	distance *= SCREEN_TILE_SIZE;

	if (direction == PanUp) {
		int new_pan = data()->pan_finish_y + distance;
		data()->pan_finish_y = new_pan;
	} else if (direction == PanRight) {
		int new_pan = data()->pan_finish_x - distance;
		data()->pan_finish_x = new_pan;
	} else if (direction == PanDown) {
		int new_pan = data()->pan_finish_y - distance;
		data()->pan_finish_y = new_pan;
	} else if (direction == PanLeft) {
		int new_pan = data()->pan_finish_x + distance;
		data()->pan_finish_x = new_pan;
	}

	data()->pan_speed = 2 << speed;
}

void Game_Player::ResetPan(int speed) {
	data()->pan_finish_x = lcf::rpg::SavePartyLocation::kPanXDefault;
	data()->pan_finish_y = lcf::rpg::SavePartyLocation::kPanYDefault;
	data()->pan_speed = 2 << speed;
}

int Game_Player::GetPanWait() {
	const auto distance = std::max(
			std::abs(data()->pan_current_x - data()->pan_finish_x),
			std::abs(data()->pan_current_y - data()->pan_finish_y));
	const auto speed = data()->pan_speed;
	assert(speed > 0);
	return distance / speed + (distance % speed != 0);
}

void Game_Player::UpdatePan() {
	if (!IsPanActive())
		return;

	const int step = data()->pan_speed;
	const int pan_remain_x = data()->pan_current_x - data()->pan_finish_x;
	const int pan_remain_y = data()->pan_current_y - data()->pan_finish_y;

	int dx = std::min(step, std::abs(pan_remain_x));
	dx = pan_remain_x >= 0 ? dx : -dx;
	int dy = std::min(step, std::abs(pan_remain_y));
	dy = pan_remain_y >= 0 ? dy : -dy;

	int screen_x = Game_Map::GetPositionX();
	int screen_y = Game_Map::GetPositionY();

	Game_Map::AddScreenX(screen_x, dx);
	Game_Map::AddScreenY(screen_y, dy);

	// If we hit the edge of the map before pan finishes.
	if (dx == 0 && dy == 0) {
		return;
	}

	Game_Map::Scroll(dx, dy);

	data()->pan_current_x -= dx;
	data()->pan_current_y -= dy;
}

