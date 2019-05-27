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
	Game_Character(Vehicle, getDataFromType(_type))
{
	type = _type;
	SetDirection(Left);
	SetSpriteDirection(Left);
	SetAnimationType(AnimType::AnimType_non_continuous);
	SetLayer(RPG::EventPage::Layers_same);
	LoadSystemSettings();
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
	if (IsInUse()) {
		SetMapId(Game_Map::GetMapId());
	} else if (IsInCurrentMap()) {
		MoveTo(GetX(), GetY());
	}

	switch (type) {
		case None:
			break;
		case Boat:
		case Ship:
			SetMoveSpeed(RPG::EventPage::MoveSpeed_normal);
			break;
		case Airship:
			SetMoveSpeed(RPG::EventPage::MoveSpeed_double);
			break;
	}
}

void Game_Vehicle::SetPosition(int _map_id, int _x, int _y) {
	SetMapId(_map_id);
	MoveTo(_x, _y);
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
		data()->remaining_ascent = SCREEN_TILE_SIZE;
		SetFlying(true);
		Main_Data::game_player->SetFlying(true);
	}
	Game_System::BgmPlay(GetBGM());
}

void Game_Vehicle::GetOff() {
	if (type == Airship) {
		data()->remaining_descent = SCREEN_TILE_SIZE;
	} else {
		Main_Data::game_player->UnboardingFinished();
	}
	// Get off airship can be trigger while airship is moving. Don't break the animation
	// until its finished.
	if (type != Airship || (!IsMoving() && !IsJumping())) {
		SetDirection(Left);
		SetSpriteDirection(Left);
	}
}

bool Game_Vehicle::IsInUse() const {
	return Main_Data::game_player->GetVehicle() == this;
}

bool Game_Vehicle::IsAboard() const {
	return IsInUse() && Main_Data::game_player->IsAboard();
}

void Game_Vehicle::SyncWithPlayer() {
	SetX(Main_Data::game_player->GetX());
	SetY(Main_Data::game_player->GetY());
	SetRemainingStep(Main_Data::game_player->GetRemainingStep());
	SetJumping(Main_Data::game_player->IsJumping());
	SetBeginJumpX(Main_Data::game_player->GetBeginJumpX());
	SetBeginJumpY(Main_Data::game_player->GetBeginJumpY());
	if (!IsAscendingOrDescending()) {
		SetDirection(Main_Data::game_player->GetDirection());
		SetSpriteDirection(Main_Data::game_player->GetSpriteDirection());
	} else {
		if (!IsMoving() && !IsJumping()) {
			SetDirection(Left);
			SetSpriteDirection(Left);
		}
	}
}

int Game_Vehicle::GetAltitude() const {
	if (!IsFlying())
		return 0;
	else if (IsAscending())
		return (SCREEN_TILE_SIZE - data()->remaining_ascent) / (SCREEN_TILE_SIZE / TILE_SIZE);
	else if (IsDescending())
		return data()->remaining_descent / (SCREEN_TILE_SIZE / TILE_SIZE);
	else
		return SCREEN_TILE_SIZE / (SCREEN_TILE_SIZE / TILE_SIZE);
}

int Game_Vehicle::GetScreenY(bool apply_shift) const {
	return Game_Character::GetScreenY(apply_shift) - GetAltitude();
}

bool Game_Vehicle::CanLand() const {
	return Game_Map::CanLandAirship(GetX(), GetY());
}

void Game_Vehicle::UpdateAnimationAirship() {
	if (IsAboard()) {
		const auto limit = 11;

		IncAnimCount();

		if (GetAnimCount() > limit) {
			IncAnimFrame();
		}
	} else {
		ResetAnimation();
	}
}

void Game_Vehicle::UpdateAnimationShip() {
	const auto limit = 15;

	IncAnimCount();

	if (GetAnimCount() > limit) {
		IncAnimFrame();
	}
}

void Game_Vehicle::AnimateAscentDescent() {
	if (IsAscending()) {
		data()->remaining_ascent = data()->remaining_ascent - 8;
	} else if (IsDescending()) {
		data()->remaining_descent = data()->remaining_descent - 8;
		if (!IsDescending()) {
			if (CanLand()) {
				Main_Data::game_player->UnboardingFinished();
				SetFlying(false);
				Main_Data::game_player->SetFlying(false);
			} else {
				// Can't land here, ascend again
				data()->remaining_ascent = SCREEN_TILE_SIZE;
			}
		}
	}
}

void Game_Vehicle::Update() {
	if (IsProcessed()) {
		return;
	}
	SetProcessed(true);

	if (!IsAboard()) {
		Game_Character::UpdateMovement();
	}

	if (type == Airship) {
		UpdateAnimationAirship();
	} else {
		UpdateAnimationShip();
	}
	Game_Character::UpdateFlash();
}

int Game_Vehicle::GetVehicleType() const {
	return data()->vehicle;
}
