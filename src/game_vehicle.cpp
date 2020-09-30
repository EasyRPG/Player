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
#include <lcf/data.h>
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

Game_Vehicle::Game_Vehicle(Type type)
	: Game_VehicleBase(Vehicle)
{
	data()->vehicle = static_cast<int>(type);
	SetDirection(Left);
	SetFacing(Left);
	SetAnimationType(AnimType::AnimType_non_continuous);
	SetLayer(lcf::rpg::EventPage::Layers_same);

	switch (GetVehicleType()) {
		case None:
			break;
		case Boat:
			SetSpriteGraphic(ToString(lcf::Data::system.boat_name), lcf::Data::system.boat_index);
			SetMapId(lcf::Data::treemap.start.boat_map_id);
			SetX(lcf::Data::treemap.start.boat_x);
			SetY(lcf::Data::treemap.start.boat_y);
			SetMoveSpeed(lcf::rpg::EventPage::MoveSpeed_normal);
			break;
		case Ship:
			SetSpriteGraphic(ToString(lcf::Data::system.ship_name), lcf::Data::system.ship_index);
			SetMapId(lcf::Data::treemap.start.ship_map_id);
			SetX(lcf::Data::treemap.start.ship_x);
			SetY(lcf::Data::treemap.start.ship_y);
			SetMoveSpeed(lcf::rpg::EventPage::MoveSpeed_normal);
			break;
		case Airship:
			SetSpriteGraphic(ToString(lcf::Data::system.airship_name), lcf::Data::system.airship_index);
			SetMapId(lcf::Data::treemap.start.airship_map_id);
			SetX(lcf::Data::treemap.start.airship_x);
			SetY(lcf::Data::treemap.start.airship_y);
			SetMoveSpeed(lcf::rpg::EventPage::MoveSpeed_double);
			break;
	}
}

void Game_Vehicle::SetSaveData(lcf::rpg::SaveVehicleLocation save) {
	auto type = data()->vehicle;
	*data() = std::move(save);

	// Old EasyRPG savegames pre 6.0 didn't write the vehicle chunk.
	data()->vehicle = type;

	SanitizeData(TypeNames[type]);
}

bool Game_Vehicle::IsInCurrentMap() const {
	return GetMapId() == Game_Map::GetMapId();
}

const lcf::rpg::Music& Game_Vehicle::GetBGM() {
	switch (GetVehicleType()) {
	case None:
		assert(false);
		break;
	case Boat:
		return Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_Boat);
	case Ship:
		return Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_Ship);
	case Airship:
		return Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_Airship);
	}

	static lcf::rpg::Music empty;
	return empty;
}

void Game_Vehicle::StartDescent() {
	if (IsFlying()) {
		SetFacing(Left);
		data()->remaining_descent = SCREEN_TILE_SIZE;
	}
}

void Game_Vehicle::StartAscent() {
	if (!IsFlying()) {
		data()->remaining_ascent = SCREEN_TILE_SIZE;
		SetFlying(true);
	}
}

bool Game_Vehicle::IsInUse() const {
	return Main_Data::game_player->GetVehicle() == this;
}

bool Game_Vehicle::IsAboard() const {
	return IsInUse() && Main_Data::game_player->IsAboard();
}

void Game_Vehicle::SyncWithRider(const Game_Character* rider) {
	SetProcessed(true);
	SetMapId(rider->GetMapId());
	SetX(rider->GetX());
	SetY(rider->GetY());
	SetDirection(rider->GetDirection());
	SetFacing(rider->GetFacing());
	SetRemainingStep(rider->GetRemainingStep());

	// RPG_RT doesn't copy jumping chunks

	UpdateAnimation();
	CancelMoveRoute();
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

int Game_Vehicle::GetScreenY(bool apply_shift, bool apply_jump) const {
	return Game_Character::GetScreenY(apply_shift, apply_jump) - GetAltitude();
}

bool Game_Vehicle::CanLand() const {
	return Game_Map::CanLandAirship(GetX(), GetY());
}

void Game_Vehicle::UpdateNextMovementAction() {
	UpdateMoveRoute(data()->move_route_index, data()->move_route, true);
}

void Game_Vehicle::UpdateAnimation() {
	if (!IsJumping() && (GetVehicleType() != Airship || IsFlying())) {
		// RPG_RT Animates vehicles slower when moving
		const auto limit = GetStopCount() ? 16 : 12;

		IncAnimCount();

		if (GetAnimCount() >= limit) {
			IncAnimFrame();
		}
	} else {
		ResetAnimation();
	}
}

bool Game_Vehicle::AnimateAscentDescent() {
	if (IsAscending()) {
		data()->remaining_ascent = data()->remaining_ascent - 8;
		return true;
	} else if (IsDescending()) {
		data()->remaining_descent = data()->remaining_descent - 8;
		if (!IsDescending()) {
			SetFlying(false);
			if (CanLand()) {
				SetDefaultDirection();
			} else {
				StartAscent();
			}
		}
		return true;
	}
	return false;
}

void Game_Vehicle::ForceLand() {
	data()->remaining_descent = 0;
	data()->remaining_ascent = 0;
	data()->flying = 0;
}

void Game_Vehicle::Update() {
	Game_Character::Update();
}

StringView Game_Vehicle::GetOrigSpriteName() const {
	if (!data()->orig_sprite_name.empty()) {
		return data()->orig_sprite_name;
	}
	switch (GetVehicleType()) {
		case Boat:
			return lcf::Data::system.boat_name;
		case Ship:
			return lcf::Data::system.ship_name;
		case Airship:
			return lcf::Data::system.airship_name;
		default:
			break;
	}
	return {};
}

int Game_Vehicle::GetOrigSpriteIndex() const {
	if (!data()->orig_sprite_name.empty()) {
		return data()->orig_sprite_id;
	}
	switch (GetVehicleType()) {
		case Boat:
			return lcf::Data::system.boat_index;
		case Ship:
			return lcf::Data::system.ship_index;
		case Airship:
			return lcf::Data::system.airship_index;
		default:
			break;
	}
	return 0;
}
