/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "data.h"
#include "main_data.h"
#include "game_system.h"
#include "game_map.h"
#include "game_player.h"
#include "game_vehicle.h"

////////////////////////////////////////////////////////////
Game_Vehicle::Game_Vehicle(Type _type) {
	type = _type;
	altitude = 0;
	driving = false;
	direction = 4;
	walk_anime = false;
	step_anime = false;
	LoadSystemSettings();
}

////////////////////////////////////////////////////////////
void Game_Vehicle::LoadSystemSettings() {
	switch (type) {
		case Boat:
			character_name = Data::system.boat_name;
			character_index = Data::system.boat_index;
			bgm = Data::system.boat_music;
			map_id = Data::treemap.start.boat_map_id;
			x = Data::treemap.start.boat_x;
			y = Data::treemap.start.boat_y;
			break;
		case Ship:
			character_name = Data::system.ship_name;
			character_index = Data::system.ship_index;
			bgm = Data::system.ship_music;
			map_id = Data::treemap.start.ship_map_id;
			x = Data::treemap.start.ship_x;
			y = Data::treemap.start.ship_y;
			break;
		case Airship:
			character_name = Data::system.airship_name;
			character_index = Data::system.airship_index;
			bgm = Data::system.airship_music;
			map_id = Data::treemap.start.airship_map_id;
			x = Data::treemap.start.airship_x;
			y = Data::treemap.start.airship_y;
			break;
	}
}

////////////////////////////////////////////////////////////
void Game_Vehicle::Refresh() {
	if (driving) {
		map_id = Game_Map::GetMapId();
		SyncWithPlayer();
	}
	else if (map_id == Game_Map::GetMapId())
		MoveTo(x, y);
	switch (type) {
		case Boat:
			priority_type = 1;
			move_speed = 4;
			break;
		case Ship:
			priority_type = 1;
			move_speed = 5;
			break;
		case Airship:
			priority_type = driving ? 2 : 0;
			move_speed = 6;
			break;
	}
	walk_anime = driving;
	step_anime = driving;
}

////////////////////////////////////////////////////////////
void Game_Vehicle::SetPosition(int _map_id, int _x, int _y) {
	map_id = _map_id;
	x = _x;
	y = _y;
}

bool Game_Vehicle::IsInPosition(int x, int y) const {
	return map_id == Game_Map::GetMapId() && Game_Character::IsInPosition(x, y);
}

bool Game_Vehicle::IsTransparent() const {
	return map_id != Game_Map::GetMapId() || Game_Character::IsTransparent();
}

////////////////////////////////////////////////////////////
void Game_Vehicle::GetOn() {
	driving = true;
	walk_anime = true;
	step_anime = true;
	if (type == Airship)
		priority_type = 2;		// Change priority to "Above Characters"
	Game_System::BgmPlay(bgm);
}

void Game_Vehicle::GetOff() {
	driving = false;
	walk_anime = false;
	step_anime = false;
	direction = 4;
}

void Game_Vehicle::SyncWithPlayer() {
	x = Main_Data::game_player->GetX();
	y = Main_Data::game_player->GetY();
	real_x = Main_Data::game_player->GetRealX();
	real_y = Main_Data::game_player->GetRealY();
	direction = Main_Data::game_player->GetDirection();
	UpdateBushDepth();
}

int Game_Vehicle::GetSpeed() const {
	return move_speed;
}

int Game_Vehicle::GetScreenY() const {
	return Game_Character::GetScreenY() - altitude;
}

bool Game_Vehicle::IsMovable() {
	if (type == Airship && altitude < MAX_ALTITUDE)
		return false;
	return !IsMoving();
}

////////////////////////////////////////////////////////////
void Game_Vehicle::Update() {
	Game_Character::Update();
	if (type == Airship) {
		if (driving) {
			if (altitude < MAX_ALTITUDE)
				altitude++;
		}
		else if (altitude > 0) {
			altitude--;
			if (altitude == 0)
				priority_type = 0;    // Return priority to "Below Characters"
		}
	}
}

bool Game_Vehicle::CheckEventTriggerTouch(int x, int y) {
	return false;
}

