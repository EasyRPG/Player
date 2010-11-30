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
#include "game_player.h"
#include "main_data.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
Game_Character::Game_Character() :
	x(0),
	y(0),
	tile_id(0),
	character_index(0),
	real_x(0),
	real_y(0),
	direction(2),
	pattern(1),
	move_route_forcing(false),
	through(false),
	animation_id(0),
	move_speed(4),
	move_frequency(6),
	prelock_direction(0),
	locked(false),
	anime_count(0),
	stop_count(0),
	jump_count(0),
	original_pattern(1),
	last_pattern(0),
	step_anime(false),
	walk_anime(true),
	turn_enabled(true),
	direction_fix(false) {
}

////////////////////////////////////////////////////////////
bool Game_Character::IsMoving() const {
	return real_x != x * 128 || real_y != y * 128;
}

////////////////////////////////////////////////////////////
bool Game_Character::IsPassable(int x, int y, int d) const {
	int new_x = x + (d == 6 ? 1 : d == 4 ? -1 : 0);
	int new_y = y + (d == 2 ? 1 : d == 8 ? -1 : 0);

	if (!Game_Map::IsValid(new_x, new_y)) return false;

	if (through) return true;

	if (!Game_Map::IsPassable(x, y, d, this)) return false;
	
	if (!Game_Map::IsPassable(new_x, new_y, 10 - d)) return false;
	
	for (tEventHash::iterator i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); i++) {
		Game_Event* evnt = i->second;
		if (evnt->GetX() == new_x && evnt->GetY() == new_y) {
			if (!evnt->GetThrough()) {
				if (this != (const Game_Character*)Main_Data::game_player)
					return false;

				if (!evnt->GetCharacterName().empty())
					return false;
			}
		}
	}

	if (Main_Data::game_player->GetX() == new_x && Main_Data::game_player->GetY() == new_y) {
		if (!Main_Data::game_player->GetThrough() && !character_name.empty()) {
			return false;
		}
	}

	return true;
}

bool Game_Character::IsJumping() const {
	return jump_count > 0;
}

////////////////////////////////////////////////////////////
void Game_Character::MoveTo(int x, int y) {
	this->x = x % Game_Map::GetWidth();
	this->y = y % Game_Map::GetHeight();
	real_x = x * 128;
	real_y = y * 128;
	prelock_direction = 0;
}

////////////////////////////////////////////////////////////
int Game_Character::GetScreenX() const {
	return (real_x - Game_Map::GetDisplayX() + 3) / 8 + 8;
}

////////////////////////////////////////////////////////////
int Game_Character::GetScreenY() const {
	int y = (real_y - Game_Map::GetDisplayY() + 3) / 8 + 16;

	/*int n;
	if (jump_count >= jump_peak)
		n = jump_count - jump_peak;
	else
		n = jump_peak - jump_count;*/

	return y; /*- (jump_peak * jump_peak - n * n) / 2;*/
}

////////////////////////////////////////////////////////////
int Game_Character::GetScreenZ() const {
	return GetScreenZ(0);
}
int Game_Character::GetScreenZ(int height) const {
	/*if (always_on_top) return 999;*/
	
	int z = (real_y - Game_Map::GetDisplayY() + 3) / 8 + 16;

	/*if (tile_id > 0)
		return z + Game_Map::priorities[tile_id] * 16
	else*/
		return z + ((height > 16) ? 15 : 0);
}

////////////////////////////////////////////////////////////
void Game_Character::Update() {
	/*if (IsJumping())
		UpdateJump();
	else*/ if (IsMoving())
		UpdateMove();
	else
		UpdateStop();

	if (anime_count > 24 - move_speed) {
		if (!step_anime && stop_count > 0) {
			pattern = original_pattern;
			last_pattern = last_pattern == 0 ? 2 : 0;
		} else {
			if (last_pattern == 0) {
				if (pattern == 2) {
					pattern = 1;
					last_pattern = 2;
				} else {
					pattern = 2;
				}
			} else {
				if (pattern == 0) {
					pattern = 1;
					last_pattern = 0;
				} else {
					pattern = 0;
				}
			}
		}

		anime_count = 0;
	}

	/*if (wait_count > 0) {
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
void Game_Character::UpdateMove() {
	int distance = (1 << move_speed) / 2;
	if (y * 128 > real_y)
		real_y = min(real_y + distance, y * 128);

	if (x * 128 < real_x)
		real_x = max(real_x - distance, x * 128);

	if (x * 128 > real_x)
		real_x = min(real_x + distance, x * 128);

	if (y * 128 < real_y)
		real_y = max(real_y - distance, y * 128);

	if (walk_anime)
		anime_count += 1.5;
	else if (step_anime)
		anime_count += 1;
}

////////////////////////////////////////////////////////////
void Game_Character::UpdateStop() {
	if (step_anime)
		anime_count += 1;
	else if (pattern != original_pattern)
		anime_count += 1.5;

	//if (!starting || !IsLock())
		stop_count += 1;
}

////////////////////////////////////////////////////////////
void Game_Character::MoveDown() {
	if (turn_enabled) TurnDown();

	if (IsPassable(x, y, 2)) {
		TurnDown();
		y += 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x, y + 1);
	}*/
}

////////////////////////////////////////////////////////////
void Game_Character::MoveLeft() {
	if (turn_enabled) TurnLeft();

	if (IsPassable(x, y, 4)) {
		TurnLeft();
		x -= 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x - 1, y);
	}*/
}

////////////////////////////////////////////////////////////
void Game_Character::MoveRight() {
	if (turn_enabled) TurnRight();

	if (IsPassable(x, y, 6)) {
		TurnRight();
		x += 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x + 1, y);
	}*/
}

////////////////////////////////////////////////////////////
void Game_Character::MoveUp() {
	if (turn_enabled) TurnUp();

	if (IsPassable(x, y, 8)) {
		TurnUp();
		y -= 1;
		//IncreaseSteps();
	} /*else {
		check_event_trigger_touch(x, y - 1);
	}*/
}
////////////////////////////////////////////////////////////
void Game_Character::TurnDown() {
	if (!direction_fix) {
		direction = 2;
		stop_count = 0;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::TurnLeft() {
	if (!direction_fix) {
		direction = 4;
		stop_count = 0;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::TurnRight() {
	if (!direction_fix) {
		direction = 6;
		stop_count = 0;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::TurnUp() {
	if (!direction_fix) {
		direction = 8;
		stop_count = 0;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::Lock() {
	if (!locked) {
		prelock_direction = direction;
		// TODO
		//TurnTowardPlayer();
		locked = true;
	}
}

void Game_Character::Unlock() {
	if (locked) {
		locked = false;
		SetDirection(prelock_direction);
	}
}

void Game_Character::SetDirection(int direction) {
	if ((!direction_fix) && (direction != 0)) {
		this->direction = direction;
		stop_count = 0;
	}
}

////////////////////////////////////////////////////////////
int Game_Character::GetX() const {
	return x;
}

int Game_Character::GetY() const {
	return y;
}

int Game_Character::GetTileId() const {
	return tile_id;
}

std::string Game_Character::GetCharacterName() const {
	return character_name;
}

int Game_Character::GetCharacterIndex() const {
	return character_index;
}

int Game_Character::GetRealX() const {
	return real_x;
}

int Game_Character::GetRealY() const {
	return real_y;
}

int Game_Character::GetDirection() const {
	return direction;
}

int Game_Character::GetPattern() const {
	return pattern;
}

bool Game_Character::GetMoveRouteForcing() const {
	return move_route_forcing;
}

bool Game_Character::GetThrough() const {
	return through;
}

int Game_Character::GetAnimationId() const {
	return animation_id;
}
void Game_Character::SetAnimationId(int new_animation_id) {
	animation_id = new_animation_id;
}

bool Game_Character::IsInPosition(int x, int y) const {
	return ((this->x == x) && (this->y && y));
}
