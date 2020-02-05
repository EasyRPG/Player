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
#include "drawable.h"
#include "player.h"
#include "utils.h"
#include "util_macro.h"
#include <cmath>
#include <cassert>

Game_Character::Game_Character(Type type, RPG::SaveMapEventBase* d) :
	move_failed(false),
	jump_plus_x(0),
	jump_plus_y(0),
	visible(true),
	_type(type),
	_data(d)
{
}

Game_Character::~Game_Character() {
	Game_Map::RemovePendingMove(this);
}

bool Game_Character::MakeWay(int x, int y) const {
	return Game_Map::MakeWay(*this, x, y);
}

void Game_Character::MoveTo(int x, int y) {
	SetX(Game_Map::RoundX(x));
	SetY(Game_Map::RoundY(y));
	SetRemainingStep(0);
	// This Fixes an RPG_RT bug where the jumping flag doesn't get reset
	// if you change maps during a jump
	SetJumping(false);
}

int Game_Character::GetJumpHeight() const {
	if (IsJumping()) {
		int jump_height = (GetRemainingStep() > SCREEN_TILE_SIZE / 2 ? SCREEN_TILE_SIZE - GetRemainingStep() : GetRemainingStep()) / 8;
		return (jump_height < 5 ? jump_height * 2 : jump_height < 13 ? jump_height + 4 : 16);
	}
	return 0;
}

int Game_Character::GetScreenX(bool apply_shift) const {
	int x = GetSpriteX() / TILE_SIZE - Game_Map::GetDisplayX() / TILE_SIZE + TILE_SIZE;

	if (Game_Map::LoopHorizontal()) {
		x = Utils::PositiveModulo(x, Game_Map::GetWidth() * TILE_SIZE);
	}
	x -= TILE_SIZE / 2;

	if (apply_shift) {
		x += Game_Map::GetWidth() * TILE_SIZE;
	}

	return x;
}

int Game_Character::GetScreenY(bool apply_shift, bool apply_jump) const {
	int y = GetSpriteY() / TILE_SIZE - Game_Map::GetDisplayY() / TILE_SIZE + TILE_SIZE;

	if (apply_jump) {
		y -= GetJumpHeight();
	}

	if (Game_Map::LoopVertical()) {
		y = Utils::PositiveModulo(y, Game_Map::GetHeight() * TILE_SIZE);
	}

	if (apply_shift) {
		y += Game_Map::GetHeight() * TILE_SIZE;
	}

	return y;
}

int Game_Character::GetScreenZ(bool apply_shift) const {
	int z = 0;

	if (IsFlying()) {
		z = Priority_EventsFlying;
	} else if (GetLayer() == RPG::EventPage::Layers_same) {
		z = Priority_Player;
	} else if (GetLayer() == RPG::EventPage::Layers_below) {
		z = Priority_EventsBelow;
	} else if (GetLayer() == RPG::EventPage::Layers_above) {
		z = Priority_EventsAbove;
	}

	// For events on the screen, this should be inside a 0-40 range
	z += GetScreenY(apply_shift, false) >> 3;

	return z;
}

void Game_Character::UpdateMovement() {
	if (IsMoveRouteOverwritten()) {
		UpdateMoveRoute(data()->move_route_index, data()->move_route);
	}

	UpdateSelfMovement();

	bool moved = false;

	if (IsJumping()) {
		SetStopCount(0);
		UpdateJump();
		moved = true;
	} else if (IsMoving()) {
		SetStopCount(0);
		SetRemainingStep(GetRemainingStep() - min(1 << (1 + GetMoveSpeed()), GetRemainingStep()));
		moved = true;
	} else {
		if (GetStopCount() == 0 || IsMoveRouteOverwritten() ||
					((Game_Message::GetContinueEvents() || !Game_Map::GetInterpreter().IsRunning()) && !IsPaused())) {
			SetStopCount(GetStopCount() + 1);
		}
	}

	// These actions happen after movement has finished but before stop count timer.
	if (IsStopping() && IsMoveRouteActive() && IsMoveRouteOverwritten()) {
		const auto& move_route = GetMoveRoute();
		if (!move_route.move_commands.empty() && GetMoveRouteIndex() >= static_cast<int>(move_route.move_commands.size())) {
			if (move_route.repeat) {
				SetMoveRouteRepeated(true);
				SetMoveRouteIndex(0);
			} else if(moved) {
				// If the last command of a move route is a move, and
				// the move just completed, cancel immediately and
				// don't wait for stop_count.
				CancelMoveRoute();
				SetMoveRouteIndex(0);
			}
		}
	}
}

void Game_Character::UpdateAnimation(bool was_moving) {
	if (IsAnimPaused()) {
		ResetAnimation();
		return;
	}

	if (!IsAnimated()) {
		return;
	}

	const auto step_idx = Utils::Clamp(GetMoveSpeed(), 1, 6) - 1;

	constexpr int spin_limits[] = { 23, 14, 11, 7, 5, 3 };
	constexpr int stationary_limits[] = { 11, 9, 7, 5, 4, 3 };
	constexpr int continuous_limits[] = { 15, 11, 9, 7, 6, 5 };

	if (IsSpinning()) {
		const auto limit = spin_limits[step_idx];

		IncAnimCount();

		if (GetAnimCount() > limit) {
			SetSpriteDirection((GetSpriteDirection() + 1) % 4);
			SetAnimCount(0);
		}
		return;
	}

	if (IsJumping()) {
		// Note: We start ticking animations right away on the last frame of the jump, not the frame after.
		// Hence there is no "was_jumping" to pass in here.
		ResetAnimation();
		return;
	}

	const auto stationary_limit = stationary_limits[step_idx];
	const auto continuous_limit = continuous_limits[step_idx];

	if (IsContinuous()
			|| was_moving
			|| data()->anim_frame == RPG::EventPage::Frame_left || data()->anim_frame == RPG::EventPage::Frame_right
			|| GetAnimCount() < stationary_limit) {
		IncAnimCount();
	}

	if (GetAnimCount() > continuous_limit
			|| (was_moving && GetAnimCount() > stationary_limit)) {
		IncAnimFrame();
		return;
	}
}

void Game_Character::UpdateFlash() {
	Flash::Update(data()->flash_current_level, data()->flash_time_left);
}

void Game_Character::UpdateJump() {
	static const int jump_speed[] = {8, 12, 16, 24, 32, 64};
	SetRemainingStep(GetRemainingStep() - min(jump_speed[GetMoveSpeed() - 1], GetRemainingStep()));
	if (GetRemainingStep() <= 0) {
		SetJumping(false);
	}
}

void Game_Character::UpdateMoveRoute(int32_t& current_index, const RPG::MoveRoute& current_route) {
	auto is_move_route_possible = [&]() {
		return IsStopping() && !IsStopCountActive();
	};

	const int last_move_route_index = current_route.move_commands.size();

	if (current_route.move_commands.empty()) {
		return;
	}

	if (!is_move_route_possible()) {
		return;
	}

	MoveOption option =
		current_route.skippable ? MoveOption::IgnoreIfCantMove : MoveOption::Normal;

	move_failed = false;

	int num_cmds_processed = 0;

	while (true) {
		//Move route is finished
		if (current_index >= last_move_route_index) {
			if (current_route.repeat) {
				if (IsMoveRouteOverwritten()) {
					SetMoveRouteRepeated(true);
				}
				current_index = 0;
			} else {
				if (IsMoveRouteOverwritten()) {
					CancelMoveRoute();
				}
				break;
			}

		}

		// Repeating move route will end after doing one full cycle in a single frame.
		if (num_cmds_processed >= static_cast<int>(current_route.move_commands.size())) {
			break;
		}

		const RPG::MoveCommand& move_command = current_route.move_commands[current_index];

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
				Move(GetDirectionToHero());
				break;
			case RPG::MoveCommand::Code::move_away_from_hero:
				Move(GetDirectionAwayHero());
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
				BeginJump(current_index, current_route);
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
				Main_Data::game_switches->Set(move_command.parameter_a, true);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
				Game_Map::Refresh();
				break;
			case RPG::MoveCommand::Code::switch_off: // Parameter A: Switch to turn off
				Main_Data::game_switches->Set(move_command.parameter_a, false);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
				Game_Map::Refresh();
				break;
			case RPG::MoveCommand::Code::change_graphic: // String: File, Parameter A: index
				SetSpriteGraphic(move_command.parameter_string, move_command.parameter_a);
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

		if (move_failed && !current_route.skippable) {
			break;
		}

		++current_index;
		++num_cmds_processed;

		if(!is_move_route_possible()) {
			break;
		}
	} // while (true)
}

void Game_Character::Move(int dir, MoveOption option) {
	int dx = (dir == Right || dir == UpRight || dir == DownRight) - (dir == Left || dir == DownLeft || dir == UpLeft);
	int dy = (dir == Down || dir == DownRight || dir == DownLeft) - (dir == Up || dir == UpRight || dir == UpLeft);

	if (IsJumping()) {
		jump_plus_x += dx;
		jump_plus_y += dy;
		return;
	}

	move_failed = !MakeWay(GetX() + dx, GetY() + dy);

	if (!move_failed || option == MoveOption::Normal) {
		SetDirection(dir);
		if (!(IsDirectionFixed() || IsFacingLocked() || IsSpinning())) {
			if (dir > 3) // Diagonal
				SetSpriteDirection(GetSpriteDirection() % 2 ? -dx + 2 : dy + 1);
			else
				SetSpriteDirection(dir);
		}
	}

	const auto new_x = Game_Map::RoundX(GetX() + dx);
	const auto new_y = Game_Map::RoundY(GetY() + dy);

	if (IsMoveRouteActive()) {
		SetMaxStopCountForStep();
	}

	if (move_failed) {
		OnMoveFailed(Game_Map::RoundX(GetX() + dx), Game_Map::RoundY(GetY() + dy));
		return;
	}

	SetX(new_x);
	SetY(new_y);
	SetRemainingStep(SCREEN_TILE_SIZE);
	BeginMove();

	SetStopCount(0);
}

void Game_Character::MoveForward(MoveOption option) {
	Move(GetDirection(), option);
}

void Game_Character::MoveRandom(MoveOption option) {
	Move(Utils::GetRandomNumber(0, 3), option);
}

void Game_Character::Turn(int dir) {
	SetDirection(dir);
	if (!IsJumping()) {
		SetSpriteDirection(dir);
	}
	move_failed = false;
	SetStopCount(0);
	SetMaxStopCountForTurn();
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

int Game_Character::GetDirectionToHero() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		return (sx > 0) ? Left : Right;
	} else {
		return (sy > 0) ? Up : Down;
	}
}

int Game_Character::GetDirectionAwayHero() {
	int sx = DistanceXfromPlayer();
	int sy = DistanceYfromPlayer();

	if ( std::abs(sx) > std::abs(sy) ) {
		return (sx > 0) ? Right : Left;
	} else {
		return (sy > 0) ? Down : Up;
	}
}

void Game_Character::TurnTowardHero() {
	Turn(GetDirectionToHero());
}

void Game_Character::TurnAwayFromHero() {
	Turn(GetDirectionAwayHero());
}

void Game_Character::FaceRandomDirection() {
	Turn(Utils::GetRandomNumber(0, 3));
}

void Game_Character::Wait() {
	SetStopCount(0);
	SetMaxStopCountForWait();
}

void Game_Character::BeginJump(int32_t& current_index, const RPG::MoveRoute& current_route) {
	jump_plus_x = 0;
	jump_plus_y = 0;
	SetJumping(true);

	bool end_found = false;
	unsigned int i;
	for (i = current_index; i < current_route.move_commands.size(); ++i) {
		const RPG::MoveCommand& move_command = current_route.move_commands[i];
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
				Move(GetDirectionToHero());
				break;
			case RPG::MoveCommand::Code::move_away_from_hero:
				Move(GetDirectionAwayHero());
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
		current_index = i;
		SetJumping(false);
		return;
	}

	int new_x = GetX() + jump_plus_x;
	int new_y = GetY() + jump_plus_y;

	if (!MakeWay(new_x, new_y)) {
		move_failed = true;
	}

	if (!move_failed || !current_route.skippable) {
		if (jump_plus_x != 0 || jump_plus_y != 0) {
			if (std::abs(jump_plus_y) >= std::abs(jump_plus_x)) {
				SetDirection(jump_plus_y > 0 ? Down : Up);
				if (!IsDirectionFixed() && !IsFacingLocked()) {
					SetSpriteDirection(GetDirection());
				}
			} else {
				SetDirection(jump_plus_x > 0 ? Right : Left);
				if (!IsDirectionFixed() && !IsFacingLocked()) {
					SetSpriteDirection(GetDirection());
				}
			}
		}
	}

	if (move_failed) {
		// Reset to begin jump command and try again...
		SetJumping(false);

		if (current_route.skippable) {
			current_index = i;
			return;
		}

		return;
	}

	int begin_x = GetX();
	int begin_y = GetY();

	// Adjust positions for looping maps. jump begin positions
	// get set off the edge of the map to preserve direction.
	if (Game_Map::LoopHorizontal()
			&& (new_x < 0 || new_x >= Game_Map::GetWidth()))
	{
		auto old_x = new_x;
		new_x = Game_Map::RoundX(new_x);
		begin_x += new_x - old_x;
	}

	if (Game_Map::LoopVertical()
			&& (new_y < 0 || new_y >= Game_Map::GetHeight()))
	{
		auto old_y = new_y;
		new_y = Game_Map::RoundY(new_y);
		begin_y += new_y - old_y;
	}

	SetBeginJumpX(begin_x);
	SetBeginJumpY(begin_y);
	SetX(new_x);
	SetY(new_y);
	current_index = i;

	SetRemainingStep(SCREEN_TILE_SIZE);
	SetStopCount(0);
	SetMaxStopCountForStep();
	move_failed = false;
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
	const auto prev_max_sc = GetMaxStopCount();
	if (IsMoveRouteActive()) {
		CancelMoveRoute();
	}

	SetPaused(false);
	SetStopCount(0xFFFF);
	SetMoveRouteIndex(0);
	SetMoveRouteRepeated(false);
	SetMoveRoute(new_route);

	if (GetMoveRoute().move_commands.empty()) {
		// Matches RPG_RT behavior
		SetMaxStopCountForStep();
		return;
	}

	Game_Map::AddPendingMove(this);

	original_move_frequency = GetMoveFrequency();

	SetMoveRouteOverwritten(true);
	SetMoveFrequency(frequency);

	if (frequency != original_move_frequency) {
		SetMaxStopCountForStep();
	} else {
		SetMaxStopCount(prev_max_sc);
	}
}

void Game_Character::CancelMoveRoute() {
	Game_Map::RemovePendingMove(this);
	SetMoveRouteOverwritten(false);
	SetMoveRouteRepeated(false);
	SetMoveFrequency(original_move_frequency);
	SetMaxStopCountForStep();
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
	} else if (IsJumping()) {
		x -= ((GetX() - GetBeginJumpX()) * GetRemainingStep());
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
	} else if (IsJumping()) {
		y -= (GetY() - GetBeginJumpY()) * GetRemainingStep();
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

bool Game_Character::IsAnimated() const {
	auto at = GetAnimationType();
	return !IsAnimPaused()
		&& at != RPG::EventPage::AnimType_fixed_graphic
		&& at != RPG::EventPage::AnimType_step_frame_fix;
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

int Game_Character::GetBushDepth() const {
	if ((GetLayer() != RPG::EventPage::Layers_same) || IsJumping() || IsFlying()) {
		return 0;
	}

	return Game_Map::GetBushDepth(GetX(), GetY());
}

void Game_Character::Flash(int r, int g, int b, int power, int frames) {
	data()->flash_red = r;
	data()->flash_green = g;
	data()->flash_blue = b;
	data()->flash_current_level = power;
	data()->flash_time_left = frames;
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

void Game_Character::SetMaxStopCountForStep() {
	const auto freq = GetMoveFrequency();
	SetMaxStopCount(freq >= 8 ? 0 : 1 << (9 - freq));
}

void Game_Character::SetMaxStopCountForTurn() {
	const auto freq = GetMoveFrequency();
	SetMaxStopCount(freq >= 8 ? 0 : 1 << (8 - freq));
}

void Game_Character::SetMaxStopCountForWait() {
	const auto freq = GetMoveFrequency();
	SetMaxStopCount(20 + (freq >= 8 ? 0 : 1 << (8 - freq)));
}

bool Game_Character::IsMoveRouteActive() const {
	return IsMoveRouteOverwritten();
}


int Game_Character::GetVehicleType() const {
	return 0;
}

void Game_Character::SetActive(bool active) {
	data()->active = active;
	SetVisible(active);
}

