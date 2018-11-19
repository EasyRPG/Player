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

const char Game_Vehicle::TypeNames[4][8] {
	"Party", // RPG_RT special case, see CommandSetVehicleLocation
	"Boat",
	"Ship",
	"Airship"
};

static RPG::SaveVehicleLocation* getDataFromType(Game_Vehicle::Type ty) {
	assert(ty >= 1 && ty <= 3 && "Invalid Vehicle index");
	switch (ty) {
		case Game_Vehicle::Boat:
			return &Main_Data::game_data.boat_location;
		case Game_Vehicle::Ship:
			return &Main_Data::game_data.ship_location;
		case Game_Vehicle::Airship:
			return &Main_Data::game_data.airship_location;
		case Game_Vehicle::None:
			break;
	}
	return nullptr;
}

Game_Vehicle::Game_Vehicle(Type _type) :
	Game_Character(getDataFromType(_type))
{
	type = _type;
	SetDirection(Left);
	SetSpriteDirection(Left);
	SetAnimPaused(type == Airship);
	SetAnimationType(RPG::EventPage::AnimType_continuous);
	LoadSystemSettings();
}

int Game_Vehicle::GetSteppingSpeed() const {
	return 16;
}

int Game_Vehicle::GetMoveSpeed() const {
	return data()->move_speed;
}

void Game_Vehicle::SetMoveSpeed(int speed) {
	data()->move_speed = speed;
	if (IsInUse())
		Main_Data::game_player->SetMoveSpeed(speed);
}

int Game_Vehicle::GetOriginalMoveRouteIndex() const {
	return data()->original_move_route_index;
}

void Game_Vehicle::SetOriginalMoveRouteIndex(int new_index) {
	data()->original_move_route_index = new_index;
}


bool Game_Vehicle::MakeWay(int x, int y, int d) const {
	if (d > 3) {
		return MakeWayDiagonal(x, y, d);
	}

	int new_x = Game_Map::RoundX(x + (d == Right ? 1 : d == Left ? -1 : 0));
	int new_y = Game_Map::RoundY(y + (d == Down ? 1 : d == Up ? -1 : 0));

	if (!Game_Map::IsValid(new_x, new_y))
		return false;

	if (GetThrough()) return true;

	if (!Game_Map::IsPassableVehicle(new_x, new_y, type))
		return false;

	return true;
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
		assert(false);
		break;
	case Boat:
		return Game_System::GetSystemBGM(Game_System::BGM_Boat);
	case Ship:
		return Game_System::GetSystemBGM(Game_System::BGM_Ship);
	case Airship:
		return Game_System::GetSystemBGM(Game_System::BGM_Airship);
	}

	// keeping the compiler happy
	return Game_System::GetSystemBGM(Game_System::BGM_Battle);
}

void Game_Vehicle::Refresh() {
	if (IsInUse())
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
			SetLayer(IsInUse() ? RPG::EventPage::Layers_above : RPG::EventPage::Layers_same);
			SetMoveSpeed(RPG::EventPage::MoveSpeed_double);
			break;
	}
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
	return data()->remaining_ascent > 0;
}

bool Game_Vehicle::IsDescending() const {
	return data()->remaining_descent > 0;
}

bool Game_Vehicle::IsAscendingOrDescending() const {
	return IsAscending() || IsDescending();
}

bool Game_Vehicle::GetVisible() const {
	return IsInCurrentMap() && Game_Character::GetVisible();
}

void Game_Vehicle::GetOn() {
	if (type == Airship) {
		SetLayer(RPG::EventPage::Layers_above);
		data()->remaining_ascent = SCREEN_TILE_WIDTH;
		SetFlying(true);
		Main_Data::game_player->SetFlying(true);
	} else {
		SetAnimPaused(false);
	}
	Game_System::BgmPlay(GetBGM());
}

void Game_Vehicle::GetOff() {
	if (type == Airship) {
		data()->remaining_descent = SCREEN_TILE_WIDTH;
	} else {
		Main_Data::game_player->UnboardingFinished();
	}
	SetDirection(Left);
	SetSpriteDirection(Left);
}

bool Game_Vehicle::IsInUse() const {
	return Main_Data::game_player->GetVehicle() == this;
}

void Game_Vehicle::SyncWithPlayer() {
	if (!IsInUse() || IsAscending() || IsDescending())
		return;
	SetX(Main_Data::game_player->GetX());
	SetY(Main_Data::game_player->GetY());
	SetRemainingStep(Main_Data::game_player->GetRemainingStep());
	SetDirection(Main_Data::game_player->GetDirection());
	SetSpriteDirection(Main_Data::game_player->GetSpriteDirection());
}

int Game_Vehicle::GetAltitude() const {
	if (!IsFlying())
		return 0;
	else if (IsAscending())
		return (SCREEN_TILE_WIDTH - data()->remaining_ascent) / (SCREEN_TILE_WIDTH / TILE_SIZE);
	else if (IsDescending())
		return data()->remaining_descent / (SCREEN_TILE_WIDTH / TILE_SIZE);
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

	if (!Main_Data::game_player->IsBoardingOrUnboarding()) {
		SyncWithPlayer();
	}

	if (type == Airship) {
		if (IsAscending()) {
			data()->remaining_ascent = data()->remaining_ascent - 8;
			if (!IsAscending())
				SetAnimPaused(false);
		} else if (IsDescending()) {
			data()->remaining_descent = data()->remaining_descent - 8;
			if (!IsDescending()) {
				if (CanLand()) {
					SetLayer(RPG::EventPage::Layers_same);
					Main_Data::game_player->UnboardingFinished();
					SetFlying(false);
					Main_Data::game_player->SetFlying(false);
					SetAnimPaused(true);
					SetAnimFrame(AnimFrame::Frame_middle);
				} else {
					// Can't land here, ascend again
					data()->remaining_ascent = SCREEN_TILE_WIDTH;
				}
			}
		}
	}
}

bool Game_Vehicle::CheckEventTriggerTouch(int /* x */, int /* y */) {
	return false;
}
