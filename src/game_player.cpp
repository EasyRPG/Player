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

// Constructor
Game_Player::Game_Player():
	location(Main_Data::game_data.party_location),
	teleporting(false),
	vehicle_type(-1),
	vehicle_getting_on(false),
	vehicle_getting_off(false),
	new_map_id(0),
	new_x(0),
	new_y(0) {
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

// Is Passable
bool Game_Player::IsPassable(int x, int y, int d) const {
	int new_x = x + (d == RPG::EventPage::Direction_right ? 1 : d == RPG::EventPage::Direction_left ? -1 : 0);
	int new_y = y + (d == RPG::EventPage::Direction_down ? 1 : d == RPG::EventPage::Direction_up ? -1 : 0);

	if (!Game_Map::IsValid(new_x, new_y)) return false;

	if (Player::debug_flag &&
		Input::IsPressed(Input::DEBUG_THROUGH)) {
			return true;
	}

	return Game_Character::IsPassable(x, y, d);
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
		move_route_owner = NULL;
		Refresh(); // Reset sprite if it was changed by a move
		Game_Map::Setup(new_map_id);
	}

	Main_Data::game_player->SetOpacity(255);

	MoveTo(new_x, new_y);
}

bool Game_Player::IsTeleporting() const {
	return teleporting;
}

// Center
void Game_Player::Center(int x, int y) {
	int center_x = (DisplayUi->GetWidth() - (SCREEN_TILE_WIDTH / 8)) * 8;
	int center_y = (DisplayUi->GetHeight() - (SCREEN_TILE_WIDTH / 16)) * 8;

	int max_x = (Game_Map::GetWidth() - DisplayUi->GetWidth() / 16) * 256;
	int max_y = (Game_Map::GetHeight() - DisplayUi->GetHeight() / 16) * 256;
	Game_Map::SetDisplayX(max(0, min((x * SCREEN_TILE_WIDTH - center_x), max_x)));
	Game_Map::SetDisplayY(max(0, min((y * SCREEN_TILE_WIDTH - center_y), max_y)));
}

// MoveTo
void Game_Player::MoveTo(int x, int y) {
	Game_Character::MoveTo(x, y);
	Center(x, y);

	// TODO: vehicle stuff
	/* if in_vehicle?                                    # Riding in vehicle
      vehicle = $game_map.vehicles[@vehicle_type]     # Get vehicle
      vehicle.refresh                                 # Refresh
    end */
}

void Game_Player::UpdateScroll(int last_real_x, int last_real_y) {
	int center_x = (DisplayUi->GetWidth() - (SCREEN_TILE_WIDTH / 8)) * 8;
	int center_y = (DisplayUi->GetHeight() - (SCREEN_TILE_WIDTH / 16)) * 8;

	if (Game_Map::IsPanLocked())
		return;

	if (Game_Map::GetPanX() != 0 || Game_Map::GetPanY() != 0) {
		int dx = real_x - center_x + Game_Map::GetPanX() - Game_Map::GetDisplayX();
		int dy = real_y - center_y + Game_Map::GetPanY() - Game_Map::GetDisplayY();
		if (dx > 0)
			Game_Map::ScrollRight(dx);
		if (dx < 0)
			Game_Map::ScrollLeft(-dx);
		if (dy > 0)
			Game_Map::ScrollDown(dy);
		if (dy < 0)
			Game_Map::ScrollUp(-dy);
	}
	else {
		if (real_y > last_real_y && real_y - Game_Map::GetDisplayY() > center_y)
			Game_Map::ScrollDown(real_y - last_real_y);

		if (real_x < last_real_x && real_x - Game_Map::GetDisplayX() < center_x)
			Game_Map::ScrollLeft(last_real_x - real_x);

		if (real_x > last_real_x && real_x - Game_Map::GetDisplayX() > center_x)
			Game_Map::ScrollRight(real_x - last_real_x);

		if (real_y < last_real_y && real_y - Game_Map::GetDisplayY() < center_y)
			Game_Map::ScrollUp(last_real_y - real_y);
	}
}

// Update
void Game_Player::Update() {
	bool last_moving = IsMoving();

	if (!IsMoving() && !Game_Map::GetInterpreter().IsRunning()
		/*move_route_forcing || Game_Temp::message_window_showing*/) {
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

	UpdateScroll(last_real_x, last_real_y);

	UpdateNonMoving(last_moving);
}

void Game_Player::UpdateNonMoving(bool last_moving) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return;

	if ( IsMoving() ) return;

	if ( last_moving && CheckTouchEvent() ) return;

	if ( !Game_Message::visible && Input::IsTriggered(Input::DECISION) ) {
		// TODO
		//if ( GetOnOffVehicle() ) return;
		if ( CheckActionEvent() ) return;
	}

	if ( last_moving )
		Game_Map::UpdateEncounterSteps();
}

bool Game_Player::CheckActionEvent() {
	// TODO
	//if ( IsInAirship() ) {
		//return false;
	//}
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
	int triggers[] = { 1, 2 };
	std::vector<int> v_triggers( triggers, triggers + sizeof triggers / sizeof(int) );
	return CheckEventTriggerHere(v_triggers);
}

bool Game_Player::CheckEventTriggerHere(const std::vector<int>& triggers) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, GetX(), GetY());

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if ( (*i)->GetLayer() == RPG::EventPage::Layers_below && std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end() ) {
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
	for (i = events.begin(); i != events.end(); i++) {
		if ( (*i)->GetLayer() == RPG::EventPage::Layers_same &&
			std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end()
		)
		{
			(*i)->Start();
			result = true;
		}
	}

	if ( !result && Game_Map::IsCounter(front_x, front_y) ) {
		front_x = Game_Map::XwithDirection(GetX(), GetDirection());
		front_y = Game_Map::YwithDirection(GetY(), GetDirection());

		Game_Map::GetEventsXY(events, front_x, front_y);

		std::vector<Game_Event*>::iterator i;
		for (i = events.begin(); i != events.end(); i++) {
			if ( (*i)->GetLayer() == 1 &&
				std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end()
			)
			{
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
	for (i = events.begin(); i != events.end(); i++) {
		if ( (*i)->GetLayer() == 1 && ((*i)->GetTrigger() == 1 || (*i)->GetTrigger() == 2) ) {
			(*i)->Start();
			result = true;
		}
	}
	return result;
}

void Game_Player::Refresh() {
	Game_Actor* actor;

	if (Game_Party::GetActors().empty()) {
		character_name.clear();
		return;
	}

	actor = Game_Party::GetActors()[0];

	character_name = actor->GetCharacterName();
	character_index = actor->GetCharacterIndex();
}

bool Game_Player::GetOnOffVehicle() {
	if (!IsMovable())
		return false;
	if (InVehicle())
		return GetOffVehicle();
    else
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

    vehicle_getting_on = true;
    vehicle_type = type;
	if (type == Game_Vehicle::Airship)
		through = true;
	// TODO:
	// else
	// 	ForceMoveForward();
	walking_bgm = Game_System::GetCurrentBGM();
	Game_Map::GetVehicle(type)->GetOn();
	return true;
}

bool Game_Player::GetOffVehicle() {
	if (InAirship()) {
		if (!AirshipLandOk(GetX(), GetY()))
			return false;
	}
	else {
		int front_x = Game_Map::XwithDirection(GetX(), GetDirection());
		int front_y = Game_Map::YwithDirection(GetY(), GetDirection());
		if (!CanWalk(front_x, front_y))
			return false;
	}

	Game_Map::GetVehicle((Game_Vehicle::Type) vehicle_type)->GetOff();
	if (InAirship())
		SetDirection(RPG::EventPage::Direction_down);
	else {
		// TODO
		// ForceMoveForward();
		opacity = 255;
	}

    vehicle_getting_off = true;
    move_speed = 4;
    through = false;
	Game_System::BgmPlay(walking_bgm);

	return true;
}

bool Game_Player::IsMovable() const {
	if (IsMoving())
		return false;
	if (GetMoveRouteForcing())
		return false;
	if (vehicle_getting_on)
		return false;
	if (vehicle_getting_off)
		return false;
	if (Game_Message::visible)
		return false;
	if (InAirship() && !Game_Map::GetVehicle(Game_Vehicle::Airship)->IsMovable())
		return false;
    return true;
}

bool Game_Player::InVehicle() const {
	return vehicle_type >= 0;
}

bool Game_Player::InAirship() const {
	return vehicle_type == Game_Vehicle::Airship;
}

bool Game_Player::AirshipLandOk(int x, int y) const {
	// TODO:
	// if (!Game_Map::AirshipLandOk(GetX(), GetY()))
	// 	return false;
	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);
	if (!events.empty())
		return false;
	return true;
}

bool Game_Player::CanWalk(int x, int y) {
	int last_vehicle_type = vehicle_type;
    vehicle_type = -1;
	bool result = IsPassable(x, y, GetDirection());
    vehicle_type = last_vehicle_type;
    return result;
}

void Game_Player::BeginMove() {
	int terrain_id = Game_Map::GetTerrainTag(GetX(), GetY());
	const RPG::Terrain& terrain = Data::terrains[terrain_id - 1];
	if (!terrain.on_damage_se || (terrain.on_damage_se && (terrain.damage > 0))) {
		Game_System::SePlay(terrain.footstep);
	}
	Game_Party::ApplyDamage(terrain.damage);
}
