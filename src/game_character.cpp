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
#include "input.h"
#include "main_data.h"
#include "game_message.h"
#include "player.h"
#include "util_macro.h"
#include <math.h>
#include <cassert>
#include <cstdlib>

Game_Character::Game_Character() :
	tile_id(0),
	real_x(0),
	real_y(0),
	pattern(RPG::EventPage::Frame_middle),
	original_pattern(RPG::EventPage::Frame_middle),
	last_pattern(0),
	through(false),
	animation_id(0),
	animation_type(RPG::EventPage::AnimType_non_continuous),
	move_route_owner(NULL),
	original_move_frequency(-1),
	move_type(RPG::EventPage::MoveType_stationary),
	move_failed(false),
	move_count(0),
	wait_count(0),
	jumping(false),
	jump_peak(0),
	jump_index(0),
	jump_x(0),
	jump_y(0),
	jump_plus_x(0),
	jump_plus_y(0),
	anime_count(0),
	stop_count(0),
	walk_animation(true),
	cycle_stat(false),
	opacity(255),
	visible(true) {
}

Game_Character::~Game_Character() {
	if (move_route_owner != NULL) {
		move_route_owner->EndMoveRoute(this);
	}
}

int Game_Character::GetSteppingSpeed() const {
	return GetMoveSpeed();
}

bool Game_Character::IsMoving() const {
	if (move_count > 0) return false; //Jumping

	return real_x != GetX() * SCREEN_TILE_WIDTH || real_y != GetY() * SCREEN_TILE_WIDTH;
}

bool Game_Character::IsJumping() const {
	return move_count > 0;
}

bool Game_Character::IsStopping() const {
	return !(IsMoving() || IsJumping());
}

bool Game_Character::IsPassable(int x, int y, int d) const {
	int new_x = x + (d == RPG::EventPage::Direction_right ? 1 : d == RPG::EventPage::Direction_left ? -1 : 0);
	int new_y = y + (d == RPG::EventPage::Direction_down ? 1 : d == RPG::EventPage::Direction_up ? -1 : 0);

	if (Game_Map::GetLoopHorizontal()) {
		new_x = Game_Map::WrapX(new_x);
	}
	if (Game_Map::GetLoopVertical()) {
		new_y = Game_Map::WrapY(new_y);
	}

	if (!Game_Map::IsValid(new_x, new_y))
		return false;

	if (GetThrough()) return true;

	if (!Game_Map::IsPassable(x, y, d, this))
		return false;

	if (!Game_Map::IsPassable(new_x, new_y, (d + 2) % 4, this))
		return false;

	if (Main_Data::game_player->IsInPosition(new_x, new_y)
		&& !Main_Data::game_player->GetThrough() && !GetSpriteName().empty()
		&& GetLayer() != RPG::EventPage::Layers_above) {
			return false;
	}

	return true;
}

bool Game_Character::IsLandable(int x, int y) const
{
	if (!Game_Map::IsValid(x, y))
		return false;

	if (GetThrough()) return true;

	if (!Game_Map::IsLandable(x, y, this))
		return false;

	if (Main_Data::game_player->IsInPosition(x, y)) {
		if (!Main_Data::game_player->GetThrough() && !GetSpriteName().empty() && (this != Main_Data::game_player.get())) {
			return false;
		}
	}

	return true;
}

bool Game_Character::IsMessageBlocking() const {
	return Game_Message::message_waiting && !Game_Message::GetContinueEvents();
}

void Game_Character::MoveTo(int x, int y) {
	SetX(x % Game_Map::GetWidth());
	SetY(y % Game_Map::GetHeight());
	real_x = GetX() * SCREEN_TILE_WIDTH;
	real_y = GetY() * SCREEN_TILE_WIDTH;
}

int Game_Character::GetScreenX() const {
	int x = real_x - Game_Map::GetDisplayX();
	int max_width = Game_Map::GetWidth() * SCREEN_TILE_WIDTH;

	if (x - max_width > 0) {
		x -= max_width;
	}
	else if (x < 0) {
		x += max_width;
	}

	x = x / (SCREEN_TILE_WIDTH / TILE_SIZE) + (TILE_SIZE/2);

	return x;
	/*return real_x / (SCREEN_TILE_WIDTH / TILE_SIZE) - Game_Map::GetDisplayX() / (SCREEN_TILE_WIDTH / TILE_SIZE) + (TILE_SIZE/2);*/
}

int Game_Character::GetScreenY() const {
	/*int y = real_y / (SCREEN_TILE_WIDTH / TILE_SIZE) - Game_Map::GetDisplayY() / (SCREEN_TILE_WIDTH / TILE_SIZE) + TILE_SIZE;*/d
	int y = real_y - Game_Map::GetDisplayY();
	int max_height = Game_Map::GetHeight() * SCREEN_TILE_WIDTH;

	if (y - max_height > 0) {
		y -= max_height;
	}
	else if (y < 0) {
		y += max_height;
	}

	y = y / (SCREEN_TILE_WIDTH / TILE_SIZE) + TILE_SIZE;

	int n;
	if (move_count >= jump_peak)
		n = move_count - jump_peak;
	else
		n = jump_peak - move_count;

	return y - (jump_peak * jump_peak - n * n) / 2;
}

int Game_Character::GetScreenZ() const {
	return GetScreenZ(0);
}

int Game_Character::GetScreenZ(int /* height */) const {
	if (GetLayer() == RPG::EventPage::Layers_above) return 999;

	if (GetLayer() == RPG::EventPage::Layers_below) return 0;
	
	int z = (real_y - Game_Map::GetDisplayY() + 3) / TILE_SIZE + (SCREEN_TILE_WIDTH / TILE_SIZE);

	return z;
}

void Game_Character::Update() {
	if (IsJumping()) {
		UpdateJump();
		anime_count += (IsSpinning() ? 1.0 : 0);
	} else if (IsContinuous() || IsSpinning()) {
		UpdateMove();
		UpdateStop();
	} else {
		if (IsMoving()) {
			UpdateMove();
		} else {
			UpdateStop();
		}
	}

	if (anime_count > 36.0/(GetSteppingSpeed()+1)) {
		if (IsSpinning()) {
			SetPrelockDirection((GetPrelockDirection() + 1) % 4);
		} else if (!IsContinuous() && IsStopping()) {
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

	if (wait_count > 0) {
		wait_count -= 1;
		return;
	}

	if (stop_count >= ((GetMoveFrequency() > 7) ? 0 : pow(2.0, 9 - GetMoveFrequency()))) {
		if (IsMoveRouteOverwritten()) {
			MoveTypeCustom();
		} else if (!IsMessageBlocking()) {
			UpdateSelfMovement();
		}
	}
}

void Game_Character::UpdateMove() {
	int distance = ((SCREEN_TILE_WIDTH / 128) << GetMoveSpeed());

	int real_targetx = GetX() * SCREEN_TILE_WIDTH;
	int real_targety = GetY() * SCREEN_TILE_WIDTH;
	int real_map_width = Game_Map::GetWidth() * SCREEN_TILE_WIDTH;
	int real_map_height = Game_Map::GetHeight() * SCREEN_TILE_WIDTH;

	int dist = real_x - real_targetx;

	if (dist < 0) { // target is to the right
		dist = -dist;
		int dist2 = real_map_width - dist;

		if (dist < dist2) {
			// better move to the right
			real_x = (real_x + distance) % real_map_width;
		}
		else if (Game_Map::GetLoopHorizontal() && dist > dist2) {
			// better move to the left
			real_x = (real_x - distance) % real_map_width;
		}
	}
	else if (dist > 0) { // target is to the left
		int dist2 = real_map_width - dist;

		if (dist < dist2) {
			// better move to the left
			real_x = (real_x -distance) % real_map_width;
		}
		else if (Game_Map::GetLoopHorizontal() && dist > dist2) {
			// better move to the right
			real_x = (real_x + distance) % real_map_width;
		}
	}

	dist = real_y - real_targety;

	if (dist < 0) { // target is above
		dist = -dist;
		int dist2 = real_map_height - dist;

		if (dist < dist2) {
			// better move upwards
			real_y = (real_y + distance) % real_map_height;
		}
		else if (Game_Map::GetLoopVertical() && dist > dist2) {
			// better move downwards
			real_y = (real_y - distance) % real_map_height;
		}
	}
	else if (dist > 0) { // target is below
		int dist2 = real_map_height - dist;

		if (dist < dist2) {
			// better move downwards
			real_y = (real_y - distance) % real_map_height;
		}
		else if (Game_Map::GetLoopVertical() && dist > dist2) {
			// better move upwards
			real_y = (real_y + distance) % real_map_height;
		}
	}

	if (real_x < 0) {
		real_x += real_map_width;
	}

	if (real_y < 0) {
		real_y += real_map_height;
	}

	anime_count += 
		(IsSpinning() ? 1.0 :
		(animation_type != RPG::EventPage::AnimType_fixed_graphic && walk_animation) ? 1.5 :
		0);
}

void Game_Character::UpdateJump() {
	static double x_step;
	static double y_step;

	if (move_count == jump_peak*2) {//First frame?
		x_step = (GetX()*SCREEN_TILE_WIDTH-real_x) / (jump_peak*2);
		y_step = (GetY()*SCREEN_TILE_WIDTH-real_y) / (jump_peak*2);
	}
	move_count--;

	real_x += x_step;
	real_y += y_step;
}

void Game_Character::UpdateSelfMovement() {
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

void Game_Character::UpdateStop() {
	if (pattern != original_pattern && !IsContinuous())
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

		if (move_failed) {
			Wait();
			stop_count = 0;
			// TODO: After waiting, try once more in the same direction
			cycle_stat = move_failed ? !cycle_stat : cycle_stat;
		}
	}
}

void Game_Character::MoveTypeCycleUpDown() {
	if (IsStopping()) {
		cycle_stat ? MoveUp() : MoveDown();

		if (move_failed) {
			Wait();
			stop_count = 0;
			// TODO: After waiting, try once more in the same direction
			cycle_stat = !cycle_stat;
		}
	}
}

void Game_Character::MoveTypeTowardsPlayer() {
	if (IsStopping()) {
		int sx = GetX() - Main_Data::game_player->GetX();
		int sy = GetY() - Main_Data::game_player->GetY();

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
		int sx = GetX() - Main_Data::game_player->GetX();
		int sy = GetY() - Main_Data::game_player->GetY();

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
	// Detect if custom movement or event overwrite
	const RPG::MoveRoute* active_route;
	int active_route_index;
	bool overwrite_changed = IsMoveRouteOverwritten();
	if (IsMoveRouteOverwritten()) {
		active_route = &GetMoveRoute();
		active_route_index = GetMoveRouteIndex();
	}
	else {
		active_route = &original_move_route;
		active_route_index = GetOriginalMoveRouteIndex();
	}

	if (IsStopping()) {
		move_failed = false;
		if ((size_t)active_route_index >= active_route->move_commands.size()) {
			// End of Move list
			if (active_route->repeat) {
				active_route_index = 0;
				SetMoveRouteRepeated(true);
			} else if (IsMoveRouteOverwritten()) {
				SetMoveRouteOverwritten(false);
				EndMoveRoute();
				stop_count = 0;
			}
		} else {
			do {
				const RPG::MoveCommand& move_command = active_route->move_commands[active_route_index];

				int command_id = move_command.command_id;
				if (!jumping && command_id == RPG::MoveCommand::Code::begin_jump) {
					active_route_index = BeginJump(active_route, active_route_index);
				}

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
					TurnTowardHero();
					break;
				case RPG::MoveCommand::Code::face_away_from_hero:
					TurnAwayFromHero();
					break;
				case RPG::MoveCommand::Code::wait:
					Wait();
					break;
				case RPG::MoveCommand::Code::begin_jump:
					// Multiple BeginJumps are ignored
					break;
				case RPG::MoveCommand::Code::end_jump:
					active_route_index = EndJump(active_route, active_route_index);
					break;
				case RPG::MoveCommand::Code::lock_facing:
					SetFacingLocked(true);
					break;
				case RPG::MoveCommand::Code::unlock_facing:
					SetFacingLocked(false);
					break;
				case RPG::MoveCommand::Code::increase_movement_speed:
					SetMoveSpeed(min(GetMoveSpeed() + 1, 6));
					break;
				case RPG::MoveCommand::Code::decrease_movement_speed:
					SetMoveSpeed(max(GetMoveSpeed() - 1, 1));
					break;
				case RPG::MoveCommand::Code::increase_movement_frequence:
					SetMoveFrequency(min(GetMoveFrequency() + 1, 8));
					break;
				case RPG::MoveCommand::Code::decrease_movement_frequence:
					SetMoveFrequency(max(GetMoveFrequency() - 1, 1));
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
					SetGraphic(move_command.parameter_string, move_command.parameter_a);
					break;
				case RPG::MoveCommand::Code::play_sound_effect: // String: File, Parameters: Volume, Tempo, Balance
					if (move_command.parameter_string != "(OFF)" && move_command.parameter_string != "(Brak)") {
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
					walk_animation = false;
					break;
				case RPG::MoveCommand::Code::start_animation:
					walk_animation = true;
					break;
				case RPG::MoveCommand::Code::increase_transp:
					SetOpacity(max(40, GetOpacity() - 45));
					break;
				case RPG::MoveCommand::Code::decrease_transp:
					SetOpacity(GetOpacity() + 45);
					break;
				}

				if (active_route->skippable || !move_failed) {
					++active_route_index;
				}
			} while (jumping);

			if ((size_t)active_route_index >= active_route->move_commands.size()) {
				stop_count = (active_route->repeat ? 0 : 256);
			}
		}
	}

	// When the overwrite status changed the active_index belongs to the
	// current non-active move route
	if (overwrite_changed != IsMoveRouteOverwritten()) {
		if (IsMoveRouteOverwritten()) {
			SetOriginalMoveRouteIndex(active_route_index);
		}
		else {
			SetMoveRouteIndex(active_route_index);
		}
	}
	else {
		if (IsMoveRouteOverwritten()) {
			SetMoveRouteIndex(active_route_index);
		}
		else {
			SetOriginalMoveRouteIndex(active_route_index);
		}
	}
}

void Game_Character::EndMoveRoute() {
	if (move_route_owner != NULL) {
		move_route_owner->EndMoveRoute(this);
		move_route_owner = NULL;
	}

	SetMoveFrequency(original_move_frequency);
}

void Game_Character::MoveDown() {
	if (!IsDirectionFixed()) TurnDown();

	if (jumping) {
		jump_plus_y++;
		return;
	}

	if (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_down)) {
		SetY(Game_Map::WrapY(GetY() + 1));
		BeginMove();
		stop_count = 0;
		move_failed = false;
	} else {
		CheckEventTriggerTouch(GetX(), GetY() + 1);
		move_failed = true;
	}
}

void Game_Character::MoveLeft() {
	if (!IsDirectionFixed()) TurnLeft();

	if (jumping) {
		jump_plus_x--;
		return;
	}

	if (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_left)) {
		SetX(Game_Map::WrapX(GetX() - 1));
		BeginMove();
		stop_count = 0;
		move_failed = false;
	} else {
		CheckEventTriggerTouch(GetX() - 1, GetY());
		move_failed = true;
	}
}

void Game_Character::MoveRight() {
	if (!IsDirectionFixed()) TurnRight();

	if (jumping) {
		jump_plus_x++;
		return;
	}

	if (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_right)) {
		SetX(Game_Map::WrapX(GetX() + 1));
		BeginMove();
		stop_count = 0;
		move_failed = false;
	} else {
		CheckEventTriggerTouch(GetX() + 1, GetY());
		move_failed = true;
	}
}

void Game_Character::MoveUp() {
	if (!IsDirectionFixed()) TurnUp();

	if (jumping) {
		jump_plus_y--;
		return;
	}

	if (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_up)) {
		SetY(Game_Map::WrapY(GetY() - 1));
		BeginMove();
		stop_count = 0;
		move_failed = false;
	} else {
		CheckEventTriggerTouch(GetX(), GetY() - 1);
		move_failed = true;
	}
}

void Game_Character::MoveForward() {
	switch (GetDirection()) {
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
	if (!IsDirectionFixed()) {
		if (GetDirection() % 2) {
			TurnLeft();
		} else {
			TurnDown();
		}
	}

	if (jumping) {
		jump_plus_x--;
		jump_plus_y++;
		return;
	}

	if ((IsPassable(GetX(), GetY(), RPG::EventPage::Direction_left)
		&& IsPassable(GetX() - 1, GetY(), RPG::EventPage::Direction_down))
		|| (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_down)
		&& IsPassable(GetX(), GetY() + 1, RPG::EventPage::Direction_left))) {
			SetX(GetX() - 1);
			SetY(GetY() + 1);
			BeginMove();
			stop_count = 0;
			move_failed = false;
	}
}

void Game_Character::MoveDownRight() {
	if (!IsDirectionFixed()) {
		if (GetDirection() % 2) {
			TurnRight();
		} else {
			TurnDown();
		}
	}

	if (jumping) {
		jump_plus_x++;
		jump_plus_y++;
		return;
	}

	if ((IsPassable(GetX(), GetY(), RPG::EventPage::Direction_right)
		&& IsPassable(GetX() + 1, GetY(), RPG::EventPage::Direction_down))
		|| (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_down)
		&& IsPassable(GetX(), GetY() + 1, RPG::EventPage::Direction_right))) {
			SetX(GetX() + 1);
			SetY(GetY() + 1);
			BeginMove();
			stop_count = 0;
			move_failed = false;
	}
}


void Game_Character::MoveUpLeft() {
	if (!IsDirectionFixed()) {
		if (GetDirection() % 2) {
			TurnLeft();
		} else {
			TurnUp();
		}
	}

	if (jumping) {
		jump_plus_x--;
		jump_plus_y--;
		return;
	}

	if ((IsPassable(GetX(), GetY(), RPG::EventPage::Direction_left)
		&& IsPassable(GetX() - 1, GetY(), RPG::EventPage::Direction_up))
		|| (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_up)
		&& IsPassable(GetX(), GetY() - 1, RPG::EventPage::Direction_left))) {
			SetX(GetX() - 1);
			SetY(GetY() - 1);
			BeginMove();
			stop_count = 0;
			move_failed = false;
	}
}


void Game_Character::MoveUpRight() {
	if (!IsDirectionFixed()) {
		if (GetDirection() % 2) {
			TurnRight();
		} else {
			TurnUp();
		}
	}

	if (jumping) {
		jump_plus_x++;
		jump_plus_y--;
		return;
	}

	if ((IsPassable(GetX(), GetY(), RPG::EventPage::Direction_right)
		&& IsPassable(GetX() + 1, GetY(), RPG::EventPage::Direction_up))
		|| (IsPassable(GetX(), GetY(), RPG::EventPage::Direction_up)
		&& IsPassable(GetX(), GetY() - 1, RPG::EventPage::Direction_right))) {
			SetX(GetX() + 1);
			SetY(GetY() - 1);
			BeginMove();
			stop_count = 0;
			move_failed = false;
	}
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
	int sx = GetDistanceXfromPlayer();
	int sy = GetDistanceYfromPlayer();

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
	int sx = GetDistanceXfromPlayer();
	int sy = GetDistanceYfromPlayer();

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
	SetDirection(RPG::EventPage::Direction_down);
	move_failed = false;
	if (!IsSpinning()) {
		stop_count = pow(2.0, 8 - GetMoveFrequency());
	}
}

void Game_Character::TurnLeft() {
	SetDirection(RPG::EventPage::Direction_left);
	move_failed = false;
	if (!IsSpinning()) {
		stop_count = pow(2.0, 8 - GetMoveFrequency());
	}
}

void Game_Character::TurnRight() {
	SetDirection(RPG::EventPage::Direction_right);
	move_failed = false;
	if (!IsSpinning()) {
		stop_count = pow(2.0, 8 - GetMoveFrequency());
	}
}

void Game_Character::TurnUp() {
	SetDirection(RPG::EventPage::Direction_up);
	move_failed = false;
	if (!IsSpinning()) {
		stop_count = pow(2.0, 8 - GetMoveFrequency());
	}
}

void Game_Character::Turn90DegreeLeft() {
	switch (GetDirection()) {
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
	switch (GetDirection()) {
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
	switch (GetDirection()) {
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


void Game_Character::BeginMove() {
	// no-op
}

void Game_Character::TurnTowardHero() {
	int sx = GetDistanceXfromPlayer();
	int sy = GetDistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		(sx > 0) ? TurnLeft() : TurnRight();
	}
	else if ( std::abs(sx) < std::abs(sy) ) {
		(sy > 0) ? TurnUp() : TurnDown();
	}
}

void Game_Character::TurnAwayFromHero() {
	int sx = GetDistanceXfromPlayer();
	int sy = GetDistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		(sx > 0) ? TurnRight() : TurnLeft();
	}
	else if ( std::abs(sx) < std::abs(sy) ) {
		(sy > 0) ? TurnDown() : TurnUp();
	}
}

void Game_Character::FaceRandomDirection() {
	switch (rand() % 4) {
	case 0:
		TurnDown();
		break;
	case 1:
		TurnLeft();
		break;
	case 2:
		TurnRight();
		break;
	case 3:
		TurnUp();
		break;
	}
}

void Game_Character::Wait() {
	if (jumping) {
		return;
	}

	wait_count += 20;
}

int Game_Character::BeginJump(const RPG::MoveRoute* current_route, int current_index) {
	jump_x = GetX();
	jump_y = GetY();
	jump_plus_x = 0;
	jump_plus_y = 0;
	jump_index = current_index;

	// Search EndJump command.
	// When missing the move route ends directly.

	for (unsigned int i = current_index; i < current_route->move_commands.size(); ++i) {
		const RPG::MoveCommand& move_command = current_route->move_commands[i];

		if (move_command.command_id == RPG::MoveCommand::Code::end_jump) {
			// End jump found
			jumping = true;
			return current_index;
		}
	}

	// No end jump found
	return current_route->move_commands.size() - 1;
}

int Game_Character::EndJump(const RPG::MoveRoute* current_route, int current_index) {
	jumping = false;

	if (
		// A character can always land on a tile they were already standing on
		!(jump_plus_x == 0 && jump_plus_y == 0) &&
		!IsLandable(jump_x + jump_plus_x, jump_y + jump_plus_y)
	) {
		// Reset to begin jump command and try again...
		move_failed = true;

		if (current_route->skippable) {
			return current_index;
		}

		return jump_index;
	}

	SetX(jump_x + jump_plus_x);
	SetY(jump_y + jump_plus_y);

	//TODO: C++11 got round() function defined in math.h
	float distance = sqrt((float)(jump_plus_x * jump_plus_x + jump_plus_y * jump_plus_y));
	if (distance >= floor(distance) + 0.5) distance = ceil(distance);
	else distance = floor(distance);

	jump_peak = 10 + (int)distance - GetMoveSpeed();
	move_count = jump_peak * 2;

	stop_count = 0;
	move_failed = false;

	return current_index;
}

int Game_Character::GetDistanceXfromPlayer() const {
	int sx = GetX() - Main_Data::game_player->GetX();
	if (Game_Map::GetLoopHorizontal()) {
		if (std::abs(sx) > Game_Map::GetWidth() / 2) {
			sx -= Game_Map::GetWidth();
		}
	}
	return sx;
}

int Game_Character::GetDistanceYfromPlayer() const {
	int sy = GetY() - Main_Data::game_player->GetY();
	if (Game_Map::GetLoopVertical()) {
		if (std::abs(sy) > Game_Map::GetHeight() / 2) {
			sy -= Game_Map::GetHeight();
		}
	}
	return sy;
}

void Game_Character::Lock() {
	if (!IsDirectionFixed()) {
		int prelock_dir = GetDirection();
		TurnTowardHero();
		SetPrelockDirection(prelock_dir);
	}
}

void Game_Character::Unlock() {
	if (!IsDirectionFixed()) {
		SetDirection(GetPrelockDirection());
	}
}

void Game_Character::ForceMoveRoute(RPG::MoveRoute* new_route,
									int frequency,
									Game_Interpreter* owner) {
	EndMoveRoute();

	original_move_frequency = GetMoveFrequency();

	SetMoveRoute(*new_route);
	SetMoveRouteIndex(0);
	SetMoveRouteOverwritten(true);
	SetMoveRouteRepeated(false);
	SetMoveFrequency(frequency);
	move_route_owner = owner;
	wait_count = 0;
	stop_count = 256;
}

void Game_Character::CancelMoveRoute(Game_Interpreter* /* owner */) {
	// FIXME unused Game_Interpreter* parameter
	SetMoveRouteOverwritten(false);
	move_route_owner = NULL;
}

bool Game_Character::DetachMoveRouteOwner(Game_Interpreter* owner) {
	if (owner == move_route_owner) {
		move_route_owner = NULL;
		return true;
	}
	return false;
}

int Game_Character::GetTileId() const {
	return tile_id;
}

int Game_Character::GetRealX() const {
	return real_x;
}

int Game_Character::GetRealY() const {
	return real_y;
}

int Game_Character::GetPattern() const {
	return pattern;
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
	return ((GetX() == x) && (GetY() == y));
}

void Game_Character::SetOpacity(int opacity) {
	this->opacity = min(max(0, opacity), 255);
}

int Game_Character::GetOpacity() const {
	return opacity;
}

bool Game_Character::GetVisible() const {
	return visible;
}

void Game_Character::SetVisible(bool visible) {
	this->visible = visible;
}

bool Game_Character::IsFlashPending() const {
	return GetFlashTimeLeft() > 0;
}

bool Game_Character::IsDirectionFixed() {
	return
		animation_type == RPG::EventPage::AnimType_fixed_continuous ||
		animation_type == RPG::EventPage::AnimType_fixed_graphic ||
		animation_type == RPG::EventPage::AnimType_fixed_non_continuous ||
		IsFacingLocked();
}

bool Game_Character::IsContinuous() {
	return
		animation_type == RPG::EventPage::AnimType_continuous ||
		animation_type == RPG::EventPage::AnimType_fixed_continuous;
}

bool Game_Character::IsSpinning() {
	return animation_type == RPG::EventPage::AnimType_spin;
}

int Game_Character::GetBushDepth() {
	return Game_Map::GetBushDepth(GetX(), GetY());
}

void Game_Character::SetGraphic(const std::string& name, int index) {
	SetSpriteName(name);
	SetSpriteIndex(index);
	pattern = RPG::EventPage::Frame_middle;
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
