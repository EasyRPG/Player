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
#include "game_actor.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_system.h"
#include "input.h"
#include "main_data.h"
#include "player.h"
#include "util_macro.h"
#include <algorithm>

Game_Player::Game_Player():
	location(Main_Data::game_data.party_location),
	teleporting(false),
	new_map_id(0),
	new_x(0),
	new_y(0),
	last_pan_x(0),
	last_pan_y(0) {
	SetDirection(RPG::EventPage::Direction_down);
	SetMoveSpeed(4);
}

int Game_Player::GetX() const {
	return location.position_x;
}

void Game_Player::SetX(int new_x) {
	location.position_x = new_x;
}

int Game_Player::GetY() const {
	return location.position_y;
}

void Game_Player::SetY(int new_y) {
	location.position_y = new_y;
}

int Game_Player::GetMapId() const {
	return location.map_id;
}

void Game_Player::SetMapId(int new_map_id) {
	location.map_id = new_map_id;
}

int Game_Player::GetDirection() const {
	return location.direction;
}

void Game_Player::SetDirection(int new_direction) {
	location.direction = new_direction;
}

int Game_Player::GetPrelockDirection() const {
	return location.prelock_direction;
}

void Game_Player::SetPrelockDirection(int new_direction) {
	location.prelock_direction = new_direction;
}

bool Game_Player::IsFacingLocked() const {
	return location.lock_facing;
}

void Game_Player::SetFacingLocked(bool locked) {
	location.lock_facing = locked;
}

int Game_Player::GetLayer() const {
	return location.layer;
}

void Game_Player::SetLayer(int new_layer) {
	location.layer = new_layer;
}

int Game_Player::GetMoveSpeed() const {
	return location.move_speed;
}

void Game_Player::SetMoveSpeed(int speed) {
	location.move_speed = speed;
}

int Game_Player::GetMoveFrequency() const {
	return location.move_frequency;
}

void Game_Player::SetMoveFrequency(int frequency) {
	location.move_frequency = frequency;
}

const RPG::MoveRoute& Game_Player::GetMoveRoute() const {
	return location.move_route;
}

void Game_Player::SetMoveRoute(const RPG::MoveRoute& move_route) {
	location.move_route = move_route;
}

int Game_Player::GetOriginalMoveRouteIndex() const {
	return 0;
}

void Game_Player::SetOriginalMoveRouteIndex(int /* new_index */) {
	// no-op
}

int Game_Player::GetMoveRouteIndex() const {
	return location.move_route_index;
}

void Game_Player::SetMoveRouteIndex(int new_index) {
	location.move_route_index = new_index;
}

bool Game_Player::IsMoveRouteOverwritten() const {
	return location.move_route_overwrite;
}

void Game_Player::SetMoveRouteOverwritten(bool force) {
	location.move_route_overwrite = force;
}

bool Game_Player::IsMoveRouteRepeated() const {
	return location.move_route_repeated;
}

void Game_Player::SetMoveRouteRepeated(bool force) {
	location.move_route_repeated = force;
}

const std::string& Game_Player::GetSpriteName() const {
	return location.sprite_name;
}

void Game_Player::SetSpriteName(const std::string& sprite_name) {
	location.sprite_name = sprite_name;
}

int Game_Player::GetSpriteIndex() const {
	return location.sprite_id;
}

void Game_Player::SetSpriteIndex(int index) {
	location.sprite_id = index;
}

bool Game_Player::GetVisible() const {
	return visible && !location.aboard;
}

Color Game_Player::GetFlashColor() const {
	return Color(location.flash_red, location.flash_green, location.flash_blue, 128);
}

void Game_Player::SetFlashColor(const Color& flash_color) {
	location.flash_red = flash_color.red;
	location.flash_blue = flash_color.blue;
	location.flash_green = flash_color.green;
}

double Game_Player::GetFlashLevel() const {
	return location.flash_current_level;
}

void Game_Player::SetFlashLevel(double flash_level) {
	location.flash_current_level = flash_level;
}

int Game_Player::GetFlashTimeLeft() const {
	return location.flash_time_left;
}

void Game_Player::SetFlashTimeLeft(int time_left) {
	location.flash_time_left = time_left;
}

void Game_Player::ReserveTeleport(int map_id, int x, int y) {
	new_map_id = map_id;
	new_x = x;
	new_y = y;
}

void Game_Player::StartTeleport() {
	teleporting = true;
}

void Game_Player::PerformTeleport() {
	if (!teleporting) return;

	teleporting = false;

	if (Game_Map::GetMapId() != new_map_id) {
		Refresh(); // Reset sprite if it was changed by a move
		Game_Map::Setup(new_map_id);
		last_pan_x = 0;
		last_pan_y = 0;
	}

	SetOpacity(255);

	MoveTo(new_x, new_y);
	if (InVehicle())
		GetVehicle()->MoveTo(new_x, new_y);
}

bool Game_Player::IsPassable(int x, int y, int d) const {
	if (Player::debug_flag && Input::IsPressed(Input::DEBUG_THROUGH))
		return true;

	if (location.aboard)
		return GetVehicle()->IsPassable(x, y, d);

	return Game_Character::IsPassable(x, y, d);
}

bool Game_Player::IsTeleporting() const {
	return teleporting;
}

void Game_Player::Center(int x, int y) {
	int center_x = (DisplayUi->GetWidth() / (TILE_SIZE / 16) - TILE_SIZE * 2) * 8 - Game_Map::GetPanX();
	int center_y = (DisplayUi->GetHeight() / (TILE_SIZE / 16) - TILE_SIZE) * 8 - Game_Map::GetPanY();
	int max_x = (Game_Map::GetWidth() - DisplayUi->GetWidth() / TILE_SIZE) * SCREEN_TILE_WIDTH;
	int max_y = (Game_Map::GetHeight() - DisplayUi->GetHeight() / TILE_SIZE) * SCREEN_TILE_WIDTH;
	Game_Map::SetDisplayX(max(0, min((x * SCREEN_TILE_WIDTH - center_x), max_x)));
	Game_Map::SetDisplayY(max(0, min((y * SCREEN_TILE_WIDTH - center_y), max_y)));
}

void Game_Player::MoveTo(int x, int y) {
	x = max(0, min(x, Game_Map::GetWidth() - 1));
	y = max(0, min(y, Game_Map::GetHeight() - 1));

	Game_Character::MoveTo(x, y);
	Center(x, y);
}

void Game_Player::UpdateScroll(int last_real_x, int last_real_y) {
	int center_x = (DisplayUi->GetWidth() / (TILE_SIZE / 16) - TILE_SIZE * 2) * 8 - Game_Map::GetPanX();
	int center_y = (DisplayUi->GetHeight() / (TILE_SIZE / 16) - TILE_SIZE) * 8 - Game_Map::GetPanY();
	int dx = 0;
	int dy = 0;

	if (!Game_Map::IsPanLocked()) {
		if ((real_x > last_real_x && real_x - Game_Map::GetDisplayX() > center_x) ||
			(real_x < last_real_x && real_x - Game_Map::GetDisplayX() < center_x)) {
			dx = real_x - last_real_x;
		}
		if ((real_y > last_real_y && real_y - Game_Map::GetDisplayY() > center_y) ||
			(real_y < last_real_y && real_y - Game_Map::GetDisplayY() < center_y)) {
			dy = real_y - last_real_y;
		}
	}

	if (Game_Map::GetPanX() != last_pan_x || Game_Map::GetPanY() != last_pan_y) {
		dx += Game_Map::GetPanX() - last_pan_x;
		dy += Game_Map::GetPanY() - last_pan_y;

		last_pan_x = Game_Map::GetPanX();
		last_pan_y = Game_Map::GetPanY();
	}

	if (dx > 0)
		Game_Map::ScrollRight(dx);
	else if (dx < 0)
		Game_Map::ScrollLeft(-dx);
	if (dy > 0)
		Game_Map::ScrollDown(dy);
	else if (dy < 0)
		Game_Map::ScrollUp(-dy);
}

void Game_Player::Update() {
	bool last_moving = IsMoving();

	if (IsMovable() && !Game_Map::GetInterpreter().IsRunning()) {
		switch (Input::dir4) {
			case 2:
				MoveDown();
				break;
			case 4:
				MoveLeft();
				break;
			case 6:
				MoveRight();
				break;
			case 8:
				MoveUp();
		}
	}

	int last_real_x = real_x;
	int last_real_y = real_y;

	Game_Character::Update();

	if (location.aboard)
		GetVehicle()->SyncWithPlayer();

	UpdateScroll(last_real_x, last_real_y);

	UpdateNonMoving(last_moving);
}

void Game_Player::UpdateNonMoving(bool last_moving) {
	if (Game_Map::GetInterpreter().IsRunning()) return;

	if (IsMoving() ) return;

	if (last_moving && location.boarding) {
		// Boarding completed
		location.aboard = true;
		location.boarding = false;
		SetMoveSpeed(GetVehicle()->GetMoveSpeed());
		SetDirection(GetVehicle()->GetDirection());
		return;
	}

	if (last_moving && location.unboarding) {
		// Unboarding completed
		location.unboarding = false;
		location.vehicle = Game_Vehicle::None;
		CheckTouchEvent();
		return;
	}

	if (InAirship() && !GetVehicle()->IsInUse()) {
		// Airship has landed
		Unboard();
		location.vehicle = Game_Vehicle::None;
		SetDirection(RPG::EventPage::Direction_down);

	}

	if (last_moving && CheckTouchEvent()) return;

	if (!Game_Message::visible && Input::IsTriggered(Input::DECISION)) {
		if ( GetOnOffVehicle() ) return;
		if ( CheckActionEvent() ) return;
	}

	if (last_moving)
		Game_Map::UpdateEncounterSteps();
}

bool Game_Player::CheckActionEvent() {
	if (InAirship())
		return false;
	int triggers_here[] = { 0 };
	std::vector<int> triggers(triggers_here, triggers_here + sizeof triggers_here / sizeof(int));

	if ( CheckEventTriggerHere(triggers) ) {
		return true;
	}

	int triggers_there[] = { 0, 1, 2 };
	triggers.assign(triggers_there, triggers_there + sizeof triggers_there / sizeof(int));

	return CheckEventTriggerThere(triggers);

}

bool Game_Player::CheckTouchEvent() {
	if (InAirship())
		return false;
	int triggers[] = { RPG::EventPage::Trigger_touched, RPG::EventPage::Trigger_collision };
	std::vector<int> v_triggers( triggers, triggers + sizeof(triggers) / sizeof(int) );
	return CheckEventTriggerHere(v_triggers);
}

bool Game_Player::CheckEventTriggerHere(const std::vector<int>& triggers) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, GetX(), GetY());

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); ++i) {
		if (((*i)->GetLayer() == RPG::EventPage::Layers_below ||
			(*i)->GetLayer() == RPG::EventPage::Layers_above)
			&& std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end() ) {
			(*i)->Start();
			result = (*i)->GetStarting();
		}
	}
	return result;
}

bool Game_Player::CheckEventTriggerThere(const std::vector<int>& triggers) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
	int front_y = Game_Map::YwithDirection(GetY(), GetDirection());

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, front_x, front_y);

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); ++i) {
		if ( (*i)->GetLayer() == RPG::EventPage::Layers_same &&
			std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end()
		)
		{
			if (!(*i)->GetList().empty()) {
				(*i)->Lock();
			}
			(*i)->Start();
			result = true;
		}
	}

	if ( !result && Game_Map::IsCounter(front_x, front_y) ) {
		front_x = Game_Map::XwithDirection(front_x, GetDirection());
		front_y = Game_Map::YwithDirection(front_y, GetDirection());

		Game_Map::GetEventsXY(events, front_x, front_y);

		std::vector<Game_Event*>::iterator i;
		for (i = events.begin(); i != events.end(); ++i) {
			if ( (*i)->GetLayer() == 1 &&
				std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end()
			)
			{
				if (!(*i)->GetList().empty()) {
					(*i)->Lock();
				}
				(*i)->Start();
				result = true;
			}
		}
	}
	return result;
}

bool Game_Player::CheckEventTriggerTouch(int x, int y) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); ++i) {
		if ((*i)->GetLayer() == RPG::EventPage::Layers_same &&
			((*i)->GetTrigger() == RPG::EventPage::Trigger_touched ||
			(*i)->GetTrigger() == RPG::EventPage::Trigger_collision) ) {
			if (!(*i)->GetList().empty()) {
				(*i)->Lock();
			}
			(*i)->Start();
			result = true;

		}
	}
	return result;
}

void Game_Player::Refresh() {
	Game_Actor* actor;

	if (Main_Data::game_party->GetActors().empty()) {
		SetSpriteName("");
		return;
	}

	actor = Main_Data::game_party->GetActors()[0];

	SetSpriteName(actor->GetSpriteName());
	SetSpriteIndex(actor->GetSpriteIndex());

	if (location.aboard)
		GetVehicle()->SyncWithPlayer();
}

bool Game_Player::GetOnOffVehicle() {
	if (!IsMovable())
		return false;
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

	location.vehicle = type;
	location.preboard_move_speed = GetMoveSpeed();
	if (type != Game_Vehicle::Airship) {
		location.boarding = true;
		through = true;
		MoveForward();
		through = false;
	} else {
		location.aboard = true;
		SetMoveSpeed(GetVehicle()->GetMoveSpeed());
		SetDirection(RPG::EventPage::Direction_left);
	}

	walking_bgm = Game_System::GetCurrentBGM();
	GetVehicle()->GetOn();
	return true;
}

bool Game_Player::GetOffVehicle() {
	if (!InAirship()) {
		int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
		int front_y = Game_Map::YwithDirection(GetY(), GetDirection());
		if (!CanWalk(front_x, front_y))
			return false;
	}

	GetVehicle()->GetOff();
	if (!InAirship()) {
		location.unboarding = true;
		Unboard();
		through = true;
		MoveForward();
		through = false;
	}

	return true;
}

bool Game_Player::IsMovable() const {
	if (IsMoving())
		return false;
	if (IsMoveRouteOverwritten())
		return false;
	if (location.boarding || location.unboarding)
		return false;
	if (Game_Message::message_waiting)
		return false;
	if (InAirship() && !GetVehicle()->IsMovable())
		return false;
    return true;
}

bool Game_Player::InVehicle() const {
	return location.vehicle > 0;
}

bool Game_Player::InAirship() const {
	return location.vehicle == Game_Vehicle::Airship;
}

Game_Vehicle* Game_Player::GetVehicle() const {
	return Game_Map::GetVehicle((Game_Vehicle::Type) location.vehicle);
}

bool Game_Player::CanWalk(int x, int y) {
	return Game_Map::IsPassable(x, y, GetDirection(), this);
}

void Game_Player::BeginMove() {
	int terrain_id = Game_Map::GetTerrainTag(GetX(), GetY());
	const RPG::Terrain& terrain = Data::terrains[terrain_id - 1];
	if (!terrain.on_damage_se || (terrain.on_damage_se && (terrain.damage > 0))) {
		Game_System::SePlay(terrain.footstep);
	}
	Main_Data::game_party->ApplyDamage(terrain.damage);
}

void Game_Player::Unboard() {
	location.aboard = false;
	SetMoveSpeed(location.preboard_move_speed);

	Game_System::BgmPlay(walking_bgm);
}
