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
	_type(type), _data(d)
{
}

Game_Character::~Game_Character() {
	Game_Map::RemovePendingMove(this);
}

void Game_Character::MoveTo(int map_id, int x, int y) {
	data()->map_id = map_id;
	// RPG_RT does not round the position for this function.
	SetX(x);
	SetY(y);
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

void Game_Character::Update() {
	if (!IsActive() || IsProcessed()) {
		return;
	}
	SetProcessed(true);

	if (IsStopping()) {
		this->UpdateNextMovementAction();
	}
	UpdateFlash();

	if (IsStopping()) {
		if (GetStopCount() == 0 || IsMoveRouteOverwritten() ||
				((Game_Message::GetContinueEvents() || !Game_Map::GetInterpreter().IsRunning()) && !IsPaused())) {
			SetStopCount(GetStopCount() + 1);
		}
	} else if (IsJumping()) {
		static const int jump_speed[] = {8, 12, 16, 24, 32, 64};
		auto amount = jump_speed[GetMoveSpeed() -1 ];
		this->UpdateMovement(amount);
	} else {
		int amount = 1 << (1 + GetMoveSpeed());
		this->UpdateMovement(amount);
	}

	this->UpdateAnimation();
}

void Game_Character::UpdateMovement(int amount) {
	SetRemainingStep(GetRemainingStep() - amount);
	if (GetRemainingStep() <= 0) {
		SetRemainingStep(0);
		SetJumping(0);

		// FIXME: Empty check?
		auto& move_route = GetMoveRoute();
		if (IsMoveRouteOverwritten() && GetMoveRouteIndex() >= static_cast<int>(move_route.move_commands.size())) {
			SetMoveRouteRepeated(true);
			SetMoveRouteIndex(0);
			if (!move_route.repeat) {
				// If the last command of a move route is or jump,
				// RPG_RT cancels the entire move route immediately.
				CancelMoveRoute();
			}
		}
	}

	SetStopCount(0);
}

void Game_Character::UpdateAnimation() {
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

	if (IsAnimPaused() || IsJumping()) {
		ResetAnimation();
		return;
	}

	if (!IsAnimated()) {
		return;
	}

	const auto stationary_limit = stationary_limits[step_idx];
	const auto continuous_limit = continuous_limits[step_idx];

	// FIXME: Verify this
	if (IsContinuous()
			|| GetStopCount() == 0
			|| data()->anim_frame == RPG::EventPage::Frame_left || data()->anim_frame == RPG::EventPage::Frame_right
			|| GetAnimCount() < stationary_limit) {
		IncAnimCount();
	}

	if (GetAnimCount() > continuous_limit
			|| (GetStopCount() == 0 && GetAnimCount() > stationary_limit)) {
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

void Game_Character::UpdateMoveRoute(int32_t& current_index, const RPG::MoveRoute& current_route, bool is_overwrite) {
	if (current_route.move_commands.empty()) {
		return;
	}

	if (is_overwrite && !IsMoveRouteOverwritten()) {
		return;
	}

	const int start_index = current_index;

	while (true) {
		if (!IsStopping() || IsStopCountActive()) {
			return;
		}

		//Move route is finished
		if (current_index >= static_cast<int>(current_route.move_commands.size())) {
			if (is_overwrite) {
				SetMoveRouteRepeated(true);
			}
			if (!current_route.repeat) {
				if (is_overwrite) {
					CancelMoveRoute();
				}
				return;
			}
			current_index = 0;
			if (current_index == start_index) {
				return;
			}
		}

		const RPG::MoveCommand& move_command = current_route.move_commands[current_index];
		const auto prev_direction = GetDirection();
		const auto prev_facing = GetSpriteDirection();
		const auto saved_index = current_index;
		const auto cmd = move_command.command_id;

		if (cmd >= RPG::MoveCommand::Code::move_up && cmd <= RPG::MoveCommand::Code::move_forward) {
			switch (move_command.command_id) {
				case RPG::MoveCommand::Code::move_up:
				case RPG::MoveCommand::Code::move_right:
				case RPG::MoveCommand::Code::move_down:
				case RPG::MoveCommand::Code::move_left:
				case RPG::MoveCommand::Code::move_upright:
				case RPG::MoveCommand::Code::move_downright:
				case RPG::MoveCommand::Code::move_downleft:
				case RPG::MoveCommand::Code::move_upleft:
					SetDirection(move_command.command_id);
					break;
				case RPG::MoveCommand::Code::move_random:
					TurnRandom();
					break;
				case RPG::MoveCommand::Code::move_towards_hero:
					TurnTowardHero();
					break;
				case RPG::MoveCommand::Code::move_away_from_hero:
					TurnAwayFromHero();
					break;
				case RPG::MoveCommand::Code::move_forward:
					break;
				default:
					break;
			}
			Move(GetDirection());

			if (IsStopping()) {
				// Move failed
				if (current_route.skippable) {
					SetDirection(prev_direction);
					SetSpriteDirection(prev_facing);
				} else {
					return;
				}
			}
			// FIXME: Necessary?
			if (cmd == RPG::MoveCommand::Code::move_forward) {
				SetSpriteDirection(prev_facing);
			}

			// FIXME: Review how stop counts are handled
			SetMaxStopCountForStep();
		} else if (cmd >= RPG::MoveCommand::Code::face_up && cmd <= RPG::MoveCommand::Code::face_away_from_hero) {
			switch (move_command.command_id) {
				case RPG::MoveCommand::Code::face_up:
					SetDirection(Up);
					break;
				case RPG::MoveCommand::Code::face_right:
					SetDirection(Right);
					break;
				case RPG::MoveCommand::Code::face_down:
					SetDirection(Down);
					break;
				case RPG::MoveCommand::Code::face_left:
					SetDirection(Left);
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
					TurnRandom();
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
			SetSpriteDirection(GetDirection());
			SetMaxStopCountForTurn();
			SetStopCount(0);
		} else {
			switch (move_command.command_id) {
				case RPG::MoveCommand::Code::wait:
					SetMaxStopCountForWait();
					SetStopCount(0);
					break;
				case RPG::MoveCommand::Code::begin_jump:
					BeginJump(current_index, current_route);
					if (IsStopping()) {
						// Jump failed
						if (current_route.skippable) {
							SetDirection(prev_direction);
							SetSpriteDirection(prev_facing);
						} else {
							current_index = saved_index;
							return;
						}
					}
					break;
				case RPG::MoveCommand::Code::end_jump:
					// EndJump();
					break;
				case RPG::MoveCommand::Code::lock_facing:
					SetFacingLocked(true);
					break;
				case RPG::MoveCommand::Code::unlock_facing:
					// FIXME: Verify this behavior
					if (!IsDirectionFixedAnimationType(GetAnimationType())) {
						SetFacingLocked(false);
					}
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
					Game_Map::SetNeedRefresh(true);
					Game_Map::Refresh();
					break;
				case RPG::MoveCommand::Code::switch_off: // Parameter A: Switch to turn off
					Main_Data::game_switches->Set(move_command.parameter_a, false);
					Game_Map::SetNeedRefresh(true);
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
		}
		// FIXME: RPG_RT will hang forever on bad command code.
		// Should we emulate this?
		++current_index;

		// FIXME: If a jump skips past start_index, we will repeat longer?
		if (current_index == start_index) {
			return;
		}
	} // while (true)
}


bool Game_Character::MakeWay(int from_x, int from_y, int to_x, int to_y) {
	return Game_Map::MakeWay(*this, from_x, from_y, to_x, to_y);
}

void Game_Character::Move(int dir) {
	int dx = GetDxFromDirection(dir);
	int dy = GetDyFromDirection(dir);

	bool move_success = false;

	SetDirection(dir);
	// RPG_RT only does the IsSpinning() check for Game_Event. We did it for all types here
	// in order to avoid a virtual call and because normally with RPG_RT, spinning
	// player or vehicle is impossible.
	if (!(IsFacingLocked() || IsSpinning())) {
		if (dir > 3) // Diagonal
			SetSpriteDirection(GetSpriteDirection() % 2 ? -dx + 2 : dy + 1);
		else
			SetSpriteDirection(dir);
	}

	auto makeX = [&]() { return MakeWay(GetX(), GetY(), GetX() + dx, GetY()); };
	auto makeY = [&]() { return MakeWay(GetX(), GetY(), GetX(), GetY() + dy); };

	if (dx && dy) {
		// For diagonal movement, RPG_RT checks if we can reach the tile using (vert, horiz), and then (horiz, vert).
		move_success = (makeY() && makeX()) || (makeX() && makeY());
	} else if (dx) {
		move_success = makeX();
	} else if (dy) {
		move_success = makeY();
	}

	if (!move_success) {
		return;
	}

	const auto new_x = Game_Map::RoundX(GetX() + dx);
	const auto new_y = Game_Map::RoundY(GetY() + dy);

	SetX(new_x);
	SetY(new_y);
	SetRemainingStep(SCREEN_TILE_SIZE);
	// FIXME: This happens elsewhere?
	SetStopCount(0);
}

void Game_Character::Turn90DegreeLeft() {
	SetDirection(GetDirection90DegreeLeft(GetSpriteDirection()));
}

void Game_Character::Turn90DegreeRight() {
	SetDirection(GetDirection90DegreeRight(GetSpriteDirection()));
}

void Game_Character::Turn180Degree() {
	SetDirection(GetDirection180Degree(GetSpriteDirection()));
}

void Game_Character::Turn90DegreeLeftOrRight() {
	if (Utils::ChanceOf(1,2)) {
		Turn90DegreeLeft();
	} else {
		Turn90DegreeRight();
	}
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
	SetDirection(GetDirectionToHero());
}

void Game_Character::TurnAwayFromHero() {
	SetDirection(GetDirectionAwayHero());
}

void Game_Character::TurnRandom() {
	SetDirection(Utils::GetRandomNumber(0, 3));
}

void Game_Character::Wait() {
	SetStopCount(0);
	SetMaxStopCountForWait();
}

void Game_Character::BeginJump(int32_t& current_index, const RPG::MoveRoute& current_route) {
	int jdx = 0;
	int jdy = 0;

	bool end_found = false;
	for (++current_index; current_index < current_route.move_commands.size(); ++current_index) {
		const RPG::MoveCommand& move_command = current_route.move_commands[current_index];
		const auto cmd = move_command.command_id;
		if (cmd >= RPG::MoveCommand::Code::move_up && cmd <= RPG::MoveCommand::Code::move_forward) {
			switch (move_command.command_id) {
				case RPG::MoveCommand::Code::move_up:
				case RPG::MoveCommand::Code::move_right:
				case RPG::MoveCommand::Code::move_down:
				case RPG::MoveCommand::Code::move_left:
				case RPG::MoveCommand::Code::move_upright:
				case RPG::MoveCommand::Code::move_downright:
				case RPG::MoveCommand::Code::move_downleft:
				case RPG::MoveCommand::Code::move_upleft:
					SetDirection(move_command.command_id);
					break;
				case RPG::MoveCommand::Code::move_random:
					TurnRandom();
					break;
				case RPG::MoveCommand::Code::move_towards_hero:
					TurnTowardHero();
					break;
				case RPG::MoveCommand::Code::move_away_from_hero:
					TurnAwayFromHero();
					break;
				case RPG::MoveCommand::Code::move_forward:
					break;
				default:
					break;
			}
			jdx += GetDxFromDirection(GetDirection());
			jdy += GetDyFromDirection(GetDirection());
		}

		if (cmd >= RPG::MoveCommand::Code::face_up && cmd <= RPG::MoveCommand::Code::face_away_from_hero) {
			switch (move_command.command_id) {
				case RPG::MoveCommand::Code::face_up:
					SetDirection(Up);
					break;
				case RPG::MoveCommand::Code::face_right:
					SetDirection(Right);
					break;
				case RPG::MoveCommand::Code::face_down:
					SetDirection(Down);
					break;
				case RPG::MoveCommand::Code::face_left:
					SetDirection(Left);
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
					TurnRandom();
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
		}

		if (move_command.command_id == RPG::MoveCommand::Code::end_jump) {
			end_found = true;
			// Note: outer function increment will cause the end jump to past after the return.
			break;
		}
	}

	int new_x = GetX() + jdx;
	int new_y = GetY() + jdy;

	// FIXME: Test all these
	if (std::abs(jdy) >= std::abs(jdx)) {
		SetDirection(jdy >= 0 ? Down : Up);
	} else {
		SetDirection(jdx >= 0 ? Right : Left);
	}

	SetJumping(true);

	if (jdx != 0 || jdy != 0) {
		if (!IsFacingLocked()) {
			SetSpriteDirection(GetDirection());
		}

		// FIXME: Remove dependency on jump from within Game_Map::MakeWay
		if (!MakeWay(GetX(), GetY(), new_x, new_y)) {
			SetJumping(false);
			return;
		}
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
	SetJumping(true);
	SetRemainingStep(SCREEN_TILE_SIZE);

	SetStopCount(0);
	SetMaxStopCountForStep();
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
	if (!IsMoveRouteOverwritten()) {
		original_move_frequency = GetMoveFrequency();
	} else {
		Game_Map::RemovePendingMove(this);
	}

	SetPaused(false);
	SetStopCount(0xFFFF);
	SetMoveRouteIndex(0);
	SetMoveRouteRepeated(false);
	SetMoveFrequency(frequency);
	SetMoveRouteOverwritten(true);
	SetMoveRoute(new_route);
	if (frequency != original_move_frequency) {
		SetMaxStopCountForStep();
	}

	if (GetMoveRoute().move_commands.empty()) {
		CancelMoveRoute();
		return;
	}

	Game_Map::AddPendingMove(this);
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
	SetMaxStopCount(GetMaxStopCountForStep(GetMoveFrequency()));
}

void Game_Character::SetMaxStopCountForTurn() {
	SetMaxStopCount(GetMaxStopCountForTurn(GetMoveFrequency()));
}

void Game_Character::SetMaxStopCountForWait() {
	SetMaxStopCount(GetMaxStopCountForWait(GetMoveFrequency()));
}

int Game_Character::GetVehicleType() const {
	return 0;
}

void Game_Character::SetActive(bool active) {
	data()->active = active;
	SetVisible(active);
}

