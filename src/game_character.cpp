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
#include "output.h"
#include "rand.h"
#include <cmath>
#include <cassert>
#include <unordered_set>

Game_Character::Game_Character(Type type, lcf::rpg::SaveMapEventBase* d) :
	_type(type), _data(d)
{
}

Game_Character::~Game_Character() {
}

void Game_Character::SanitizeData(StringView name) {
	SanitizeMoveRoute(name, data()->move_route, data()->move_route_index, "move_route_index");
}

void Game_Character::SanitizeMoveRoute(StringView name, const lcf::rpg::MoveRoute& mr, int32_t& idx, StringView chunk_name) {
	const auto n = static_cast<int32_t>(mr.move_commands.size());
	if (idx < 0 || idx > n) {
		idx = n;
		Output::Warning("{} {}: Save Data invalid {}={}. Fixing ...", TypeToStr(_type), name, chunk_name, idx);
	}
}

void Game_Character::MoveTo(int map_id, int x, int y) {
	data()->map_id = map_id;
	// RPG_RT does not round the position for this function.
	SetX(x);
	SetY(y);
	SetRemainingStep(0);
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
		x = Utils::PositiveModulo(x, Game_Map::GetTilesX() * TILE_SIZE);
	}
	x -= TILE_SIZE / 2;

	if (apply_shift) {
		x += Game_Map::GetTilesX() * TILE_SIZE;
	}

	return x;
}

int Game_Character::GetScreenY(bool apply_shift, bool apply_jump) const {
	int y = GetSpriteY() / TILE_SIZE - Game_Map::GetDisplayY() / TILE_SIZE + TILE_SIZE;

	if (apply_jump) {
		y -= GetJumpHeight();
	}

	if (Game_Map::LoopVertical()) {
		y = Utils::PositiveModulo(y, Game_Map::GetTilesY() * TILE_SIZE);
	}

	if (apply_shift) {
		y += Game_Map::GetTilesY() * TILE_SIZE;
	}

	return y;
}

Drawable::Z_t Game_Character::GetScreenZ(bool apply_shift) const {
	Drawable::Z_t z = 0;

	if (IsFlying()) {
		z = Priority_EventsFlying;
	} else if (GetLayer() == lcf::rpg::EventPage::Layers_same) {
		z = Priority_Player;
	} else if (GetLayer() == lcf::rpg::EventPage::Layers_below) {
		z = Priority_EventsBelow;
	} else if (GetLayer() == lcf::rpg::EventPage::Layers_above) {
		z = Priority_EventsAbove;
	}

	// 0x8000 (32768) is added to shift negative numbers into the positive range
	Drawable::Z_t y = static_cast<Drawable::Z_t>(GetScreenY(apply_shift, false) + 0x8000);
	Drawable::Z_t x = static_cast<Drawable::Z_t>(GetScreenX(apply_shift) + 0x8000);

	// The rendering order of characters is: Highest Y-coordinate, Highest X-coordinate, Highest ID
	// To encode this behaviour all of them get 16 Bit in the Z value
	// L- YY XX II (1 letter = 8 bit)
	// L: Layer (specified by the event page)
	// -: Unused
	// Y: Y-coordinate
	// X: X-coordinate
	// I: ID (This is only applied by subclasses, characters itself put nothing (0) here
	z += (y << 32) + (x << 16);

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
				((Main_Data::game_system->GetMessageContinueEvents() || !Game_Map::GetInterpreter().IsRunning()) && !IsPaused())) {
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
		SetJumping(false);

		auto& move_route = GetMoveRoute();
		if (IsMoveRouteOverwritten() && GetMoveRouteIndex() >= static_cast<int>(move_route.move_commands.size())) {
			SetMoveRouteFinished(true);
			SetMoveRouteIndex(0);
			if (!move_route.repeat) {
				// If the last command of a move route is a move or jump,
				// RPG_RT cancels the entire move route immediately.
				CancelMoveRoute();
			}
		}
	}

	SetStopCount(0);
}

void Game_Character::UpdateAnimation() {
	const auto speed = Utils::Clamp(GetMoveSpeed(), 1, 6);

	if (IsSpinning()) {
		const auto limit = GetSpinAnimFrames(speed);

		IncAnimCount();

		if (GetAnimCount() >= limit) {
			SetFacing((GetFacing() + 1) % 4);
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

	const auto stationary_limit = GetStationaryAnimFrames(speed);
	const auto continuous_limit = GetContinuousAnimFrames(speed);

	if (IsContinuous()
			|| GetStopCount() == 0
			|| data()->anim_frame == lcf::rpg::EventPage::Frame_left || data()->anim_frame == lcf::rpg::EventPage::Frame_right
			|| GetAnimCount() < stationary_limit - 1) {
		IncAnimCount();
	}

	if (GetAnimCount() >= continuous_limit
			|| (GetStopCount() == 0 && GetAnimCount() >= stationary_limit)) {
		IncAnimFrame();
		return;
	}
}

void Game_Character::UpdateFlash() {
	Flash::Update(data()->flash_current_level, data()->flash_time_left);
}

void Game_Character::UpdateMoveRoute(int32_t& current_index, const lcf::rpg::MoveRoute& current_route, bool is_overwrite) {
	if (current_route.move_commands.empty()) {
		return;
	}

	if (is_overwrite && !IsMoveRouteOverwritten()) {
		return;
	}

	const auto num_commands = static_cast<int>(current_route.move_commands.size());
	// Invalid index could occur from a corrupted save game.
	// Player, Vehicle, and Event all check for and fix this, but we still assert here in
	// case any bug causes this to happen still.
	assert(current_index >= 0);
	assert(current_index <= num_commands);

	const auto start_index = current_index;

	while (true) {
		if (!IsStopping() || IsStopCountActive()) {
			return;
		}

		//Move route is finished
		if (current_index >= num_commands) {
			if (is_overwrite) {
				SetMoveRouteFinished(true);
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

		using Code = lcf::rpg::MoveCommand::Code;
		const auto& move_command = current_route.move_commands[current_index];
		const auto prev_direction = GetDirection();
		const auto prev_facing = GetFacing();
		const auto saved_index = current_index;
		const auto cmd = static_cast<Code>(move_command.command_id);

		if (cmd >= Code::move_up && cmd <= Code::move_forward) {
			switch (cmd) {
				case Code::move_up:
				case Code::move_right:
				case Code::move_down:
				case Code::move_left:
				case Code::move_upright:
				case Code::move_downright:
				case Code::move_downleft:
				case Code::move_upleft:
					SetDirection(static_cast<Game_Character::Direction>(cmd));
					break;
				case Code::move_random:
					TurnRandom();
					break;
				case Code::move_towards_hero:
					TurnTowardHero();
					break;
				case Code::move_away_from_hero:
					TurnAwayFromHero();
					break;
				case Code::move_forward:
					break;
				default:
					break;
			}
			Move(GetDirection());

			if (IsStopping()) {
				// Move failed
				if (current_route.skippable) {
					SetDirection(prev_direction);
					SetFacing(prev_facing);
				} else {
					return;
				}
			}
			if (cmd == Code::move_forward) {
				SetFacing(prev_facing);
			}

			SetMaxStopCountForStep();
		} else if (cmd >= Code::face_up && cmd <= Code::face_away_from_hero) {
			SetDirection(GetFacing());
			switch (cmd) {
				case Code::face_up:
					SetDirection(Up);
					break;
				case Code::face_right:
					SetDirection(Right);
					break;
				case Code::face_down:
					SetDirection(Down);
					break;
				case Code::face_left:
					SetDirection(Left);
					break;
				case Code::turn_90_degree_right:
					Turn90DegreeRight();
					break;
				case Code::turn_90_degree_left:
					Turn90DegreeLeft();
					break;
				case Code::turn_180_degree:
					Turn180Degree();
					break;
				case Code::turn_90_degree_random:
					Turn90DegreeLeftOrRight();
					break;
				case Code::face_random_direction:
					TurnRandom();
					break;
				case Code::face_hero:
					TurnTowardHero();
					break;
				case Code::face_away_from_hero:
					TurnAwayFromHero();
					break;
				default:
					break;
			}
			SetFacing(GetDirection());
			SetMaxStopCountForTurn();
			SetStopCount(0);
		} else {
			switch (cmd) {
				case Code::wait:
					SetMaxStopCountForWait();
					SetStopCount(0);
					break;
				case Code::begin_jump:
					if (!BeginMoveRouteJump(current_index, current_route)) {
						// Jump failed
						if (current_route.skippable) {
							SetDirection(prev_direction);
							SetFacing(prev_facing);
						} else {
							current_index = saved_index;
							return;
						}
					}
					break;
				case Code::end_jump:
					break;
				case Code::lock_facing:
					SetFacingLocked(true);
					break;
				case Code::unlock_facing:
					SetFacingLocked(false);
					break;
				case Code::increase_movement_speed:
					SetMoveSpeed(min(GetMoveSpeed() + 1, 6));
					break;
				case Code::decrease_movement_speed:
					SetMoveSpeed(max(GetMoveSpeed() - 1, 1));
					break;
				case Code::increase_movement_frequence:
					SetMoveFrequency(min(GetMoveFrequency() + 1, 8));
					break;
				case Code::decrease_movement_frequence:
					SetMoveFrequency(max(GetMoveFrequency() - 1, 1));
					break;
				case Code::switch_on: // Parameter A: Switch to turn on
					Main_Data::game_switches->Set(move_command.parameter_a, true);
					++current_index; // In case the current_index is already 0 ...
					Game_Map::SetNeedRefresh(true);
					Game_Map::Refresh();
					// If page refresh has reset the current move route, abort now.
					if (current_index == 0) {
						return;
					}
					--current_index;
					break;
				case Code::switch_off: // Parameter A: Switch to turn off
					Main_Data::game_switches->Set(move_command.parameter_a, false);
					++current_index; // In case the current_index is already 0 ...
					Game_Map::SetNeedRefresh(true);
					Game_Map::Refresh();
					// If page refresh has reset the current move route, abort now.
					if (current_index == 0) {
						return;
					}
					--current_index;
					break;
				case Code::change_graphic: // String: File, Parameter A: index
					MoveRouteSetSpriteGraphic(ToString(move_command.parameter_string), move_command.parameter_a);
					break;
				case Code::play_sound_effect: // String: File, Parameters: Volume, Tempo, Balance
					if (move_command.parameter_string != "(OFF)" && move_command.parameter_string != "(Brak)") {
						lcf::rpg::Sound sound;
						sound.name = ToString(move_command.parameter_string);
						sound.volume = move_command.parameter_a;
						sound.tempo = move_command.parameter_b;
						sound.balance = move_command.parameter_c;

						Main_Data::game_system->SePlay(sound);
					}
					break;
				case Code::walk_everywhere_on:
					SetThrough(true);
					data()->move_route_through = true;
					break;
				case Code::walk_everywhere_off:
					SetThrough(false);
					data()->move_route_through = false;
					break;
				case Code::stop_animation:
					SetAnimPaused(true);
					break;
				case Code::start_animation:
					SetAnimPaused(false);
					break;
				case Code::increase_transp:
					SetTransparency(GetTransparency() + 1);
					break;
				case Code::decrease_transp:
					SetTransparency(GetTransparency() - 1);
					break;
				default:
					break;
			}
		}
		++current_index;

		if (current_index == start_index) {
			return;
		}
	} // while (true)
}


bool Game_Character::MakeWay(int from_x, int from_y, int to_x, int to_y) {
	return Game_Map::MakeWay(*this, from_x, from_y, to_x, to_y);
}


bool Game_Character::CheckWay(int from_x, int from_y, int to_x, int to_y) {
	return Game_Map::CheckWay(*this, from_x, from_y, to_x, to_y);
}


bool Game_Character::CheckWay(
		int from_x, int from_y, int to_x, int to_y, bool ignore_all_events,
		std::unordered_set<int> *ignore_some_events_by_id) {
	return Game_Map::CheckWay(*this, from_x, from_y, to_x, to_y,
		ignore_all_events, ignore_some_events_by_id);
}


bool Game_Character::Move(int dir) {
	if (!IsStopping()) {
		return true;
	}

	bool move_success = false;

	SetDirection(dir);
	UpdateFacing();

	const auto x = GetX();
	const auto y = GetY();
	const auto dx = GetDxFromDirection(dir);
	const auto dy = GetDyFromDirection(dir);

	if (dx && dy) {
		// For diagonal movement, RPG_RT trys vert -> horiz and if that fails, then horiz -> vert.
		move_success = (MakeWay(x, y, x, y + dy) && MakeWay(x, y + dy, x + dx, y + dy))
					|| (MakeWay(x, y, x + dx, y) && MakeWay(x + dx, y, x + dx, y + dy));
	} else if (dx) {
		move_success = MakeWay(x, y, x + dx, y);
	} else if (dy) {
		move_success = MakeWay(x, y, x, y + dy);
	}

	if (!move_success) {
		return false;
	}

	const auto new_x = Game_Map::RoundX(x + dx);
	const auto new_y = Game_Map::RoundY(y + dy);

	SetX(new_x);
	SetY(new_y);
	SetRemainingStep(SCREEN_TILE_SIZE);

	return true;
}

void Game_Character::Turn90DegreeLeft() {
	SetDirection(GetDirection90DegreeLeft(GetDirection()));
}

void Game_Character::Turn90DegreeRight() {
	SetDirection(GetDirection90DegreeRight(GetDirection()));
}

void Game_Character::Turn180Degree() {
	SetDirection(GetDirection180Degree(GetDirection()));
}

void Game_Character::Turn90DegreeLeftOrRight() {
	if (Rand::ChanceOf(1,2)) {
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
	SetDirection(Rand::GetRandomNumber(0, 3));
}

void Game_Character::Wait() {
	SetStopCount(0);
	SetMaxStopCountForWait();
}

bool Game_Character::BeginMoveRouteJump(int32_t& current_index, const lcf::rpg::MoveRoute& current_route) {
	int jdx = 0;
	int jdy = 0;

	for (++current_index; current_index < static_cast<int>(current_route.move_commands.size()); ++current_index) {
		using Code = lcf::rpg::MoveCommand::Code;
		const auto& move_command = current_route.move_commands[current_index];
		const auto cmd = static_cast<Code>(move_command.command_id);
		if (cmd >= Code::move_up && cmd <= Code::move_forward) {
			switch (cmd) {
				case Code::move_up:
				case Code::move_right:
				case Code::move_down:
				case Code::move_left:
				case Code::move_upright:
				case Code::move_downright:
				case Code::move_downleft:
				case Code::move_upleft:
					SetDirection(move_command.command_id);
					break;
				case Code::move_random:
					TurnRandom();
					break;
				case Code::move_towards_hero:
					TurnTowardHero();
					break;
				case Code::move_away_from_hero:
					TurnAwayFromHero();
					break;
				case Code::move_forward:
					break;
				default:
					break;
			}
			jdx += GetDxFromDirection(GetDirection());
			jdy += GetDyFromDirection(GetDirection());
		}

		if (cmd >= Code::face_up && cmd <= Code::face_away_from_hero) {
			switch (cmd) {
				case Code::face_up:
					SetDirection(Up);
					break;
				case Code::face_right:
					SetDirection(Right);
					break;
				case Code::face_down:
					SetDirection(Down);
					break;
				case Code::face_left:
					SetDirection(Left);
					break;
				case Code::turn_90_degree_right:
					Turn90DegreeRight();
					break;
				case Code::turn_90_degree_left:
					Turn90DegreeLeft();
					break;
				case Code::turn_180_degree:
					Turn180Degree();
					break;
				case Code::turn_90_degree_random:
					Turn90DegreeLeftOrRight();
					break;
				case Code::face_random_direction:
					TurnRandom();
					break;
				case Code::face_hero:
					TurnTowardHero();
					break;
				case Code::face_away_from_hero:
					TurnAwayFromHero();
					break;
				default:
					break;
			}
		}

		if (cmd == Code::end_jump) {
			int new_x = GetX() + jdx;
			int new_y = GetY() + jdy;

			auto rc = Jump(new_x, new_y);
			if (rc) {
				SetMaxStopCountForStep();
			}
			// Note: outer function increment will cause the end jump to pass after the return.
			return rc;
		}
	}

	// Commands finished with no end jump. Back up the index by 1 to allow outer loop increment to work.
	--current_index;

	// Jump is skipped
	return true;
}

bool Game_Character::Jump(int x, int y) {
	if (!IsStopping()) {
		return true;
	}

	auto begin_x = GetX();
	auto begin_y = GetY();
	const auto dx = x - begin_x;
	const auto dy = y - begin_y;

	if (std::abs(dy) >= std::abs(dx)) {
		SetDirection(dy >= 0 ? Down : Up);
	} else {
		SetDirection(dx >= 0 ? Right : Left);
	}

	SetJumping(true);

	if (dx != 0 || dy != 0) {
		if (!IsFacingLocked()) {
			SetFacing(GetDirection());
		}

		// FIXME: Remove dependency on jump from within Game_Map::MakeWay?
		// RPG_RT passes INT_MAX into from_x to tell it to skip self tile checks, which is hacky..
		if (!MakeWay(begin_x, begin_y, x, y)) {
			SetJumping(false);
			return false;
		}
	}

	// Adjust positions for looping maps. jump begin positions
	// get set off the edge of the map to preserve direction.
	if (Game_Map::LoopHorizontal()
			&& (x < 0 || x >= Game_Map::GetTilesX()))
	{
		const auto old_x = x;
		x = Game_Map::RoundX(x);
		begin_x += x - old_x;
	}

	if (Game_Map::LoopVertical()
			&& (y < 0 || y >= Game_Map::GetTilesY()))
	{
		auto old_y = y;
		y = Game_Map::RoundY(y);
		begin_y += y - old_y;
	}

	SetBeginJumpX(begin_x);
	SetBeginJumpY(begin_y);
	SetX(x);
	SetY(y);
	SetJumping(true);
	SetRemainingStep(SCREEN_TILE_SIZE);

	return true;
}

int Game_Character::DistanceXfromPlayer() const {
	int sx = GetX() - Main_Data::game_player->GetX();
	if (Game_Map::LoopHorizontal()) {
		if (std::abs(sx) > Game_Map::GetTilesX() / 2) {
			if (sx > 0)
				sx -= Game_Map::GetTilesX();
			else
				sx += Game_Map::GetTilesX();
		}
	}
	return sx;
}

int Game_Character::DistanceYfromPlayer() const {
	int sy = GetY() - Main_Data::game_player->GetY();
	if (Game_Map::LoopVertical()) {
		if (std::abs(sy) > Game_Map::GetTilesY() / 2) {
			if (sy > 0)
				sy -= Game_Map::GetTilesY();
			else
				sy += Game_Map::GetTilesY();
		}
	}
	return sy;
}

void Game_Character::ForceMoveRoute(const lcf::rpg::MoveRoute& new_route,
									int frequency) {
	if (!IsMoveRouteOverwritten()) {
		original_move_frequency = GetMoveFrequency();
	}

	SetPaused(false);
	SetStopCount(0xFFFF);
	SetMoveRouteIndex(0);
	SetMoveRouteFinished(false);
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
}

void Game_Character::CancelMoveRoute() {
	if (IsMoveRouteOverwritten()) {
		SetMoveFrequency(original_move_frequency);
		SetMaxStopCountForStep();
	}
	SetMoveRouteOverwritten(false);
	SetMoveRouteFinished(false);
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
	return Utils::Clamp((8 - GetTransparency()) * 32 - 1, 0, 255);
}

bool Game_Character::IsAnimated() const {
	auto at = GetAnimationType();
	return !IsAnimPaused()
		&& at != lcf::rpg::EventPage::AnimType_fixed_graphic
		&& at != lcf::rpg::EventPage::AnimType_step_frame_fix;
}

bool Game_Character::IsContinuous() const {
	auto at = GetAnimationType();
	return
		at == lcf::rpg::EventPage::AnimType_continuous ||
		at == lcf::rpg::EventPage::AnimType_fixed_continuous;
}

bool Game_Character::IsSpinning() const {
	return GetAnimationType() == lcf::rpg::EventPage::AnimType_spin;
}

int Game_Character::GetBushDepth() const {
	if ((GetLayer() != lcf::rpg::EventPage::Layers_same) || IsJumping() || IsFlying()) {
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

void Game_Character::UpdateFacing() {
	// RPG_RT only does the IsSpinning() check for Game_Event. We did it for all types here
	// in order to avoid a virtual call and because normally with RPG_RT, spinning
	// player or vehicle is impossible.
	if (IsFacingLocked() || IsSpinning()) {
		return;
	}
	const auto dir = GetDirection();
	const auto facing = GetFacing();
	if (dir >= 4) /* is diagonal */ {
		// [UR, DR, DL, UL] -> [U, D, D, U]
		const auto f1 = ((dir + (dir >= 6)) % 2) * 2;
		// [UR, DR, DL, UL] -> [R, R, L, L]
		const auto f2 = (dir / 2) - (dir < 6);
		if (facing != f1 && facing != f2) {
			// Reverse the direction.
			SetFacing((facing + 2) % 4);
		}
	} else {
		SetFacing(dir);
	}
}

/*
* Following function are made by @Ell1e
* I changed them to return a list instead of setting up MoveRoute.
*/

/* (You might want to check the CommandSmartMoveRoute() alternate
 * variant with less parameters, or CommandStepToward(), for the
 * actual description of the end-user facing commadns.)
 *
 * This is the generic version of the path finding command.
 *
 * Parameter string advanced options:
 *   Use the string to specify advanced options, like "maxRouteSteps=5" to
 *   limit the resulting movement route to a maximum of steps (if the
 *   target is further away, it simply won't be fully reached), or
 *   like "maxSearchSteps=100" where a larger number gives better results
 *   for further away targets but causes more lag, or like
 *   "ignoreEventID=93" where some event can be specified by ID to be
 *   treated as passable by the path search, so it won't try to find
 *   a path around it. The "ignoreEventID" option can be used multiple
 *   times to ignore multiple events.
 *   Example string: "maxSearchSteps=50 ignoreEventID=5 ignoreEventId=6"
 */
std::vector<Game_Character::SearchNode> Game_Character::CommandSmartMoveRoute(
	int maxRouteStepsDefault, int maxSearchStepsDefault,
	int abortIfAlreadyMovingDefault,
	int destX,
	int destY
) {
	std::unordered_set<int> ignoreEventIDs;
	bool outputDebugInfo = 0;
	int maxRouteSteps = maxRouteStepsDefault;
	int maxSearchSteps = maxSearchStepsDefault;
	int abortIfAlreadyMoving = abortIfAlreadyMovingDefault;

	

	// Extract search source:
	Game_Character* event = this;
	
	std::vector<SearchNode> listNull;
	// Set up helper variables:
	SearchNode start = SearchNode(event->GetX(), event->GetY(), 0, -1);
	if ((start.x == destX && start.y == destY) ||
		maxRouteSteps == 0)
		return listNull;
	std::vector<SearchNode> list;
	std::unordered_map<int, SearchNode> closedList;
	std::map<std::pair<int, int>, SearchNode> closedListByCoord;
	list.push_back(start);
	int id = 0;
	int idd = 0;
	int stepsTaken = 0; // Initialize steps taken to 0.
	SearchNode closestNode = SearchNode(destX, destY, INT_MAX, -1); // Initialize with a very high cost.
	int closestDistance = INT_MAX; // Initialize with a very high distance.
	std::unordered_set<SearchNode, SearchNodeHash> seen;

	if (outputDebugInfo >= 2) {
		Output::Debug("Game_Interpreter::CommandSearchPath: "
			"start search, character x{} y{}, to x{}, y{}, "
			"ignored event ids count: {}",
			start.x, start.y, destX, destY, ignoreEventIDs.size());
	}

	bool GameMapLoopsHorizontal = Game_Map::LoopHorizontal();
	bool GameMapLoopsVertical = Game_Map::LoopVertical();
	std::vector<SearchNode> neighbour;
	neighbour.reserve(8);
	while (!list.empty() && stepsTaken < maxSearchSteps) {
		SearchNode n = list[0];
		list.erase(list.begin());
		stepsTaken++;
		closedList[n.id] = n;
		closedListByCoord.insert({ {n.x, n.y}, n });

		if (n.x == destX && n.y == destY) {
			// Reached the destination.
			closestNode = n;
			closestDistance = 0;
			break;	// Exit the loop to build final route.
		}
		else {
			neighbour.clear();
			SearchNode nn = SearchNode(n.x + 1, n.y, n.cost + 1, 1); // Right
			neighbour.push_back(nn);
			nn = SearchNode(n.x, n.y - 1, n.cost + 1, 0); // Up
			neighbour.push_back(nn);
			nn = SearchNode(n.x - 1, n.y, n.cost + 1, 3); // Left
			neighbour.push_back(nn);
			nn = SearchNode(n.x, n.y + 1, n.cost + 1, 2); // Down
			neighbour.push_back(nn);

			nn = SearchNode(n.x - 1, n.y + 1, n.cost + 1, 6); // Down Left
			neighbour.push_back(nn);
			nn = SearchNode(n.x + 1, n.y - 1, n.cost + 1, 4); // Up Right
			neighbour.push_back(nn);
			nn = SearchNode(n.x - 1, n.y - 1, n.cost + 1, 7); // Up Left
			neighbour.push_back(nn);
			nn = SearchNode(n.x + 1, n.y + 1, n.cost + 1, 5); // Down Right
			neighbour.push_back(nn);

			for (SearchNode a : neighbour) {
				idd++;
				a.parentX = n.x;
				a.parentY = n.y;
				a.id = idd;
				a.parentID = n.id;

				// Adjust neighbor coordinates for map looping
				if (GameMapLoopsHorizontal) {
					if (a.x >= Game_Map::GetTilesX())
						a.x -= Game_Map::GetTilesX();
					else if (a.x < 0)
						a.x += Game_Map::GetTilesX();
				}

				if (GameMapLoopsVertical) {
					if (a.y >= Game_Map::GetTilesY())
						a.y -= Game_Map::GetTilesY();
					else if (a.y < 0)
						a.y += Game_Map::GetTilesY();
				}

				std::unordered_set<SearchNode, SearchNodeHash>::const_iterator
					check = seen.find(a);
				if (check != seen.end()) {
					SearchNode oldEntry = closedList[(*check).id];
					if (a.cost < oldEntry.cost) {
						// Found a shorter path to previous node, update & reinsert:
						if (outputDebugInfo >= 2) {
							Output::Debug("Game_Interpreter::CommandSearchPath: "
								"found shorter path to x:{} y:{}"
								"from x:{} y:{} direction: {}",
								a.x, a.y, n.x, n.y, a.direction);
						}
						closedList.erase(oldEntry.id);
						oldEntry.cost = a.cost;
						oldEntry.parentID = n.id;
						oldEntry.parentX = n.x;
						oldEntry.parentY = n.y;
						oldEntry.direction = a.direction;
						closedList[oldEntry.id] = oldEntry;
					}
					continue;
				}
				else if (a.x == start.x && a.y == start.y) {
					continue;
				}
				bool added = false;
				if (event->CheckWay(n.x, n.y, a.x, a.y, true, &ignoreEventIDs) ||
					(a.x == destX && a.y == destY &&
						event->CheckWay(n.x, n.y, a.x, a.y, false, NULL))) {
					if (a.direction == 4) {
						if (event->CheckWay(n.x, n.y, n.x + 1, n.y,
							true, &ignoreEventIDs) ||
							event->CheckWay(n.x, n.y, n.x, n.y - 1,
								true, &ignoreEventIDs)) {
							added = true;
							list.push_back(a);
							seen.insert(a);
						}
					}
					else if (a.direction == 5) {
						if (event->CheckWay(n.x, n.y, n.x + 1, n.y,
							true, &ignoreEventIDs) ||
							event->CheckWay(n.x, n.y, n.x, n.y + 1,
								true, &ignoreEventIDs)) {
							added = true;
							list.push_back(a);
							seen.insert(a);
						}
					}
					else if (a.direction == 6) {
						if (event->CheckWay(n.x, n.y, n.x - 1, n.y,
							true, &ignoreEventIDs) ||
							event->CheckWay(n.x, n.y, n.x, n.y + 1,
								true, &ignoreEventIDs)) {
							added = true;
							list.push_back(a);
							seen.insert(a);
						}
					}
					else if (a.direction == 7) {
						if (event->CheckWay(n.x, n.y, n.x - 1, n.y,
							true, &ignoreEventIDs) ||
							event->CheckWay(n.x, n.y, n.x, n.y - 1,
								true, &ignoreEventIDs)) {
							added = true;
							list.push_back(a);
							seen.insert(a);
						}
					}
					else {
						added = true;
						list.push_back(a);
						seen.insert(a);
					}
				}
				if (added && outputDebugInfo >= 2) {
					Output::Debug("Game_Interpreter::CommandSearchPath: "
						"discovered id:{} x:{} y:{} parentX:{} parentY:{}"
						"parentID:{} direction: {}",
						list[list.size() - 1].id,
						list[list.size() - 1].x, list[list.size() - 1].y,
						list[list.size() - 1].parentX,
						list[list.size() - 1].parentY,
						list[list.size() - 1].parentID,
						list[list.size() - 1].direction);
				}
			}
		}
		id++;
		// Calculate the Manhattan distance between the current node and the destination
		int manhattanDist = abs(destX - n.x) + abs(destY - n.y);

		// Check if this node is closer to the destination
		if (manhattanDist < closestDistance) {
			closestNode = n;
			closestDistance = manhattanDist;
			if (outputDebugInfo >= 2) {
				Output::Debug("Game_Interpreter::CommandSearchPath: "
					"new closest node at x:{} y:{} id:{}",
					closestNode.x, closestNode.y,
					closestNode.id);
			}
		}
	}

	// Check if a path to the closest node was found.
	if (closestDistance != INT_MAX) {
		// Build a route to the closest reachable node.
		if (outputDebugInfo >= 2) {
			Output::Debug("Game_Interpreter::CommandSearchPath: "
				"trying to return route from x:{} y:{} to "
				"x:{} y:{} (id:{})",
				start.x, start.y, closestNode.x, closestNode.y,
				closestNode.id);
		}
		std::vector<SearchNode> listMove;

		//Output::Debug("Chemin :");
		SearchNode node = closestNode;
		while (maxRouteSteps < 0 ||
			listMove.size() < maxRouteSteps) {
			listMove.push_back(node);
			bool foundParent = false;
			if (closedListByCoord.find({ node.parentX,
					node.parentY }) == closedListByCoord.end())
				break;
			SearchNode node2 = closedListByCoord[
			{node.parentX, node.parentY}
			];
			if (outputDebugInfo >= 2) {
				Output::Debug(
					"Game_Interpreter::CommandSearchPath: "
					"found parent leading to x:{} y:{}, "
					"it's at x:{} y:{} dir:{}",
					node.x, node.y,
					node2.x, node2.y, node2.direction);
			}
			node = node2;
		}

		std::reverse(listMove.rbegin(), listMove.rend());

		std::string debug_output_path("");
		if (listMove.size() > 0) {
			//lcf::rpg::MoveRoute route;
			//// route.skippable = true;
			//route.repeat = false;

			//for (SearchNode node2 : listMove) {
			//	if (node2.direction >= 0) {
			//		lcf::rpg::MoveCommand cmd;
			//		cmd.command_id = node2.direction;
			//		route.move_commands.push_back(cmd);
			//		if (outputDebugInfo >= 1) {
			//			if (debug_output_path.length() > 0)
			//				debug_output_path += ",";
			//			std::ostringstream dirnum;
			//			dirnum << node2.direction;
			//			debug_output_path += std::string(dirnum.str());
			//		}
			//	}
			//}

			//lcf::rpg::MoveCommand cmd;
			//cmd.command_id = 23;
			//route.move_commands.push_back(cmd);

			//event->ForceMoveRoute(route, 8);
		}
		if (outputDebugInfo >= 1) {
			Output::Debug(
				"Game_Interpreter::CommandSearchPath: "
				"setting route {} for character x{} y{}",
				" (ignored event ids count: {})",
				debug_output_path, start.x, start.y,
				ignoreEventIDs.size()
			);
		}
		return listMove;
	}

	// No path to the destination, return failure.
	return listNull;
}
