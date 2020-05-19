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
#include "utils.h"
#include "reader_util.h"
#include "scope_guard.h"
#include "scene_battle.h"
#include "scene_menu.h"
#include <algorithm>
#include <cmath>

Game_Player::Game_Player(): Game_PlayerBase(Player)
{
	SetDirection(RPG::EventPage::Direction_down);
	SetMoveSpeed(4);
	SetAnimationType(RPG::EventPage::AnimType_non_continuous);
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

void Game_Player::ReserveTeleport(const RPG::SaveTarget& target) {
	int map_id = target.map_id;

	if (Game_Map::GetMapType(target.map_id) == RPG::TreeMap::MapType_area) {
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


	if (teleport_target.GetDirection() >= 0) {
		SetDirection(teleport_target.GetDirection());
		UpdateFacing();
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
		// RPG_RT doesn't check the aboard flag for this one
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

void Game_Player::UpdateScroll(int amount, bool was_jumping) {
	if (IsPanLocked()) {
		return;
	}

	auto dx = (GetX() * SCREEN_TILE_SIZE) - Game_Map::GetPositionX() - GetPanX();
	auto dy = (GetY() * SCREEN_TILE_SIZE) - Game_Map::GetPositionY() - GetPanY();

	const auto w = Game_Map::GetWidth() * SCREEN_TILE_SIZE;
	const auto h = Game_Map::GetHeight() * SCREEN_TILE_SIZE;

	dx = Utils::PositiveModulo(dx + w / 2, w) - w / 2;
	dy = Utils::PositiveModulo(dy + h / 2, h) - h / 2;

	const auto sx = Utils::Signum(dx);
	const auto sy = Utils::Signum(dy);

	if (was_jumping) {
		const auto jdx = sx * std::abs(GetX() - GetBeginJumpX());
		const auto jdy = sy * std::abs(GetY() - GetBeginJumpY());

		Game_Map::Scroll(amount * jdx, amount * jdy);

		if (!IsJumping()) {
			// FIXME: This is to fix rounding errors?
			// FIXME: use Utils::RoundTo<T>
			const auto x = SCREEN_TILE_SIZE * Utils::RoundTo<int>(Game_Map::GetPositionX() / static_cast<double>(SCREEN_TILE_SIZE));
			const auto y = SCREEN_TILE_SIZE * Utils::RoundTo<int>(Game_Map::GetPositionY() / static_cast<double>(SCREEN_TILE_SIZE));

			// FIXME: In Player this resets the panorama, not in RPG_RT
			Game_Map::SetPositionX(x);
			Game_Map::SetPositionY(y);
		}
		return;
	}

	Game_Map::Scroll(sx * amount, sy * amount);
}

bool Game_Player::UpdateAirship() {
	auto* vehicle = GetVehicle();

	// RPG_RT doesn't check vehicle, but we have to as we don't have another way to fetch it.
	// Also in vanilla RPG_RT it's impossible for the hero to fly without the airship.
	if (vehicle && IsFlying()) {
		if (vehicle->AnimateAscentDescent()) {
			data()->aboard = false;
			SetSpriteDirection(Down);
			data()->vehicle = 0;
			SetMoveSpeed(data()->preboard_move_speed);

			Game_System::SetBeforeVehicleMusic(Game_System::GetCurrentBGM());

			return true;
		}
	}
	return false;
}

void Game_Player::UpdateNextMovementAction() {
	auto* vehicle = GetVehicle();

	// RPG_RT doesn't check vehicle, but we have to as we don't have another way to fetch it.
	// Also in vanilla RPG_RT it's impossible for the hero to fly without the airship.
	if (vehicle && IsFlying()) {
		auto was_ascending_or_descending = vehicle->IsAscendingOrDescending();
		vehicle->AnimateAscentDescent();

		if (was_ascending_or_descending) {
			return;
		}
	}

	UpdateMoveRoute(data()->move_route_index, data()->move_route, true);

	if (Game_Map::GetInterpreter().IsRunning()) {
		SetMenuCalling(false);
		return;
	}
	// FIXME: Message Visible? Or active?
	if(IsPaused() || IsMoveRouteOverwritten() || Game_Message::IsMessageActive()) {
		return;
	}

	if (IsEncounterCalling()) {
		SetMenuCalling(false);

		BattleArgs args;
		if (Game_Map::PrepareEncounter(args)) {
			Scene::instance->SetRequestedScene(Scene_Battle::Create(std::move(args)));
		}
		SetEncounterCalling(false);
		// FIXME: collision checks still done this frame?
		return;
	}

	if (IsMenuCalling()) {
		SetMenuCalling(false);

		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Scene::instance->SetRequestedScene(std::make_shared<Scene_Menu>());
		return;
	}

	CheckEventTriggerHere({ RPG::EventPage::Trigger_collision }, false);

	if (Game_Map::IsAnyEventStarting()) {
		return;
	}

	int move_dir = -1;
	switch (Input::dir4) {
		case 2:
			move_dir = Down;
			break;
		case 4:
			move_dir = Left;
			break;
		case 6:
			move_dir = Right;
			break;
		case 8:
			move_dir = Up;
			break;
	}
	if (move_dir >= 0) {
		SetThrough((Player::debug_flag && Input::IsPressed(Input::DEBUG_THROUGH)) || data()->route_through);
		Move(move_dir);
		ResetThrough();
		if (IsStopping()) {
			int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
			int front_y = Game_Map::YwithDirection(GetY(), GetDirection());
			CheckEventTriggerThere({RPG::EventPage::Trigger_touched, RPG::EventPage::Trigger_collision}, front_x, front_y, false);
		}
	}

	if (IsStopping()) {
		if (Input::IsTriggered(Input::DECISION)) {
			if (!GetOnOffVehicle()) {
				CheckActionEvent();
			}
		}
	} else {
		Main_Data::game_party->IncSteps();
		if (Main_Data::game_party->ApplyStateDamage()) {
			Main_Data::game_screen->FlashMapStepDamage();
		}
		if (UpdateEncounterSteps()) {
			SetEncounterCalling(true);
		}
	}
}

void Game_Player::UpdateMovement(int amount) {
	const bool was_jumping = IsJumping();

	Game_Character::UpdateMovement(amount);

	UpdateScroll(amount, was_jumping);

	if (!IsMoveRouteOverwritten() && IsStopping()) {
		TriggerSet triggers = { RPG::EventPage::Trigger_touched, RPG::EventPage::Trigger_collision };
		CheckEventTriggerHere(triggers, false);
	}
}

void Game_Player::Update() {
	Game_Character::Update();

	if (IsStopping()) {
		if (data()->boarding) {
			// Boarding completed
			data()->aboard = true;
			data()->boarding = false;
			// Note: RPG_RT ignores the lock_facing flag here!
			SetSpriteDirection(Left);

			auto* vehicle = GetVehicle();
			SetMoveSpeed(vehicle->GetMoveSpeed());
		}
		if (data()->unboarding) {
			// Unboarding completed
			data()->unboarding = false;
		}
	}

	auto* vehicle = GetVehicle();

	if (IsAboard() && vehicle) {
		vehicle->SyncWithRider(this);
	}

	UpdatePan();

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

bool Game_Player::CheckActionEvent() {
	if (IsFlying()) {
		return false;
	}

	bool result = false;
	int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
	int front_y = Game_Map::YwithDirection(GetY(), GetDirection());

	result |= CheckEventTriggerThere({RPG::EventPage::Trigger_touched, RPG::EventPage::Trigger_collision}, front_x, front_y, true);
	result |= CheckEventTriggerHere({RPG::EventPage::Trigger_action}, true);

	// Counter tile loop stops only if you talk to an action event.
	bool got_action = CheckEventTriggerThere({RPG::EventPage::Trigger_action}, front_x, front_y, true);
	// RPG_RT allows maximum of 3 counter tiles
	for (int i = 0; !got_action && i < 3; ++i) {
		if (!Game_Map::IsCounter(front_x, front_y)) {
			break;
		}

		front_x = Game_Map::XwithDirection(front_x, GetDirection());
		front_y = Game_Map::YwithDirection(front_y, GetDirection());

		got_action |= CheckEventTriggerThere({RPG::EventPage::Trigger_action}, front_x, front_y, true);
	}
	return result || got_action;
}

bool Game_Player::CheckEventTriggerHere(TriggerSet triggers, bool triggered_by_decision_key) {
	if (InAirship()) {
		return false;
	}

	bool result = false;

	for (auto& ev: Game_Map::GetEvents()) {
		const auto trigger = ev.GetTrigger();
		if (ev.IsActive()
				&& ev.GetX() == GetX()
				&& ev.GetY() == GetY()
				&& ev.GetLayer() != RPG::EventPage::Layers_same
				&& trigger >= 0
				&& triggers[trigger]) {
			SetEncounterCalling(false);
			SetMenuCalling(false);
			result |= ev.ScheduleForegroundExecution(triggered_by_decision_key);
		}
	}
	return result;
}

bool Game_Player::CheckEventTriggerThere(TriggerSet triggers, int x, int y, bool triggered_by_decision_key) {
	if (InAirship()) {
		return false;
	}
	bool result = false;

	for (auto& ev : Game_Map::GetEvents()) {
		const auto trigger = ev.GetTrigger();
		if (ev.IsActive()
				&& ev.GetX() == x
				&& ev.GetY() == y
				&& ev.GetLayer() == RPG::EventPage::Layers_same
				&& trigger >= 0
				&& triggers[trigger]) {
			SetEncounterCalling(false);
			SetMenuCalling(false);
			result |= ev.ScheduleForegroundExecution(triggered_by_decision_key);
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
}

bool Game_Player::GetOnOffVehicle() {
	// FIXME: Make a test case for this
	if (GetDirection() >= UpRight) {
		SetDirection(GetSpriteDirection());
	}

	const auto x = Game_Map::RoundX(GetX());
	const auto y = Game_Map::RoundY(GetY());

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

	const auto* airship = Game_Map::GetVehicle(Game_Vehicle::Airship);

	if (airship->IsInPosition(GetX(), GetY()) && IsStopping() && airship->IsStopping()) {
		if (airship->IsAscendingOrDescending()) {
			return false;
		}

		data()->vehicle = Game_Vehicle::Airship;
		data()->preboard_move_speed = GetMoveSpeed();
		data()->aboard = true;
		SetMoveSpeed(airship->GetMoveSpeed());
		// Note: RPG_RT ignores the lock_facing flag here!
		SetSpriteDirection(Left);
	} else {
		auto type = Game_Vehicle::Ship;
		if (!Game_Map::GetVehicle(type)->IsInPosition(front_x, front_y)) {
			type = Game_Vehicle::Boat;
			if (!Game_Map::GetVehicle(type)->IsInPosition(front_x, front_y)) {
				return false;
			}
		}

		if (!Game_Map::CanEmbarkShip(*this, front_x, front_y)) {
			return false;
		}

		// FIXME: RPG_RT still executes move event if triggered mid-move?
		SetThrough(true);
		Move(GetDirection());
		// FIXME: RPG_RT resets through to route_through || not visible?
		ResetThrough();

		data()->vehicle = type;
		data()->preboard_move_speed = GetMoveSpeed();
		data()->boarding = true;
	}

	Game_System::SetBeforeVehicleMusic(Game_System::GetCurrentBGM());
	GetVehicle()->GetOn();
	return true;
}

bool Game_Player::GetOffVehicle() {
	auto* vehicle = GetVehicle();
	if (!vehicle) {
		return false;
	}

	if (InAirship()) {
		if (vehicle->IsAscendingOrDescending()) {
			return false;
		}

		SetSpriteDirection(Left);
	} else {
		const auto front_x = Game_Map::XwithDirection(GetX(), GetDirection());
		const auto front_y = Game_Map::YwithDirection(GetY(), GetDirection());

		if (!Game_Map::CanDisembarkShip(*this, front_x, front_y)) {
			return false;
		}

		data()->aboard = false;
		SetMoveSpeed(data()->preboard_move_speed);
		data()->unboarding = true;

		SetThrough(true);
		Move(GetDirection());
		ResetThrough();

		data()->vehicle = 0;
		Game_System::BgmPlay(Game_System::GetBeforeVehicleMusic());
	}

	vehicle->GetOff();
	return true;
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
			CheckEventTriggerThere({RPG::EventPage::Trigger_touched, RPG::EventPage::Trigger_collision}, nx, ny, false);
		}
		return;
	}

	if (InAirship()) {
		return;
	}

	int terrain_id = Game_Map::GetTerrainTag(GetX(), GetY());
	const RPG::Terrain* terrain = ReaderUtil::GetElement(Data::terrains, terrain_id);
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

bool Game_Player::IsAboard() const {
	return data()->aboard;
}

bool Game_Player::IsBoardingOrUnboarding() const {
	return data()->boarding || data()->unboarding;
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

