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
#include "game_character.h"
#include "game_map.h"
#include "main_data.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////
Game_Character::Game_Character() {
	x = 0;
	y = 0;
	real_x = 0;
	real_y = 0;
	move_speed = 4;

	prelock_direction = 0;
}

////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////
Game_Character::~Game_Character() {
}

////////////////////////////////////////////////////////////
// IsMoving
////////////////////////////////////////////////////////////
bool Game_Character::IsMoving() {
	return (real_x != x * 128 || real_y != y * 128);
}

////////////////////////////////////////////////////////////
// IsPassable
////////////////////////////////////////////////////////////
bool Game_Character::IsPassable(int x, int y, int d) {
	/*int new_x = x + (d == 6 ? 1 : d == 4 ? -1 : 0);
	int new_y = y + (d == 2 ? 1 : d == 8 ? -1 : 0);

	if (!Main_Data::game_map->IsValid(new_x, new_y)) return false;

	if (through) return true;

	if (!Main_Data::game_map->IsPassable(x, y, d, this)) return false;
	
	if (!Main_Data::game_map->IsPassable(new_x, new_y, 10 - d)) return false;
	
	for (int i = 0; i < Main_Data::game_map->events.size(); i++) {
		Game_Event* evnt = Main_Data::game_map->events[i];
		if (evnt.x == new_x && evnt.y == new_y) {
			if (!evnt.through) {
				if (id != Main_Data::game_player.id)
					return false;
				if (!evnt.character_name.empty())
					return false;
			}
		}
	}

	if (Main_Data::game_player->x == new_x && Main_Data::game_player->y == new_y) {
		if (!Main_Data::game_player->through && !character_name.empty()) {
			return false;
		}
	}*/

	return true;
}

////////////////////////////////////////////////////////////
// MoveTo
////////////////////////////////////////////////////////////
void Game_Character::MoveTo(int x, int y) {
	this->x = x % Main_Data::game_map->GetWidth();
	this->y = y % Main_Data::game_map->GetHeight();
	real_x = x * 128;
	real_y = y * 128;
	prelock_direction = 0;
}

////////////////////////////////////////////////////////////
// GetScreenX
////////////////////////////////////////////////////////////
int Game_Character::GetScreenX() {
	return (real_x - Main_Data::game_map->display_x + 3) / 4 + 16;
}

////////////////////////////////////////////////////////////
// GetScreenY
////////////////////////////////////////////////////////////
int Game_Character::GetScreenY() {
	int y = (real_y - Main_Data::game_map->display_y + 3) / 4 + 32;

	/*int n;
	if (jump_count >= jump_peak)
		n = jump_count - jump_peak;
	else
		n = jump_peak - jump_count;*/

	return y; /*- (jump_peak * jump_peak - n * n) / 2;*/
}

////////////////////////////////////////////////////////////
// GetScreenZ
////////////////////////////////////////////////////////////
int Game_Character::GetScreenZ() {
	return GetScreenZ(0);
}
int Game_Character::GetScreenZ(int height) {
	/*if (always_on_top) return 999;*/
	
	int z = (real_y - Main_Data::game_map->display_y + 3) / 4 + 32;

	/*if (tile_id > 0)
		return z + Main_Data::game_map->priorities[tile_id] * 16
	else*/
		return z + ((height > 16) ? 15 : 0);
}

////////////////////////////////////////////////////////////
// Update
////////////////////////////////////////////////////////////
void Game_Character::Update() {
	/*if (IsJumping())
		UpdateJump();
	else*/ if (IsMoving())
		UpdateMove();
	/*else
		UpdateStop();

	if (anime_count > 18 - move_speed * 2) {
		if (!step_anime && stop_count > 0)
			pattern = original_pattern;
		else
			pattern = (pattern + 1) % 4;

		anime_count = 0;
	}
	if (wait_count > 0) {
		wait_count -= 1
		return;
	}
	if (move_route_forcing) {
		MoveTypeCustom();
		return;
	}
	if (starting || IsLock()) return;
	
	if (stop_count > (40 - move_frequency * 2) * (6 - move_frequency)) {
		switch (move_type) {
			case 1: // Random
				MoveTypeRandom();
				break;
			case 2: // Approach
				MoveTypeTowardPlayer();
				break;
			case 3: // Custom
				MoveTypeCustom();
		}
	}*/
}

////////////////////////////////////////////////////////////
// UpdateMove
////////////////////////////////////////////////////////////
void Game_Character::UpdateMove() {
	int distance = 2 ^ move_speed;
	if (y * 128 > real_y)
		real_y = min(real_y + distance, y * 128);

	if (x * 128 < real_x)
		real_x = max(real_x - distance, x * 128);

	if (x * 128 > real_x)
		real_x = min(real_x + distance, x * 128);

	if (y * 128 < real_y)
		real_y = max(real_y - distance, y * 128);

	/*if (walk_anime)
		anime_count += 1.5;
	else if (step_anime)
		anime_count += 1;*/
}

////////////////////////////////////////////////////////////
// UpdateMove
////////////////////////////////////////////////////////////
void Game_Character::MoveDown() {
	//if (turn_enabled) TurnDown();

	if (IsPassable(x, y, 2)) {
		//TurnDown()
		y += 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x, y + 1);
	}*/
}

////////////////////////////////////////////////////////////
// UpdateMove
////////////////////////////////////////////////////////////
void Game_Character::MoveLeft() {
	//if (turn_enabled) TurnLeft();

	if (IsPassable(x, y, 4)) {
		//TurnLeft()
		x -= 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x - 1, y);
	}*/
}

////////////////////////////////////////////////////////////
// UpdateMove
////////////////////////////////////////////////////////////
void Game_Character::MoveRight() {
	//if (turn_enabled) TurnRight();

	if (IsPassable(x, y, 6)) {
		//TurnRight()
		x += 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x + 1, y);
	}*/
}

////////////////////////////////////////////////////////////
// UpdateMove
////////////////////////////////////////////////////////////
void Game_Character::MoveUp() {
	//if (turn_enabled) TurnUp();
	if (IsPassable(x, y, 8)) {
		//TurnUp()
		y -= 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x, y - 1);
	}*/
}


void Game_Character::Lock() {
	//TODO

}