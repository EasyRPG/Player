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
#include "audio.h"
#include "game_character.h"
#include "game_map.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_system.h"
#include "main_data.h"
#include "util_macro.h"
#include <cassert>
#include <cstdlib>

Game_Character::Game_Character() :
	x(0),
	y(0),
	tile_id(0),
	character_index(0),
	real_x(0),
	real_y(0),
	direction(RPG::EventPage::Direction_down),
	pattern(RPG::EventPage::Frame_middle),
	original_direction(RPG::EventPage::Direction_down),
	original_pattern(RPG::EventPage::Frame_middle),
	last_pattern(0),
	move_route_forcing(false),
	through(false),
	animation_id(0),
	move_route(NULL),
	original_move_route(NULL),
	move_route_index(0),
	original_move_route_index(0),
	move_type(RPG::EventPage::MoveType_stationary),
	move_speed(RPG::EventPage::MoveSpeed_normal),
	move_frequency(6),
	prelock_direction(-1),
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
	cycle_stat(false),
	priority_type(RPG::EventPage::Layers_same),
	transparent(false) {
}

bool Game_Character::IsMoving() const {
	return real_x != x * 128 || real_y != y * 128;
}

bool Game_Character::IsJumping() const {
	return jump_count > 0;
}

bool Game_Character::IsStopping() const {
	return !(IsMoving() || IsJumping());
}

bool Game_Character::IsPassable(int x, int y, int d) const {
	int new_x = x + (d == RPG::EventPage::Direction_right ? 1 : d == RPG::EventPage::Direction_left ? -1 : 0);
	int new_y = y + (d == RPG::EventPage::Direction_down ? 1 : d == RPG::EventPage::Direction_up ? -1 : 0);

	if (!Game_Map::IsValid(new_x, new_y))
		return false;

	if (through) return true;

	if (!Game_Map::IsPassable(x, y, d, this))
		return false;

	if (!Game_Map::IsPassable(new_x, new_y, (d + 2) % 4))
		return false;

	for (tEventHash::iterator i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); i++) {
		Game_Event* evnt = i->second.get();
		if (evnt->GetX() == new_x && evnt->GetY() == new_y) {
			if (!evnt->GetThrough() && evnt->GetPriorityType() == RPG::EventPage::Layers_same) {
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

void Game_Character::MoveTo(int x, int y) {
	this->x = x % Game_Map::GetWidth();
	this->y = y % Game_Map::GetHeight();
	real_x = x * 128;
	real_y = y * 128;
	prelock_direction = -1;
}

int Game_Character::GetScreenX() const {
	return (real_x - Game_Map::GetDisplayX() + 3) / 8 + 8;
}

int Game_Character::GetScreenY() const {
	int y = (real_y - Game_Map::GetDisplayY() + 3) / 8 + 16;

	/*int n;
	if (jump_count >= jump_peak)
		n = jump_count - jump_peak;
	else
		n = jump_peak - jump_count;*/

	return y; /*- (jump_peak * jump_peak - n * n) / 2;*/
}

int Game_Character::GetScreenZ() const {
	return GetScreenZ(0);
}

int Game_Character::GetScreenZ(int /* height */) const {
	if (GetPriorityType() == RPG::EventPage::Layers_above) return 999;

	int z = (real_y - Game_Map::GetDisplayY() + 3) / 8 + 16;

	return z;
}

void Game_Character::Update() {
	/*if (IsJumping())
		UpdateJump();
	else*/ if (IsMoving())
		UpdateMove();
	else
		UpdateStop();

	if (anime_count > 18 - move_speed * 2) {
		if (!step_anime && stop_count > 0) {
			pattern = original_pattern;
			last_pattern = last_pattern == RPG::EventPage::Frame_left ? RPG::EventPage::Frame_right : RPG::EventPage::Frame_left;
		} else {
			if (last_pattern == RPG::EventPage::Frame_left) {
				if (pattern == RPG::EventPage::Frame_right) {
					pattern = RPG::EventPage::Frame_middle;
					last_pattern = RPG::EventPage::Frame_right;
				} else {
					pattern = RPG::EventPage::Frame_right;
				}
			} else {
				if (pattern == RPG::EventPage::Frame_left) {
					pattern = RPG::EventPage::Frame_middle;
					last_pattern = RPG::EventPage::Frame_left;
				} else {
					pattern = RPG::EventPage::Frame_left;
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
}

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

void Game_Character::UpdateSelfMovement() {
	if (stop_count > 30 * (5 - move_frequency)) {
		switch (move_type) {
		case RPG::EventPage::MoveType_random:
			MoveTypeRandom();
			break;
		case RPG::EventPage::MoveType_vertical:
			MoveTypeCycleUpDown();
			break;
		case RPG::EventPage::MoveType_horizontal:
			MoveTypeCycleLeftRight();
			break;
		case RPG::EventPage::MoveType_toward:
			MoveTypeTowardsPlayer();
			break;
		case RPG::EventPage::MoveType_away:
			MoveTypeAwayFromPlayer();
			break;
		case RPG::EventPage::MoveType_custom:
			MoveTypeCustom();
			break;
		}
	}
}

void Game_Character::UpdateStop() {
	if (step_anime)
		anime_count += 1;
	else if (pattern != original_pattern)
		anime_count += 1.5;

	//if (!starting || !IsLock())
		stop_count += 1;
}

void Game_Character::MoveTypeRandom() {
	if (IsStopping()) {
		switch (rand() % 6) {
		case 0:
			stop_count = 0;
			break;
		case 1: case 2:
			MoveRandom();
			break;
		default:
			MoveForward();
		}
	}
}

void Game_Character::MoveTypeCycleLeftRight() {
	if (IsStopping()) {
		cycle_stat ? MoveLeft() : MoveRight();

		cycle_stat = move_failed ? !cycle_stat : cycle_stat;
	}
}

void Game_Character::MoveTypeCycleUpDown() {
	if (IsStopping()) {
		cycle_stat ? MoveUp() : MoveDown();

		cycle_stat = move_failed ? !cycle_stat : cycle_stat;
	}
}

void Game_Character::MoveTypeTowardsPlayer() {
	if (IsStopping()) {
		int sx = x - Main_Data::game_player->GetX();
		int sy = y - Main_Data::game_player->GetY();

		if ( std::abs(sx) + std::abs(sy) >= 20 ) {
			MoveRandom();
		} else {
			switch (rand() % 6) {
			case 0:
				MoveRandom();
				break;
			case 1:
				MoveForward();
				break;
			default:
				MoveTowardsPlayer();
			}
		}
	}
}

void Game_Character::MoveTypeAwayFromPlayer() {
	if (IsStopping()) {
		int sx = x - Main_Data::game_player->GetX();
		int sy = y - Main_Data::game_player->GetY();

		if ( std::abs(sx) + std::abs(sy) >= 20 ) {
			MoveRandom();
		} else {
			switch (rand() % 6) {
			case 0:
				MoveRandom();
				break;
			case 1:
				MoveForward();
				break;
			default:
				MoveAwayFromPlayer();
			}
		}
	}
}

void Game_Character::MoveTypeCustom() {
	if (IsStopping()) {
		move_failed = false;
		if ((size_t)move_route_index >= move_route->move_commands.size()) {
			// End of Move list
			if (move_route->repeat) {
				move_route_index = 0;
			} else if (move_route_forcing) {
				move_route_forcing = false;
				if (move_route_owner != NULL) {
					move_route_owner->EndMoveRoute(move_route);
				}
				move_route = original_move_route;
				move_route_index = original_move_route_index;
				original_move_route = NULL;
			}
		} else {
			RPG::MoveCommand& move_command = move_route->move_commands[move_route_index];
			switch (move_command.command_id) {
			case RPG::MoveCommand::Code::move_up:
				MoveUp();
				break;
			case RPG::MoveCommand::Code::move_right:
				MoveRight();
				break;
			case RPG::MoveCommand::Code::move_down:
				MoveDown();
				break;
			case RPG::MoveCommand::Code::move_left:
				MoveLeft();
				break;
			case RPG::MoveCommand::Code::move_upright:
				MoveUpRight();
				break;
			case RPG::MoveCommand::Code::move_downright:
				MoveDownRight();
				break;
			case RPG::MoveCommand::Code::move_downleft:
				MoveDownLeft();
				break;
			case RPG::MoveCommand::Code::move_upleft:
				MoveUpLeft();
				break;
			case RPG::MoveCommand::Code::move_random:
				MoveRandom();
				break;
			case RPG::MoveCommand::Code::move_towards_hero:
				MoveTowardsPlayer();
				break;
			case RPG::MoveCommand::Code::move_away_from_hero:
				MoveAwayFromPlayer();
				break;
			case RPG::MoveCommand::Code::move_forward:
				MoveForward();
				break;
			case RPG::MoveCommand::Code::face_up:
				TurnUp();
				break;
			case RPG::MoveCommand::Code::face_right:
				TurnRight();
				break;
			case RPG::MoveCommand::Code::face_down:
				TurnDown();
				break;
			case RPG::MoveCommand::Code::face_left:
				TurnLeft();
				break;
			case RPG::MoveCommand::Code::turn_90_degree_right:
				Turn90DegreeRight();
				break;
			case RPG::MoveCommand::Code::turn_90_degree_left:
				Turn90DegreeLeft();
				break;
			case RPG::MoveCommand::Code::turn_180_degree:
				Turn180Degree();
				break;
			case RPG::MoveCommand::Code::turn_90_degree_random:
				Turn90DegreeLeftOrRight();
				break;
			case RPG::MoveCommand::Code::face_random_direction:
				FaceRandomDirection();
				break;
			case RPG::MoveCommand::Code::face_hero:
				FaceTowardsHero();
				break;
			case RPG::MoveCommand::Code::face_away_from_hero:
				FaceAwayFromHero();
				break;
			case RPG::MoveCommand::Code::wait:
				Wait();
				break;
			case RPG::MoveCommand::Code::begin_jump:
				BeginJump();
				break;
			case RPG::MoveCommand::Code::end_jump:
				EndJump();
				break;
			case RPG::MoveCommand::Code::lock_facing:
				Lock();
				break;
			case RPG::MoveCommand::Code::unlock_facing:
				Unlock();
				break;
			case RPG::MoveCommand::Code::increase_movement_speed:
				move_speed = min(move_speed + 1, 6);
				break;
			case RPG::MoveCommand::Code::decrease_movement_speed:
				move_speed = max(move_speed - 1, 1);
				break;
			case RPG::MoveCommand::Code::increase_movement_frequence:
				move_frequency = min(move_frequency - 1, 1);
				break;
			case RPG::MoveCommand::Code::decrease_movement_frequence:
				move_frequency = max(move_frequency - 1, 1);
				break;
			case RPG::MoveCommand::Code::switch_on: // Parameter A: Switch to turn on
				Game_Switches[move_command.parameter_a] = true;
				Game_Map::SetNeedRefresh(true);
				break;
			case RPG::MoveCommand::Code::switch_off: // Parameter A: Switch to turn off
				Game_Switches[move_command.parameter_a] = false;
				Game_Map::SetNeedRefresh(true);
				break;
			case RPG::MoveCommand::Code::change_graphic: // String: File, Parameter A: index
				character_name = move_command.parameter_string;
				character_index = move_command.parameter_a;
				break;
			case RPG::MoveCommand::Code::play_sound_effect: // String: File, Parameters: Volume, Tempo, Balance
				if (move_command.parameter_string != "(OFF)") {
					Audio().SE_Play(move_command.parameter_string,
						move_command.parameter_a, move_command.parameter_b);
				}
				break;
			case RPG::MoveCommand::Code::walk_everywhere_on:
				through = true;
				break;
			case RPG::MoveCommand::Code::walk_everywhere_off:
				through = false;
				break;
			case RPG::MoveCommand::Code::stop_animation:
				// walk_animation = false;
				break;
			case RPG::MoveCommand::Code::start_animation:
				// walk_animation = true;
				break;
			case RPG::MoveCommand::Code::increase_transp:
				transparent = true;
				break;
			case RPG::MoveCommand::Code::decrease_transp:
				transparent = false;
				break;
			}

			if (move_route->skippable || !move_failed) {
				++move_route_index;
			}
		}
	}
}

void Game_Character::MoveDown() {
	if (turn_enabled) TurnDown();

	if (IsPassable(x, y, RPG::EventPage::Direction_down)) {
		TurnDown();
		y += 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x, y + 1);
		move_failed = true;
	}
}

void Game_Character::MoveLeft() {
	if (turn_enabled) TurnLeft();

	if (IsPassable(x, y, RPG::EventPage::Direction_left)) {
		TurnLeft();
		x -= 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x - 1, y);
		move_failed = true;
	}
}

void Game_Character::MoveRight() {
	if (turn_enabled) TurnRight();

	if (IsPassable(x, y, RPG::EventPage::Direction_right)) {
		TurnRight();
		x += 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x + 1, y);
		move_failed = true;
	}
}

void Game_Character::MoveUp() {
	if (turn_enabled) TurnUp();

	if (IsPassable(x, y, RPG::EventPage::Direction_up)) {
		TurnUp();
		y -= 1;
		//IncreaseSteps();
		move_failed = false;
	} else {
		CheckEventTriggerTouch(x, y - 1);
		move_failed = true;
	}
}

void Game_Character::MoveForward() {
	switch (direction) {
	case RPG::EventPage::Direction_down:
		MoveDown();
		break;
	case RPG::EventPage::Direction_left:
		MoveLeft();
		break;
	case RPG::EventPage::Direction_right:
		MoveRight();
		break;
	case RPG::EventPage::Direction_up:
		MoveUp();
		break;
	}
}

void Game_Character::MoveDownLeft() {

}

void Game_Character::MoveDownRight() {
}


void Game_Character::MoveUpLeft() {
}


void Game_Character::MoveUpRight() {
}


void Game_Character::MoveRandom() {
	switch (rand() % 4) {
	case 0:
		MoveDown();
		break;
	case 1:
		MoveLeft();
		break;
	case 2:
		MoveRight();
		break;
	case 3:
		MoveUp();
		break;
	}
}

void Game_Character::MoveTowardsPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if (sx != 0 || sy != 0) {
		if ( std::abs(sx) > std::abs(sy) ) {
			(sx > 0) ? MoveLeft() : MoveRight();
			if (move_failed && sy != 0) {
				(sy > 0) ? MoveUp() : MoveDown();
			}
		} else {
			(sy > 0) ? MoveUp() : MoveDown();
			if (move_failed && sx != 0) {
				(sx > 0) ? MoveLeft() : MoveRight();
			}
		}
	}
}

void Game_Character::MoveAwayFromPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if (sx != 0 || sy != 0) {
		if ( std::abs(sx) > std::abs(sy) ) {
			(sx > 0) ? MoveRight() : MoveLeft();
			if (move_failed && sy != 0) {
				(sy > 0) ? MoveDown() : MoveUp();
			}
		} else {
			(sy > 0) ? MoveDown() : MoveUp();
			if (move_failed && sx != 0) {
				(sx > 0) ? MoveRight() : MoveLeft();
			}
		}
	}
}

void Game_Character::TurnDown() {
	if (!direction_fix) {
		direction = RPG::EventPage::Direction_down;
		stop_count = 0;
	}
}

void Game_Character::TurnLeft() {
	if (!direction_fix) {
		direction = RPG::EventPage::Direction_left;
		stop_count = 0;
	}
}

void Game_Character::TurnRight() {
	if (!direction_fix) {
		direction = RPG::EventPage::Direction_right;
		stop_count = 0;
	}
}

void Game_Character::TurnUp() {
	if (!direction_fix) {
		direction = RPG::EventPage::Direction_up;
		stop_count = 0;
	}
}

void Game_Character::Turn90DegreeLeft() {
	switch (direction) {
	case RPG::EventPage::Direction_down:
		TurnRight();
		break;
	case RPG::EventPage::Direction_left:
		TurnDown();
		break;
	case RPG::EventPage::Direction_right:
		TurnUp();
		break;
	case RPG::EventPage::Direction_up:
		TurnLeft();
		break;
	}
}

void Game_Character::Turn90DegreeRight() {
	switch (direction) {
	case RPG::EventPage::Direction_down:
		TurnLeft();
		break;
	case RPG::EventPage::Direction_left:
		TurnUp();
		break;
	case RPG::EventPage::Direction_right:
		TurnDown();
		break;
	case RPG::EventPage::Direction_up:
		TurnRight();
		break;
	}
}

void Game_Character::Turn180Degree() {
	switch (direction) {
	case RPG::EventPage::Direction_down:
		TurnUp();
		break;
	case RPG::EventPage::Direction_left:
		TurnRight();
		break;
	case RPG::EventPage::Direction_right:
		TurnLeft();
		break;
	case RPG::EventPage::Direction_up:
		TurnDown();
		break;
	}
}

void Game_Character::Turn90DegreeLeftOrRight() {
	int value = rand() % 2;

	if (value == 0) {
		Turn90DegreeLeft();
	} else {
		Turn90DegreeRight();
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

void Game_Character::FaceRandomDirection() {
}

void Game_Character::FaceTowardsHero() {
}

void Game_Character::FaceAwayFromHero() {
}

void Game_Character::Wait() {
}

void Game_Character::BeginJump() {
}

void Game_Character::EndJump() {
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
	if ((!direction_fix) && (direction != -1)) {
		this->direction = direction;
		stop_count = 0;
	}
}

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
	prelock_direction = -1;
	wait_count = 0;
	MoveTypeCustom();
}

void Game_Character::CancelMoveRoute(RPG::MoveRoute* route, Game_Interpreter* owner) {
	if (!move_route_forcing ||
		move_route_owner != owner ||
		move_route != route)
		return;

	move_route_forcing = false;
	move_route_owner = NULL;
	move_route = original_move_route;
	move_route_index = original_move_route_index;
	original_move_route = NULL;
}

void Game_Character::DetachMoveRouteOwner(Game_Interpreter* owner) {
	if (owner == move_route_owner) {
		move_route_owner = NULL;
	}
}

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

// Gets Character
Game_Character* Game_Character::GetCharacter(int character_id, int event_id) {
	switch (character_id) {
		case CharPlayer:
			// Player/Hero
			return Main_Data::game_player.get();
		case CharBoat:
			return Game_Map::GetVehicle(Game_Vehicle::Boat);
		case CharShip:
			return Game_Map::GetVehicle(Game_Vehicle::Ship);
		case CharAirship:
			return Game_Map::GetVehicle(Game_Vehicle::Airship);
		case CharThisEvent:
			// This event
			return (Game_Map::GetEvents().empty()) ? NULL : Game_Map::GetEvents().find(event_id)->second.get();
		default:
			// Other events
			return (Game_Map::GetEvents().empty()) ? NULL : Game_Map::GetEvents().find(character_id)->second.get();
	}
}
