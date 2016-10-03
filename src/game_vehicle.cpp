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
#include <cassert>
#include "data.h"
#include "main_data.h"
#include "game_system.h"
#include "game_map.h"
#include "game_player.h"
#include "game_vehicle.h"
#include "output.h"

Game_Vehicle::Game_Vehicle(Type _type) :
	data(_type == Boat ? Main_Data::game_data.boat_location :
		 _type == Ship ? Main_Data::game_data.ship_location :
		 Main_Data::game_data.airship_location) {
	assert(_type >= 1 && _type <= 3 && "Invalid Vehicle index");
	type = _type;
	driving = false;
	SetDirection(Left);
	SetSpriteDirection(Left);
	walk_animation = type != Airship;
	animation_type = RPG::EventPage::AnimType_continuous;
	LoadSystemSettings();
}

int Game_Vehicle::GetX() const {
	return data.position_x;
}

void Game_Vehicle::SetX(int new_x) {
	data.position_x = new_x;
}

int Game_Vehicle::GetY() const {
	return data.position_y;
}

void Game_Vehicle::SetY(int new_y) {
	data.position_y = new_y;
}

int Game_Vehicle::GetMapId() const {
	return data.map_id;
}

void Game_Vehicle::SetMapId(int new_map_id) {
	data.map_id = new_map_id;
}

int Game_Vehicle::GetDirection() const {
	return data.direction;
}

void Game_Vehicle::SetDirection(int new_direction) {
	data.direction = new_direction;
}

int Game_Vehicle::GetSpriteDirection() const {
	return data.sprite_direction;
}

void Game_Vehicle::SetSpriteDirection(int new_direction) {
	data.sprite_direction = new_direction;
}

bool Game_Vehicle::IsFacingLocked() const {
	return data.lock_facing;
}

void Game_Vehicle::SetFacingLocked(bool locked) {
	data.lock_facing = locked;
}

int Game_Vehicle::GetLayer() const {
	return data.layer;
}

void Game_Vehicle::SetLayer(int new_layer) {
	data.layer = new_layer;
}

int Game_Vehicle::GetSteppingSpeed() const {
	return 16;
}

int Game_Vehicle::GetMoveSpeed() const {
	return data.move_speed;
}

void Game_Vehicle::SetMoveSpeed(int speed) {
	data.move_speed = speed;
	if (driving)
		Main_Data::game_player->SetMoveSpeed(speed);
}

int Game_Vehicle::GetMoveFrequency() const {
	return data.move_frequency;
}

void Game_Vehicle::SetMoveFrequency(int frequency) {
	data.move_frequency = frequency;
}

const RPG::MoveRoute& Game_Vehicle::GetMoveRoute() const {
	return data.move_route;
}

void Game_Vehicle::SetMoveRoute(const RPG::MoveRoute& move_route) {
	data.move_route = move_route;
}

int Game_Vehicle::GetOriginalMoveRouteIndex() const {
	return data.original_move_route_index;
}

void Game_Vehicle::SetOriginalMoveRouteIndex(int new_index) {
	data.original_move_route_index = new_index;
}

int Game_Vehicle::GetMoveRouteIndex() const {
	return data.move_route_index;
}

void Game_Vehicle::SetMoveRouteIndex(int new_index) {
	data.move_route_index = new_index;
}

bool Game_Vehicle::IsMoveRouteOverwritten() const {
	return data.move_route_overwrite;
}

void Game_Vehicle::SetMoveRouteOverwritten(bool force) {
	data.move_route_overwrite = force;
}

bool Game_Vehicle::IsMoveRouteRepeated() const {
	return data.move_route_repeated;
}

void Game_Vehicle::SetMoveRouteRepeated(bool force) {
	data.move_route_repeated = force;
}

const std::string& Game_Vehicle::GetSpriteName() const {
	return data.sprite_name;
}

void Game_Vehicle::SetSpriteName(const std::string& sprite_name) {
	data.sprite_name = sprite_name;
}

int Game_Vehicle::GetSpriteIndex() const {
	return data.sprite_id;
}

void Game_Vehicle::SetSpriteIndex(int index) {
	data.sprite_id = index;
}

Color Game_Vehicle::GetFlashColor() const {
	return Color(data.flash_red, data.flash_green, data.flash_blue, 128);
}

void Game_Vehicle::SetFlashColor(const Color& flash_color) {
	data.flash_red = flash_color.red;
	data.flash_blue = flash_color.blue;
	data.flash_green = flash_color.green;
}

double Game_Vehicle::GetFlashLevel() const {
	return data.flash_current_level;
}

void Game_Vehicle::SetFlashLevel(double flash_level) {
	data.flash_current_level = flash_level;
}

int Game_Vehicle::GetFlashTimeLeft() const {
	return data.flash_time_left;
}

void Game_Vehicle::SetFlashTimeLeft(int time_left) {
	data.flash_time_left = time_left;
}

bool Game_Vehicle::GetThrough() const {
	return data.through;
}

void Game_Vehicle::SetThrough(bool through) {
	data.through = through;
}

bool Game_Vehicle::MakeWay(int x, int y, int d) const {
	int new_x = Game_Map::RoundX(x + (d == Right ? 1 : d == Left ? -1 : 0));
	int new_y = Game_Map::RoundY(y + (d == Down ? 1 : d == Up ? -1 : 0));

	if (!Game_Map::IsValid(new_x, new_y))
		return false;

	if (GetThrough()) return true;

	if (!Game_Map::IsPassableVehicle(new_x, new_y, type))
		return false;

	return true;
}

int Game_Vehicle::GetBushDepth() const {
	if (data.flying)
		return 0;

	return Game_Character::GetBushDepth();
}

void Game_Vehicle::LoadSystemSettings() {
	switch (type) {
		case None:
			break;
		case Boat:
			SetSpriteName(Data::system.boat_name);
			SetSpriteIndex(Data::system.boat_index);
			SetMapId(Data::treemap.start.boat_map_id);
			SetX(Data::treemap.start.boat_x);
			SetY(Data::treemap.start.boat_y);
			break;
		case Ship:
			SetSpriteName(Data::system.ship_name);
			SetSpriteIndex(Data::system.ship_index);
			SetMapId(Data::treemap.start.ship_map_id);
			SetX(Data::treemap.start.ship_x);
			SetY(Data::treemap.start.ship_y);
			break;
		case Airship:
			SetSpriteName(Data::system.airship_name);
			SetSpriteIndex(Data::system.airship_index);
			SetMapId(Data::treemap.start.airship_map_id);
			SetX(Data::treemap.start.airship_x);
			SetY(Data::treemap.start.airship_y);
			break;
	}
}

RPG::Music& Game_Vehicle::GetBGM() {
	switch (type) {
	case None:
		break;
	case Boat:
		return Game_System::GetSystemBGM(Game_System::BGM_Boat);
	case Ship:
		return Game_System::GetSystemBGM(Game_System::BGM_Ship);
	case Airship:
		return Game_System::GetSystemBGM(Game_System::BGM_Airship);
	}

	assert(false);
}

void Game_Vehicle::Refresh() {
	if (!driving && Main_Data::game_player->GetVehicle() == this)
		driving = true;

	if (driving)
		SetMapId(Game_Map::GetMapId());
	else if (IsInCurrentMap())
		MoveTo(GetX(), GetY());

	switch (type) {
		case None:
			break;
		case Boat:
		case Ship:
			SetLayer(RPG::EventPage::Layers_same);
			SetMoveSpeed(RPG::EventPage::MoveSpeed_normal);
			break;
		case Airship:
			SetLayer(driving ? RPG::EventPage::Layers_above : RPG::EventPage::Layers_same);
			SetMoveSpeed(RPG::EventPage::MoveSpeed_double);
			break;
	}
	walk_animation = (type != Airship) || driving;
}

void Game_Vehicle::SetPosition(int _map_id, int _x, int _y) {
	SetMapId(_map_id);
	SetX(_x);
	SetY(_y);
}

bool Game_Vehicle::IsInCurrentMap() const {
	return GetMapId() == Game_Map::GetMapId();
}

bool Game_Vehicle::IsInPosition(int x, int y) const {
	return IsInCurrentMap() && Game_Character::IsInPosition(x, y);
}

bool Game_Vehicle::IsAscending() const {
	return data.remaining_ascent > 0;
}

bool Game_Vehicle::IsDescending() const {
	return data.remaining_descent > 0;
}

bool Game_Vehicle::GetVisible() const {
	return IsInCurrentMap() && Game_Character::GetVisible();
}

void Game_Vehicle::GetOn() {
	driving = true;
	if (type == Airship) {
		SetLayer(RPG::EventPage::Layers_above);
		data.remaining_ascent = SCREEN_TILE_WIDTH;
		data.flying = true;
	} else {
		walk_animation = true;
	}
	Game_System::BgmPlay(GetBGM());
}

void Game_Vehicle::GetOff() {
	if (type == Airship) {
		data.remaining_descent = SCREEN_TILE_WIDTH;
	} else {
		driving = false;
	}
	SetDirection(Left);
	SetSpriteDirection(Left);
}

bool Game_Vehicle::IsInUse() const {
	return driving;
}

void Game_Vehicle::SyncWithPlayer() {
	if (!driving || IsAscending() || IsDescending())
		return;
	SetX(Main_Data::game_player->GetX());
	SetY(Main_Data::game_player->GetY());
	remaining_step = Main_Data::game_player->GetRemainingStep();
	SetDirection(Main_Data::game_player->GetDirection());
	SetSpriteDirection(Main_Data::game_player->GetSpriteDirection());
}

int Game_Vehicle::GetAltitude() const {
	if (!data.flying)
		return 0;
	else if (IsAscending())
		return (SCREEN_TILE_WIDTH - data.remaining_ascent) / (SCREEN_TILE_WIDTH / TILE_SIZE);
	else if (IsDescending())
		return data.remaining_descent / (SCREEN_TILE_WIDTH / TILE_SIZE);
	else
		return SCREEN_TILE_WIDTH / (SCREEN_TILE_WIDTH / TILE_SIZE);
}

int Game_Vehicle::GetScreenY() const {
	return Game_Character::GetScreenY() - GetAltitude();
}

bool Game_Vehicle::IsMovable() {
	if (!IsInUse())
		return false;
	if (type == Airship && (IsAscending() || IsDescending()))
		return false;
	return !IsMoving();
}

bool Game_Vehicle::CanLand() const {
	if (!Game_Map::AirshipLandOk(GetX(), GetY()))
		return false;
	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, GetX(), GetY());
	if (!events.empty())
		return false;
	if (!Game_Map::IsLandable(GetX(), GetY(), nullptr))
		return false;
	if (Game_Map::GetVehicle(Ship)->IsInPosition(GetX(), GetY()))
		return false;
	if (Game_Map::GetVehicle(Boat)->IsInPosition(GetX(), GetY()))
		return false;
	return true;
}

void Game_Vehicle::Update() {
	Game_Character::Update();
	Game_Character::UpdateSprite();
	SyncWithPlayer();

	if (type == Airship) {
		if (IsAscending()) {
			data.remaining_ascent -= 8;
			if (!IsAscending())
				walk_animation = true;
		} else if (IsDescending()) {
			data.remaining_descent -= 8;
			if (!IsDescending()) {
				if (CanLand()) {
					SetLayer(RPG::EventPage::Layers_same);
					driving = false;
					data.flying = false;
					walk_animation = false;
					pattern = 1;
				} else {
					// Can't land here, ascend again
					data.remaining_ascent = SCREEN_TILE_WIDTH;
				}
			}
		}
	}
}

bool Game_Vehicle::CheckEventTriggerTouch(int /* x */, int /* y */) {
	return false;
}
