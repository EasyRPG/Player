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
	vehicle_type(-1),
	vehicle_getting_on(false),
	vehicle_getting_off(false),
	new_map_id(0),
	new_x(0),
	new_y(0) {
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
	return location.original_move_route_index;
}

void Game_Player::SetOriginalMoveRouteIndex(int new_index) {
	location.original_move_route_index = new_index;
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

Color Game_Player::GetFlashColor() const {
	return Color(location.flash_red, location.flash_green, location.flash_blue, 0);
}

void Game_Player::SetFlashColor(const Color& flash_color) {
	location.flash_red = flash_color.red;
	location.flash_blue = flash_color.blue;
	location.flash_green = flash_color.green;
}

int Game_Player::GetFlashLevel() const {
	return location.flash_current_level;
}

void Game_Player::SetFlashLevel(int flash_level) {
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
	}

	Main_Data::game_player->SetOpacity(255);

	MoveTo(new_x, new_y);
}

bool Game_Player::IsTeleporting() const {
	return teleporting;
}

void Game_Player::Center(int x, int y) {

	int center_x = (DisplayUi->GetWidth() / ( TILE_SIZE/16) - TILE_SIZE) * 8;
	int center_y = (DisplayUi->GetHeight() / (TILE_SIZE/16) - (TILE_SIZE/2)) * 8;
	int max_x = (Game_Map::GetWidth() - DisplayUi->GetWidth() / TILE_SIZE) * (SCREEN_TILE_WIDTH);
	int max_y = (Game_Map::GetHeight() - DisplayUi->GetHeight() / TILE_SIZE) * (SCREEN_TILE_WIDTH);
	Game_Map::SetDisplayX(max(0, min((x * SCREEN_TILE_WIDTH - center_x), max_x)));
	Game_Map::SetDisplayY(max(0, min((y * SCREEN_TILE_WIDTH - center_y), max_y)));	
}

void Game_Player::MoveTo(int x, int y) {
	x = max(0, min(x, Game_Map::GetWidth() - 1));
	y = max(0, min(y, Game_Map::GetHeight() - 1));

	Game_Character::MoveTo(x, y);
	Center(x, y);

	// TODO: vehicle stuff
	/* if in_vehicle?                                    # Riding in vehicle
      vehicle = $game_map.vehicles[@vehicle_type]     # Get vehicle
      vehicle.refresh                                 # Refresh
    end */
}

void Game_Player::UpdateScroll(int last_real_x, int last_real_y) {
	int center_x = (DisplayUi->GetWidth() / ( TILE_SIZE/16) - TILE_SIZE) * 8;
	int center_y = (DisplayUi->GetHeight() / (TILE_SIZE/16) - (TILE_SIZE/2)) * 8;

 
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

void Game_Player::Update() {
	bool last_moving = IsMoving();

	if (!IsMoving() && !Game_Map::GetInterpreter().IsRunning() && !IsMoveRouteOverwritten() && !Game_Message::visible) {
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
	if (Game_Map::GetInterpreter().IsRunning()) return;

	if (IsMoving() ) return;

	if (last_moving && CheckTouchEvent()) return;

	if (!Game_Message::visible && Input::IsTriggered(Input::DECISION)) {
		// TODO
		//if ( GetOnOffVehicle() ) return;
		if ( CheckActionEvent() ) return;
	}

	if (last_moving)
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
			if (!(*i)->IsDirectionFixed() && result) {
				(*i)->SetPrelockDirection((*i)->GetDirection());
				(*i)->TurnTowardHero();
			}

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
			(*i)->Start();
			result = true;
			if (!(*i)->IsDirectionFixed() && !(*i)->GetList().empty()) {
				(*i)->SetPrelockDirection((*i)->GetDirection());
				(*i)->TurnTowardHero();
			}
		}
	}

	if ( !result && Game_Map::IsCounter(front_x, front_y) ) {
		front_x = Game_Map::XwithDirection(GetX(), GetDirection());
		front_y = Game_Map::YwithDirection(GetY(), GetDirection());

		Game_Map::GetEventsXY(events, front_x, front_y);

		std::vector<Game_Event*>::iterator i;
		for (i = events.begin(); i != events.end(); ++i) {
			if ( (*i)->GetLayer() == 1 &&
				std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end()
			)
			{
				(*i)->Start();
				result = true;
				if (!(*i)->IsDirectionFixed() && !(*i)->GetList().empty()) {
					(*i)->SetPrelockDirection((*i)->GetDirection());
					(*i)->TurnTowardHero();
				}
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
			(*i)->Start();
			result = true;
			if (!(*i)->IsDirectionFixed() && !(*i)->GetList().empty()) {
				(*i)->SetPrelockDirection((*i)->GetDirection());
				(*i)->TurnTowardHero();
			}

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
	SetMoveSpeed(4);
	through = false;
	Game_System::BgmPlay(walking_bgm);

	return true;
}

bool Game_Player::IsMovable() const {
	if (IsMoving())
		return false;
	if (IsMoveRouteOverwritten())
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
	Main_Data::game_party->ApplyDamage(terrain.damage);
}
