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
#include "audio.h"
#include "game_character.h"
#include "game_map.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_system.h"
#include "lmu_chunks.h"
#include "main_data.h"
#include "util_macro.h"
#include <cassert>
#include <cstdlib>

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
	original_direction(2),
	original_pattern(1),
	last_pattern(0),
	move_route_forcing(false),
	through(false),
	animation_id(0),
	move_route(NULL),
	original_move_route(NULL),
	move_route_index(0),
	original_move_route_index(0),
	move_type(0),
	move_speed(4),
	move_frequency(6),
	prelock_direction(0),
	move_failed(false),
	locked(false),
	wait_count(0),
	anime_count(0),
	stop_count(0),
	jump_count(0),	
	step_anime(false),
	walk_anime(true),
	turn_enabled(true),
	direction_fix(false),
	priority_type(1),
	transparent(false) {
}

////////////////////////////////////////////////////////////
bool Game_Character::IsMoving() const {
	return real_x != x * 128 || real_y != y * 128;
}

bool Game_Character::IsJumping() const {
	return jump_count > 0;
}

bool Game_Character::IsStopping() const {
	return !(IsMoving() || IsJumping());
}

////////////////////////////////////////////////////////////
bool Game_Character::IsPassable(int x, int y, int d) const {
	int new_x = x + (d == DirectionRight ? 1 : d == DirectionLeft ? -1 : 0);
	int new_y = y + (d == DirectionDown ? 1 : d == DirectionUp ? -1 : 0);

	if (!Game_Map::IsValid(new_x, new_y)) return false;

	if (through) return true;

	if (!Game_Map::IsPassable(x, y, d, this)) return false;
	
	if (!Game_Map::IsPassable(new_x, new_y, 10 - d)) return false;
	
	for (tEventHash::iterator i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); i++) {
		Game_Event* evnt = i->second;
		if (evnt->GetX() == new_x && evnt->GetY() == new_y) {
			if (!evnt->GetThrough() && evnt->GetPriorityType() == 1) {
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

int Game_Character::GetPriorityType() const {
	return priority_type;
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
	}*/

	if (move_route_forcing) {
		MoveTypeCustom();
	} else if (!locked) {
		UpdateSelfMovement();
	}

	/*
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
	int distance = (1 << move_speed);
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
void Game_Character::UpdateSelfMovement() {
	if (stop_count > 30 * (5 - move_frequency)) {
		switch (move_type) {
		case 1: // Random
			break;
		case 2: // Cycle up-down
			break;
		case 3: // Cycle left-right
			break;
		case 4: // Step towards hero
			break;
		case 5: // Step away from hero
			break;
		case 6: // Custom route
			MoveTypeCustom();
			break;
		}
	}
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
void Game_Character::MoveTypeCustom() {
	if (IsStopping()) {
		move_failed = false;
		if ((size_t)move_route_index >= move_route->move_commands.size()) {
			// End of Move list
			if (move_route->repeat) {
				move_route_index = 0;
			} else if (move_route_forcing) {
				move_route_forcing = false;
				move_route = original_move_route;
				move_route_index = original_move_route_index;
				original_move_route = NULL;
				move_route_owner->EndMoveRoute(move_route);
			}
		} else {
			RPG::MoveCommand& move_command = move_route->move_commands[move_route_index];
			switch (move_command.command_id) {
			case LMU_Reader::ChunkMoveCommands::move_up:
				MoveUp();	break;
			case LMU_Reader::ChunkMoveCommands::move_right:
				MoveRight(); break;
			case LMU_Reader::ChunkMoveCommands::move_down:
				MoveDown(); break;
			case LMU_Reader::ChunkMoveCommands::move_left:
				MoveLeft(); break;
			case LMU_Reader::ChunkMoveCommands::move_upright: break;
			case LMU_Reader::ChunkMoveCommands::move_downright: break;
			case LMU_Reader::ChunkMoveCommands::move_downleft: break;
			case LMU_Reader::ChunkMoveCommands::move_upleft: break;
			case LMU_Reader::ChunkMoveCommands::move_random: break;
			case LMU_Reader::ChunkMoveCommands::move_towards_hero: break;
			case LMU_Reader::ChunkMoveCommands::move_away_from_hero: break;
			case LMU_Reader::ChunkMoveCommands::move_forward: break;
			case LMU_Reader::ChunkMoveCommands::face_up:
				TurnUp(); break;
			case LMU_Reader::ChunkMoveCommands::face_right:
				TurnRight(); break;
			case LMU_Reader::ChunkMoveCommands::face_down:
				TurnDown(); break;
			case LMU_Reader::ChunkMoveCommands::face_left:
				TurnLeft(); break;
			case LMU_Reader::ChunkMoveCommands::turn_90_degree_right: break;
			case LMU_Reader::ChunkMoveCommands::turn_90_degree_left: break;
			case LMU_Reader::ChunkMoveCommands::turn_180_degree: break;
			case LMU_Reader::ChunkMoveCommands::turn_90_degree_random: break;
			case LMU_Reader::ChunkMoveCommands::face_random_direction: break;
			case LMU_Reader::ChunkMoveCommands::face_hero: break;
			case LMU_Reader::ChunkMoveCommands::face_away_from_hero: break;
			case LMU_Reader::ChunkMoveCommands::wait: break;
			case LMU_Reader::ChunkMoveCommands::begin_jump: break;
			case LMU_Reader::ChunkMoveCommands::end_jump: break;
			case LMU_Reader::ChunkMoveCommands::lock_facing: break;
			case LMU_Reader::ChunkMoveCommands::unlock_facing: break;
			case LMU_Reader::ChunkMoveCommands::increase_movement_speed:
				move_speed = min(move_speed + 1, 8); break;
			case LMU_Reader::ChunkMoveCommands::decrease_movement_speed: 
				move_speed = max(move_speed - 1, 1); break;
			case LMU_Reader::ChunkMoveCommands::increase_movement_frequence: break;
			case LMU_Reader::ChunkMoveCommands::decrease_movement_frequence: break;
			case LMU_Reader::ChunkMoveCommands::switch_on: // Parameter A: Switch to turn on
				Game_Switches[move_command.parameter_a] = true;
				Game_Map::SetNeedRefresh(true);
				break;
			case LMU_Reader::ChunkMoveCommands::switch_off: // Parameter A: Switch to turn off
				Game_Switches[move_command.parameter_a] = false;
				Game_Map::SetNeedRefresh(true);
				break;
			case LMU_Reader::ChunkMoveCommands::change_graphic: break; // String: File, Parameter A: index
			case LMU_Reader::ChunkMoveCommands::play_sound_effect: // String: File, Parameters: Volume, Tempo, Balance
				if (move_command.parameter_string != "(OFF)") {
					Audio::SE_Play("Sound/" + move_command.parameter_string,
						move_command.parameter_a, move_command.parameter_b);
				}
				break;
			case LMU_Reader::ChunkMoveCommands::walk_everywhere_on:
				through = true; break;
			case LMU_Reader::ChunkMoveCommands::walk_everywhere_off:
				through = false; break;
			case LMU_Reader::ChunkMoveCommands::stop_animation: break;
			case LMU_Reader::ChunkMoveCommands::start_animation: break;
			case LMU_Reader::ChunkMoveCommands::increase_transp: break; // ???
			case LMU_Reader::ChunkMoveCommands::decrease_transp: break; // ???
			}

			if (move_route->skippable || !move_failed) {
				++move_route_index;
			}
		}
	}
}

////////////////////////////////////////////////////////////
void Game_Character::MoveDown() {
	if (turn_enabled) TurnDown();

	if (IsPassable(x, y, DirectionDown)) {
		TurnDown();
		y += 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x, y + 1);
		move_failed = true;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::MoveLeft() {
	if (turn_enabled) TurnLeft();

	if (IsPassable(x, y, DirectionLeft)) {
		TurnLeft();
		x -= 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x - 1, y);
		move_failed = true;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::MoveRight() {
	if (turn_enabled) TurnRight();

	if (IsPassable(x, y, DirectionRight)) {
		TurnRight();
		x += 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x + 1, y);
		move_failed = true;
	}
}

////////////////////////////////////////////////////////////
void Game_Character::MoveUp() {
	if (turn_enabled) TurnUp();

	if (IsPassable(x, y, DirectionUp)) {
		TurnUp();
		y -= 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x, y - 1);
		move_failed = true;
	}
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

void Game_Character::TurnTowardPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		(sx > 0) ? TurnLeft() : TurnRight();
	} 
	else if ( std::abs(sx) < std::abs(sy) ) {
		(sy > 0) ? TurnUp() : TurnDown();
	}
}

int Game_Character::DistanceXfromPlayer() const {
	int sx = x - Main_Data::game_player->x;
	if (Game_Map::LoopHorizontal()) {
		if (std::abs(sx) > Game_Map::GetWidth() / 2) {
			sx -= Game_Map::GetWidth();
		}
	}
	return sx;
}

int Game_Character::DistanceYfromPlayer() const {
	int sy = y - Main_Data::game_player->y;
	if (Game_Map::LoopVertical()) {
		if (std::abs(sy) > Game_Map::GetHeight() / 2) {
			sy -= Game_Map::GetHeight();
		}
	}
	return sy;
}

////////////////////////////////////////////////////////////
void Game_Character::Lock() {
	if (!locked) {
		prelock_direction = direction;
		TurnTowardPlayer();
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
void Game_Character::ForceMoveRoute(RPG::MoveRoute* new_route,
									int frequency,
									Game_Interpreter* owner) {
	if (original_move_route == NULL) {
		original_move_route = move_route;
		original_move_route_index = move_route_index;
		original_move_frequency = move_frequency;
	}
	move_route = new_route;
	move_route_index = 0;
	move_route_forcing = true;
	move_frequency = frequency;
	move_route_owner = owner;
	prelock_direction = 0;
	wait_count = 0;
	MoveTypeCustom();
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
	return ((this->x == x) && (this->y == y));
}

bool Game_Character::IsTransparent() const {
	return transparent;
}

void Game_Character::UpdateBushDepth() {
	// TODO
}

void Game_Character::SetGraphic(const std::string& name, int index) {
	character_name = name;
	character_index = index;
}

