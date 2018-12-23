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
#include "utils.h"
#include "util_macro.h"
#include <cmath>
#include <cassert>

Game_Character::Game_Character(RPG::SaveMapEventBase* d) :
	last_pattern(0),
	original_move_frequency(-1),
	move_type(RPG::EventPage::MoveType_stationary),
	move_failed(false),
	last_move_failed(false),
	move_count(0),
	wait_count(0),
	jump_plus_x(0),
	jump_plus_y(0),
	visible(true),
	_data(d)
{
}

Game_Character::~Game_Character() {
	Game_Map::RemovePendingMove(this);
}

int Game_Character::GetSteppingSpeed() const {
	int move_speed = GetMoveSpeed();
	if (IsSpinning()) {
		// 24, 16, 12, 8, 6, 4
		return (move_speed < 4) ? 48 / (move_speed + 1) : 24 / (move_speed - 1);
	} else if (IsMoving()) {
		// 12, 10, 8, 6, 5, 4
		return (move_speed < 4) ? 60 / (move_speed + 4) : 30 / (move_speed + 1);
	} else {
		// 16, 12, 10, 8, 7, 6
		return (move_speed < 2) ? 16 : 60 / (move_speed + 3);
	}
}

bool Game_Character::IsMoving() const {
	return !IsJumping() && GetRemainingStep() > 0;
}


bool Game_Character::IsStopping() const {
	return !(IsMoving() || IsJumping());
}

bool Game_Character::MakeWay(int x, int y, int d) const {
	if (d > 3) {
		return MakeWayDiagonal(x, y, d);
	}

	return Game_Map::MakeWay(x, y, d, *this, false);
}

bool Game_Character::IsLandable(int x, int y) const
{
	if (!Game_Map::IsValid(x, y))
		return false;

	if (GetThrough()) return true;

	if (!Game_Map::IsLandable(x, y, this))
		return false;

	if (GetLayer() == RPG::EventPage::Layers_same && Main_Data::game_player->IsInPosition(x, y)) {
		if (!Main_Data::game_player->GetThrough() && !GetSpriteName().empty() && (this != Main_Data::game_player.get())) {
			return false;
		}
	}

	return true;
}

void Game_Character::MoveTo(int x, int y) {
	SetX(Game_Map::RoundX(x));
	SetY(Game_Map::RoundY(y));
	SetRemainingStep(0);
}

int Game_Character::GetScreenX(bool apply_shift) const {
	int x = GetSpriteX() / TILE_SIZE - Game_Map::GetDisplayX() / TILE_SIZE + (TILE_SIZE / 2);

	if (apply_shift) {
		x += Game_Map::GetWidth() * TILE_SIZE;
	}

	return x;
}

int Game_Character::GetScreenY(bool apply_shift) const {
	int y = GetSpriteY() / TILE_SIZE - Game_Map::GetDisplayY() / TILE_SIZE + TILE_SIZE;

	if (IsJumping()) {
		int jump_height = (GetRemainingStep() > SCREEN_TILE_SIZE / 2 ? SCREEN_TILE_SIZE - GetRemainingStep() : GetRemainingStep()) / 8;
		y -= (jump_height < 5 ? jump_height * 2 : jump_height < 13 ? jump_height + 4 : 16);
	}

	if (apply_shift) {
		y += Game_Map::GetHeight() * TILE_SIZE;
	}

	return y;
}

int Game_Character::GetScreenZ(bool apply_shift) const {
	int z = 0;

	if (IsFlying()) {
		z = Priority_EventsAbove;
	} else if (GetLayer() == RPG::EventPage::Layers_same) {
		z = Priority_Player;
	} else if (GetLayer() == RPG::EventPage::Layers_below) {
		z = Priority_EventsBelow;
	} else if (GetLayer() == RPG::EventPage::Layers_above) {
		z = Priority_EventsAbove;
	}

	// For events on the screen, this should be inside a 0-40 range
	z += GetScreenY(apply_shift) >> 3;

	return z;
}

void Game_Character::Update() {
	if (wait_count == 0 && GetStopCount() >= GetMaxStopCount()) {
		if (IsMoveRouteOverwritten()) {
			MoveTypeCustom();
		} else {
			// Only events
			UpdateSelfMovement();
		}
	}

	if (wait_count > 0) {
		--wait_count;
	}
}

void Game_Character::UpdateSprite() {
	if (IsJumping()) {
		UpdateJump();
		if (IsSpinning())
			SetAnimCount(GetAnimCount() + 1);
	} else if (IsMoving()) {
		SetRemainingStep(GetRemainingStep() - min(1 << (1 + GetMoveSpeed()), GetRemainingStep()));
		if (IsSpinning() || IsAnimated())
			SetAnimCount(GetAnimCount() + 1);
	} else {
		data()->stop_count++;

		if (IsAnimated() && (IsSpinning() || IsContinuous() || GetAnimFrame() != RPG::EventPage::Frame_middle))
			SetAnimCount(GetAnimCount() + 1);
	}

	if (GetAnimCount() >= GetSteppingSpeed()) {
		if (IsSpinning()) {
			SetSpriteDirection((GetSpriteDirection() + 1) % 4);
		} else if (!IsContinuous() && IsStopping()) {
			SetAnimFrame(RPG::EventPage::Frame_middle);
			last_pattern = last_pattern == RPG::EventPage::Frame_left ? RPG::EventPage::Frame_right : RPG::EventPage::Frame_left;
		} else {
			if (last_pattern == RPG::EventPage::Frame_left) {
				if (GetAnimFrame() == RPG::EventPage::Frame_right) {
					SetAnimFrame(RPG::EventPage::Frame_middle);
					last_pattern = RPG::EventPage::Frame_right;
				} else {
					SetAnimFrame(RPG::EventPage::Frame_right);
				}
			} else {
				if (GetAnimFrame() == RPG::EventPage::Frame_left) {
					SetAnimFrame(RPG::EventPage::Frame_middle);
					last_pattern = RPG::EventPage::Frame_left;
				} else {
					SetAnimFrame(RPG::EventPage::Frame_left);
				}
			}
		}

		SetAnimCount(0);
	}
}

void Game_Character::UpdateJump() {
	static const int jump_speed[] = {8, 12, 16, 24, 32, 64};
	SetRemainingStep(GetRemainingStep() - min(jump_speed[GetMoveSpeed() - 1], GetRemainingStep()));
	if (GetRemainingStep() <= 0) {
		SetJumping(false);
	}
}

void Game_Character::UpdateSelfMovement() {
	// no-op: Only events can have custom move routes
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
	MoveOption option =
		active_route->skippable ? MoveOption::IgnoreIfCantMove : MoveOption::Normal;

	if (IsStopping()) {
		move_failed = false;

		int original_index = active_route_index;
		bool looped_around = false;
		while (true) {
			if (!IsStopping() || wait_count > 0 || GetStopCount() < GetMaxStopCount())
				break;

			if (active_route_index == original_index && looped_around) {
				// We've gone around a full loop; stop here for now
				break;
			}

			if ((size_t)active_route_index >= active_route->move_commands.size()) {
				if (active_route->repeat && !active_route->move_commands.empty()) {
					looped_around = true;
					active_route_index = 0;
					SetMoveRouteRepeated(true);
					if (original_index == 0) {
						break;
					}
				} else {
					break;
				}
			}

			const RPG::MoveCommand& move_command = active_route->move_commands[active_route_index];

			switch (move_command.command_id) {
			case RPG::MoveCommand::Code::move_up:
			case RPG::MoveCommand::Code::move_right:
			case RPG::MoveCommand::Code::move_down:
			case RPG::MoveCommand::Code::move_left:
			case RPG::MoveCommand::Code::move_upright:
			case RPG::MoveCommand::Code::move_downright:
			case RPG::MoveCommand::Code::move_downleft:
			case RPG::MoveCommand::Code::move_upleft:
				Move(move_command.command_id, option);
				break;
			case RPG::MoveCommand::Code::move_random:
				MoveRandom(option);
				break;
			case RPG::MoveCommand::Code::move_towards_hero:
				MoveTowardsPlayer();
				break;
			case RPG::MoveCommand::Code::move_away_from_hero:
				MoveAwayFromPlayer();
				break;
			case RPG::MoveCommand::Code::move_forward:
				MoveForward(option);
				break;
			case RPG::MoveCommand::Code::face_up:
				Turn(Up);
				break;
			case RPG::MoveCommand::Code::face_right:
				Turn(Right);
				break;
			case RPG::MoveCommand::Code::face_down:
				Turn(Down);
				break;
			case RPG::MoveCommand::Code::face_left:
				Turn(Left);
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
				BeginJump(active_route, &active_route_index);
				break;
			case RPG::MoveCommand::Code::end_jump:
				// EndJump();
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
				Game_Switches.Set(move_command.parameter_a, true);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
				Game_Map::Refresh();
				break;
			case RPG::MoveCommand::Code::switch_off: // Parameter A: Switch to turn off
				Game_Switches.Set(move_command.parameter_a, false);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
				Game_Map::Refresh();
				break;
			case RPG::MoveCommand::Code::change_graphic: // String: File, Parameter A: index
				SetGraphic(move_command.parameter_string, move_command.parameter_a);
				break;
			case RPG::MoveCommand::Code::play_sound_effect: // String: File, Parameters: Volume, Tempo, Balance
				if (move_command.parameter_string != "(OFF)" && move_command.parameter_string != "(Brak)") {
					RPG::Sound sound;
					sound.name = move_command.parameter_string;
					sound.volume = move_command.parameter_a;
					sound.tempo = move_command.parameter_b;
					sound.balance = move_command.parameter_c;

					Game_System::SePlay(sound);
				}
				break;
			case RPG::MoveCommand::Code::walk_everywhere_on:
				SetThrough(true);
				data()->route_through = true;
				break;
			case RPG::MoveCommand::Code::walk_everywhere_off:
				SetThrough(false);
				data()->route_through = false;
				break;
			case RPG::MoveCommand::Code::stop_animation:
				if (IsContinuous()) {
					SetAnimFrame(RPG::EventPage::Frame_middle);
				}
				SetAnimPaused(true);
				break;
			case RPG::MoveCommand::Code::start_animation:
				SetAnimPaused(false);
				break;
			case RPG::MoveCommand::Code::increase_transp:
				SetTransparency(GetTransparency() + 1);
				break;
			case RPG::MoveCommand::Code::decrease_transp:
				SetTransparency(GetTransparency() - 1);
				break;
			}

			if (move_command.command_id <= RPG::MoveCommand::Code::move_forward) {
				any_move_successful |= !move_failed;
			}

			last_move_failed = move_failed;
			if (move_failed) {
				if (active_route->skippable) {
					last_move_failed = false;
				} else {
					break;
				}
			}

			++active_route_index;

			if ((size_t)active_route_index >= active_route->move_commands.size() && !active_route->repeat) {
				SetMaxStopCount(0);
			}
		}

		if ((size_t)active_route_index >= active_route->move_commands.size() && IsStopping() && wait_count == 0) {
			if (!(active_route->repeat && active_route->skippable) && IsMoveRouteOverwritten()) {
				CancelMoveRoute();
				Game_Map::RemovePendingMove(this);
				SetStopCount(0);
				SetMaxStopCount((GetMoveFrequency() > 7) ? 0 : (int) pow(2.0, 8 - GetMoveFrequency()));
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

void Game_Character::Move(int dir, MoveOption option) {
	int dx = (dir == Right || dir == UpRight || dir == DownRight) - (dir == Left || dir == DownLeft || dir == UpLeft);
	int dy = (dir == Down || dir == DownRight || dir == DownLeft) - (dir == Up || dir == UpRight || dir == UpLeft);

	if (IsJumping()) {
		jump_plus_x += dx;
		jump_plus_y += dy;
		return;
	}

	move_failed = !MakeWay(GetX(), GetY(), dir);

	if (!move_failed || option == MoveOption::Normal) {
		SetDirection(dir);
		if (!(IsDirectionFixed() || IsFacingLocked() || IsSpinning())) {
			if (dir > 3) // Diagonal
				SetSpriteDirection(GetSpriteDirection() % 2 ? -dx + 2 : dy + 1);
			else
				SetSpriteDirection(dir);
		}
	}

	if (move_failed) {
		if (!CheckEventTriggerTouch(Game_Map::RoundX(GetX() + dx), Game_Map::RoundY(GetY() + dy)))
			return;
	} else {
		SetX(Game_Map::RoundX(GetX() + dx));
		SetY(Game_Map::RoundY(GetY() + dy));
		SetRemainingStep(SCREEN_TILE_SIZE);
		BeginMove();
	}

	SetStopCount(0);
	SetMaxStopCount((GetMoveFrequency() > 7) ? 0 : pow(2.0, 9 - GetMoveFrequency()));
}

void Game_Character::MoveForward(MoveOption option) {
	Move(GetDirection(), option);
}

void Game_Character::MoveRandom(MoveOption option) {
	Move(Utils::GetRandomNumber(0, 3), option);
}

void Game_Character::MoveTowardsPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	// Try in the same direction of the last failed move
	if (last_move_failed) {
		MoveForward();
		if (!move_failed)
			return;
	}

	if (sx != 0 || sy != 0) {
		if ( std::abs(sx) > std::abs(sy) ) {
			Move((sx > 0) ? Left : Right);
			if (move_failed && sy != 0)
				Move((sy > 0) ? Up : Down);
		} else {
			Move((sy > 0) ? Up : Down);
			if (move_failed && sx != 0) {
				Move((sx > 0) ? Left : Right);
			}
		}
	}
}

void Game_Character::MoveAwayFromPlayer() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if (sx != 0 || sy != 0) {
		if ( std::abs(sx) > std::abs(sy) ) {
			Move((sx > 0) ? Right : Left);
			if (move_failed && sy != 0)
				Move((sy > 0) ? Down : Up);
		} else {
			Move((sy > 0) ? Down : Up);
			if (move_failed && sx != 0)
				Move((sx > 0) ? Right : Left);
		}
	}
}

void Game_Character::Turn(int dir) {
	SetDirection(dir);
	SetSpriteDirection(dir);
	move_failed = false;
	SetStopCount(0);
	SetMaxStopCount((GetMoveFrequency() > 7) ? 0 : pow(2.0, 8 - GetMoveFrequency()));
}

void Game_Character::Turn90DegreeLeft() {
	Turn((GetSpriteDirection() + 3) % 4);
}

void Game_Character::Turn90DegreeRight() {
	Turn((GetSpriteDirection() + 1) % 4);
}

void Game_Character::Turn180Degree() {
	Turn((GetSpriteDirection() + 2) % 4);
}

void Game_Character::Turn90DegreeLeftOrRight() {
	if (Utils::ChanceOf(1,2)) {
		Turn90DegreeLeft();
	} else {
		Turn90DegreeRight();
	}
}

void Game_Character::BeginMove() {
	// no-op
}

void Game_Character::TurnTowardHero() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		Turn((sx > 0) ? Left : Right);
	} else {
		Turn((sy > 0) ? Up : Down);
	}
}

void Game_Character::TurnAwayFromHero() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		Turn((sx > 0) ? Right : Left);
	} else {
		Turn((sy > 0) ? Down : Up);
	}
}

void Game_Character::FaceRandomDirection() {
	Turn(Utils::GetRandomNumber(0, 3));
}

void Game_Character::Wait() {
	wait_count += 20;
}

void Game_Character::BeginJump(const RPG::MoveRoute* current_route, int* current_index) {
	SetBeginJumpX(GetX());
	SetBeginJumpY(GetY());
	jump_plus_x = 0;
	jump_plus_y = 0;
	SetJumping(true);

	bool end_found = false;
	unsigned int i;
	for (i = *current_index; i < current_route->move_commands.size(); ++i) {
		const RPG::MoveCommand& move_command = current_route->move_commands[i];
		switch (move_command.command_id) {
			case RPG::MoveCommand::Code::move_up:
			case RPG::MoveCommand::Code::move_right:
			case RPG::MoveCommand::Code::move_down:
			case RPG::MoveCommand::Code::move_left:
			case RPG::MoveCommand::Code::move_upright:
			case RPG::MoveCommand::Code::move_downright:
			case RPG::MoveCommand::Code::move_downleft:
			case RPG::MoveCommand::Code::move_upleft:
				Move(move_command.command_id);
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
				Turn(Up);
				break;
			case RPG::MoveCommand::Code::face_right:
				Turn(Right);
				break;
			case RPG::MoveCommand::Code::face_down:
				Turn(Down);
				break;
			case RPG::MoveCommand::Code::face_left:
				Turn(Left);
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

			default:
				break;
		}

		if (move_command.command_id == RPG::MoveCommand::Code::end_jump) {
			end_found = true;
			break;
		}
	}

	if (!end_found) {
		// No EndJump found. Move route ends directly
		*current_index = i;
		SetJumping(false);
		return;
	}

	int new_x = GetBeginJumpX() + jump_plus_x;
	int new_y = GetBeginJumpY() + jump_plus_y;

	if (Game_Map::LoopHorizontal()) {
		int map_width = Game_Map::GetWidth();
		if (new_x < 0) {
			SetBeginJumpX(GetBeginJumpX() + map_width);
			new_x += map_width;
		} else if (new_x >= map_width) {
			SetBeginJumpX(GetBeginJumpX() - map_width);
			new_x -= map_width;
		}
	}

	if (Game_Map::LoopVertical()) {
		int map_height = Game_Map::GetHeight();
		if (new_y < 0) {
			SetBeginJumpY(GetBeginJumpY() + map_height);
			new_y += map_height;
		} else if (new_y >= map_height) {
			SetBeginJumpY(GetBeginJumpY() - map_height);
			new_y -= map_height;
		}
	}

	if (jump_plus_x != 0 || jump_plus_y != 0) {
		if (std::abs(jump_plus_y) >= std::abs(jump_plus_x)) {
			SetDirection(jump_plus_y > 0 ? Down : Up);
			SetSpriteDirection(GetDirection());
		} else {
			SetDirection(jump_plus_x > 0 ? Right : Left);
			SetSpriteDirection(GetDirection());
		}
	}

	if (
		// A character can always land on a tile they were already standing on
		!(jump_plus_x == 0 && jump_plus_y == 0) &&
		!IsLandable(new_x, new_y)
	) {
		// Reset to begin jump command and try again...
		move_failed = true;
		SetJumping(false);

		if (current_route->skippable) {
			*current_index = i;
			return;
		}

		return;
	}

	SetX(new_x);
	SetY(new_y);
	*current_index = i;

	SetRemainingStep(SCREEN_TILE_SIZE);
	SetStopCount(0);
	SetMaxStopCount((GetMoveFrequency() > 7) ? 0 : pow(2.0, 9 - GetMoveFrequency()));
	move_failed = false;

	if (IsMiddleFrameAvailable()) {
		SetAnimFrame(RPG::EventPage::Frame_middle);
	}
}

void Game_Character::EndJump() {
	// no-op
}

int Game_Character::DistanceXfromPlayer() const {
	int sx = GetX() - Main_Data::game_player->GetX();
	if (Game_Map::LoopHorizontal()) {
		if (std::abs(sx) > Game_Map::GetWidth() / 2) {
			if (sx > 0)
				sx -= Game_Map::GetWidth();
			else
				sx += Game_Map::GetWidth();
		}
	}
	return sx;
}

int Game_Character::DistanceYfromPlayer() const {
	int sy = GetY() - Main_Data::game_player->GetY();
	if (Game_Map::LoopVertical()) {
		if (std::abs(sy) > Game_Map::GetHeight() / 2) {
			if (sy > 0)
				sy -= Game_Map::GetHeight();
			else
				sy += Game_Map::GetHeight();
		}
	}
	return sy;
}

void Game_Character::ForceMoveRoute(const RPG::MoveRoute& new_route,
									int frequency) {
	Game_Map::RemovePendingMove(this);

	if (new_route.move_commands.empty()) {
		CancelMoveRoute();
		return;
	}

	Game_Map::AddPendingMove(this);

	original_move_frequency = GetMoveFrequency();

	SetMoveRoute(new_route);
	SetMoveRouteIndex(0);
	SetMoveRouteOverwritten(true);
	SetMoveRouteRepeated(false);
	SetMoveFrequency(frequency);
	wait_count = 0;
	SetMaxStopCount(0);
	last_move_failed = false;
	any_move_successful = false;
}

void Game_Character::CancelMoveRoute() {
	SetMoveRouteOverwritten(false);
	SetMoveFrequency(original_move_frequency);
}

int Game_Character::GetTileId() const {
	return GetSpriteName().empty() ? GetSpriteIndex() : 0;
}

int Game_Character::GetSpriteX() const {
	int x = GetX() * SCREEN_TILE_SIZE;

	if (IsMoving()) {
		int d = GetDirection();
		if (d == Right || d == UpRight || d == DownRight)
			x -= GetRemainingStep();
		else if (d == Left || d == UpLeft || d == DownLeft)
			x += GetRemainingStep();
	} else if (IsJumping())
		x -= ((GetX() - GetBeginJumpX()) * GetRemainingStep());
	if (x < 0) {
		x += Game_Map::GetWidth() * SCREEN_TILE_SIZE;
	}

	return x;
}

int Game_Character::GetSpriteY() const {
	int y = GetY() * SCREEN_TILE_SIZE;

	if (IsMoving()) {
		int d = GetDirection();
		if (d == Down || d == DownRight || d == DownLeft)
			y -= GetRemainingStep();
		else if (d == Up || d == UpRight || d == UpLeft)
			y += GetRemainingStep();
	} else if (IsJumping())
		y -= (GetY() - GetBeginJumpY()) * GetRemainingStep();

	if (y < 0) {
		y += Game_Map::GetHeight() * SCREEN_TILE_SIZE;
	}

	return y;
}

bool Game_Character::IsInPosition(int x, int y) const {
	return ((GetX() == x) && (GetY() == y));
}

int Game_Character::GetOpacity() const {
	return (8 - GetTransparency()) * 32 - 1;
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

bool Game_Character::IsAnimated() const {
	return !IsAnimPaused() && GetAnimationType() != RPG::EventPage::AnimType_fixed_graphic;
}

bool Game_Character::IsDirectionFixed() const {
	auto at = GetAnimationType();
	return
		at == RPG::EventPage::AnimType_fixed_continuous ||
		at == RPG::EventPage::AnimType_fixed_graphic ||
		at == RPG::EventPage::AnimType_fixed_non_continuous;
}

bool Game_Character::IsContinuous() const {
	auto at = GetAnimationType();
	return
		at == RPG::EventPage::AnimType_continuous ||
		at == RPG::EventPage::AnimType_fixed_continuous;
}

bool Game_Character::IsSpinning() const {
	return GetAnimationType() == RPG::EventPage::AnimType_spin;
}

bool Game_Character::IsMiddleFrameAvailable() const {
	auto at = GetAnimationType();
	return
		at == RPG::EventPage::AnimType_non_continuous ||
		at == RPG::EventPage::AnimType_continuous ||
		at == RPG::EventPage::AnimType_fixed_non_continuous ||
		at == RPG::EventPage::AnimType_fixed_continuous;
}

int Game_Character::GetBushDepth() const {
	if ((GetLayer() != RPG::EventPage::Layers_same) || IsJumping() || IsFlying()) {
		return 0;
	}

	return Game_Map::GetBushDepth(GetX(), GetY());
}

void Game_Character::SetGraphic(const std::string& name, int index) {
	if (GetSpriteName() != name || GetSpriteIndex() != index) {
		SetSpriteName(name);
		SetSpriteIndex(index);
	}
}

void Game_Character::Flash(Color color, int duration) {
	SetFlashColor(color);
	SetFlashTimeLeft(duration);
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
			return Game_Map::GetEvent(event_id);
		default:
			// Other events
			return Game_Map::GetEvent(character_id);
	}
}

int Game_Character::ReverseDir(int dir) {
	constexpr static char reversed[] =
		{ Down, Left, Up, Right, DownLeft, UpLeft, UpRight, DownRight };
	return reversed[dir];
}


bool Game_Character::MakeWayDiagonal(int x, int y, int d) const {
	int dx = (d == UpRight || d == DownRight) - (d == DownLeft || d == UpLeft);
	int dy = (d == DownRight || d == DownLeft) - (d == UpRight || d == UpLeft);
	return ((MakeWay(x, y, dy + 1) && MakeWay(x, y + dy, -dx + 2)) ||
			(MakeWay(x, y, -dx + 2) && MakeWay(x + dx, y, dy + 1)));
}
