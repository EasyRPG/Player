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

#define CUTE_C2_IMPLEMENTATION
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
#include <limits>
#include <unordered_set>

#include "cute_c2.h"

#include <lcf/data.h>
#include <lcf/rpg/terrain.h>
#include <lcf/reader_util.h>
#include "tilemap.h"
#include "tilemap_layer.h"



Game_Character::Game_Character(Type type, lcf::rpg::SaveMapEventBase* d) :
    _type(type), _data(d)
{
}

float Game_Character::GetRealX() const {
    return real_x;
}

float Game_Character::GetRealY() const {
    return real_y;
}// Game_Character::~Game_Character() {}

void Game_Character::SanitizeData(std::string_view name) {
    SanitizeMoveRoute(name, data()->move_route, data()->move_route_index, "move_route_index");
}

void Game_Character::SanitizeMoveRoute(std::string_view name, const lcf::rpg::MoveRoute& mr, int32_t& idx, std::string_view chunk_name) {
    const auto n = static_cast<int32_t>(mr.move_commands.size());
    if (idx < 0 || idx > n) {
        idx = n;
        Output::Warning("{} {}: Save Data invalid {}={}. Fixing ...", TypeToStr(_type), name, chunk_name, idx);
    }
}

void Game_Character::MoveTo(int map_id, int x, int y) {
//  if (true) {
    if (Player::game_config.allow_pixel_movement.Get()){            // TODO - PIXELMOVE
        is_moving_toward_target = false;
        real_x = (float)x;
        real_y = (float)y;
        //Output::Warning("Char Pos = {}x{}", real_x, real_y);
    }// END - PIXELMOVE

    data()->map_id = map_id;
    // RPG_RT does not round the position for this function.
    SetX(x);
    SetY(y);
    SetRemainingStep(0);
}

// int Game_Character::GetYOffset() const {
// 	return GetJumpHeight();
// }


int Game_Character::GetJumpHeight() const {
    if (IsJumping()) {
        int jump_height = (GetRemainingStep() > SCREEN_TILE_SIZE / 2 ? SCREEN_TILE_SIZE - GetRemainingStep() : GetRemainingStep()) / 8;
        return (jump_height < 5 ? jump_height * 2 : jump_height < 13 ? jump_height + 4 : 16);
    }
    return 0;
}

int Game_Character::GetScreenX() const {
    if (Player::game_config.allow_pixel_movement.Get()) {
        float val = real_x * TILE_SIZE - floor((float)Game_Map::GetDisplayX() / (float)TILE_SIZE) + TILE_SIZE / 2.0f;

        // Wraps the screen coordinate if the map loops.
        // This keeps the sprite visible when its world coordinate is wrapped (e.g. 0.1 vs 19.9).
        // The Mode7 renderer handles the "ghosting" logic separately using a radius check.
        if (Game_Map::LoopHorizontal()) {
            val = Utils::PositiveModulo(static_cast<int>(val), Game_Map::GetTilesX() * TILE_SIZE);
        }

        return floor(val);
    }

    // Legacy tile-based logic
    int x = GetSpriteX() / TILE_SIZE - Game_Map::GetDisplayX() / TILE_SIZE + TILE_SIZE;

    if (Game_Map::LoopHorizontal()) {
        x = Utils::PositiveModulo(x, Game_Map::GetTilesX() * TILE_SIZE);
    }
    x -= TILE_SIZE / 2;

    return x;
}

int Game_Character::GetScreenY(bool apply_jump) const {
    if (Player::game_config.allow_pixel_movement.Get()) {
        float val = real_y * TILE_SIZE - floor((float)Game_Map::GetDisplayY() / (float)TILE_SIZE) + TILE_SIZE;

        if (apply_jump) {
            val -= GetJumpHeight();
        }

        // Wraps the screen coordinate if the map loops.
        if (Game_Map::LoopVertical()) {
            val = Utils::PositiveModulo(static_cast<int>(val), Game_Map::GetTilesY() * TILE_SIZE);
        }

        return floor(val);
    }

    // Legacy tile-based logic
    int y = GetSpriteY() / TILE_SIZE - Game_Map::GetDisplayY() / TILE_SIZE + TILE_SIZE;

    if (apply_jump) {
        y -= GetJumpHeight();
    }

    if (Game_Map::LoopVertical()) {
        y = Utils::PositiveModulo(y, Game_Map::GetTilesY() * TILE_SIZE);
    }
    return y;
}

Drawable::Z_t Game_Character::GetScreenZ(int x_offset, int y_offset) const {
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
    Drawable::Z_t y = static_cast<Drawable::Z_t>(GetScreenY(false) + y_offset + 0x8000);
    Drawable::Z_t x = static_cast<Drawable::Z_t>(GetScreenX() + x_offset + 0x8000);

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


//	if (true) {
    if (Player::game_config.allow_pixel_movement.Get()){        // TODO - PIXELMOVE
        UpdateMoveTowardTarget();
    } // END - PIXELMOVE


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

    // --- START: Pixel Movement Jump Interpolation Fix ---
    // If a jump is in progress and pixel movement is active, we need to manually
    // interpolate the real_x and real_y coordinates from the start to the end point.
    if (IsJumping() && Player::game_config.allow_pixel_movement.Get()) {
        // total_duration is the number of "steps" a jump takes, which is always SCREEN_TILE_SIZE
        const int total_duration = SCREEN_TILE_SIZE;
        // elapsed is how many steps have passed since the jump started
        int elapsed = total_duration - GetRemainingStep();
        // progress is a float from 0.0 to 1.0 representing jump completion
        float progress = static_cast<float>(elapsed) / static_cast<float>(total_duration);

        // Linearly interpolate the real coordinates based on the jump's progress
        real_x = jump_start_real_x + (jump_end_real_x - jump_start_real_x) * progress;
        real_y = jump_start_real_y + (jump_end_real_y - jump_start_real_y) * progress;
    }
    // --- END: Pixel Movement Jump Interpolation Fix ---

    SetRemainingStep(GetRemainingStep() - amount);
    if (GetRemainingStep() <= 0) {
        SetRemainingStep(0);
        bool was_jumping = IsJumping();
        SetJumping(false);

    if (was_jumping && Player::game_config.allow_pixel_movement.Get()) {
            real_x = jump_end_real_x;
            real_y = jump_end_real_y;
            SetX(static_cast<int>(round(real_x)));
            SetY(static_cast<int>(round(real_y)));
    }


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

    if (true && is_moving_toward_target && !current_route.skippable) { // TODO - PIXELMOVE
        return;
    } // END - PIXELMOVE



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
                    TurnTowardCharacter(GetPlayer());
                    break;
                case Code::move_away_from_hero:
                    TurnAwayFromCharacter(GetPlayer());
                    break;
                case Code::move_forward:
                    break;
                default:
                    break;
            }
            /*
            Move(GetDirection());
            */


//			if (true && (cmd >= Code::move_towards_hero && cmd <= Code::move_away_from_hero)) { // TODO - PIXELMOVE
            if (Player::game_config.allow_pixel_movement.Get() && (cmd >= Code::move_towards_hero && cmd <= Code::move_away_from_hero)) {
                int flag = (1 - (cmd == Code::move_away_from_hero) * 2);
                float vx = (Main_Data::game_player->real_x - real_x) * flag;
                float vy = (Main_Data::game_player->real_y - real_y) * flag;
                float length = sqrt(vx * vx + vy * vy);
                float step_size = GetStepSize();
                MoveVector(step_size * (vx / length), step_size * (vy / length));
            }
//			else if (true) {
            else if (Player::game_config.allow_pixel_movement.Get()){        // TODO - PIXELMOVE
                float vx = (float)GetDxFromDirection(GetDirection());
                float vy = (float)GetDyFromDirection(GetDirection());
                c2v target;
                if (forced_skip) {
                    forced_skip = false;
                    target = c2V(round(target_x + vx), round(target_y + vy));
                }
                else {
                    target = c2V(round(real_x + vx), round(real_y + vy));
                }
                SetMoveTowardTarget(target, current_route.skippable);
                UpdateMoveTowardTarget();
                if (!current_route.skippable) {
                    SetMaxStopCountForStep();
                    ++current_index;
                    return;
                }
            }
            else {
                Move(GetDirection());
            } // END - PIXELMOV


            static const int move_speed[] = { 16, 8, 6, 4, 3, 2 };
            doomWait = move_speed[GetMoveSpeed() - 1];

            if (IsStopping()) {
                // Move failed
                if (current_route.skippable) {
                    SetDirection(prev_direction);
                    SetFacing(prev_facing);
                } else {
                    SetMoveFailureCount(GetMoveFailureCount() + 1);
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
                    TurnTowardCharacter(GetPlayer());
                    break;
                case Code::face_away_from_hero:
                    TurnAwayFromCharacter(GetPlayer());
                    break;
                default:
                    break;
            }
            SetFacing(GetDirection());
            SetMaxStopCountForTurn();
            SetStopCount(0);

            static const int turn_speed[] = { 64, 32, 24, 16, 12, 8 };
            doomWait = turn_speed[GetMoveSpeed() - 1];


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
                            SetMoveFailureCount(GetMoveFailureCount() + 1);
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
        SetMoveFailureCount(0);
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
        Span<int> ignore_some_events_by_id) {
    return Game_Map::CheckWay(*this, from_x, from_y, to_x, to_y,
        ignore_all_events, ignore_some_events_by_id);
}

void Game_Character::SetMoveTowardTarget(c2v position, bool skippable) {
    SetMoveTowardTarget(position.x, position.y, skippable);
}

void Game_Character::SetMoveTowardTarget(float x, float y, bool skippable) {
    is_moving_toward_target = true;
    is_move_toward_target_skippable = skippable;
    target_x = x;
    target_y = y;
    move_direction = c2Norm(c2V(target_x - real_x, target_y - real_y));
}

bool Game_Character::UpdateMoveTowardTarget() {
    if (!is_moving_toward_target || IsPaused()) {
        return false;
    }
    //forced_skip       = false;
    bool move_success = false;
    c2v vector = c2V(target_x - real_x, target_y - real_y);
    float length = c2Len(vector);
    c2v vectorNorm = c2Div(vector, length);
    float step_size = GetStepSize();
    if (length > step_size) {
        move_success = MoveVector(c2Mulvs(vectorNorm, step_size));
    }
    else {
        move_success = MoveVector(vector);
        is_moving_toward_target = false;
    }
    if (!move_success) {
        if (is_move_toward_target_skippable) {
            is_moving_toward_target = false;
        }
        else if (c2Dot(vectorNorm, move_direction) <= 0) {
            is_moving_toward_target = false;
            forced_skip = true;
        }
    }
    return move_success;
}

bool Game_Character::MoveVector(c2v vector) {
    return MoveVector(vector.x, vector.y);
}

bool Game_Character::MoveVector(float vx, float vy) {  // TODO - PIXELMOVE
//	if (abs(vx) <= Epsilon && abs(vy) <= Epsilon) {
//		return false;
//	}

    auto& player = Main_Data::game_player;
    auto player_x = player->GetX();
    auto player_y = player->GetY();


    bool vehicle = Main_Data::game_player->InVehicle();
    bool airship = Main_Data::game_player->InAirship();
    bool flying = Main_Data::game_player->IsFlying();
    bool boarding = Main_Data::game_player->IsBoardingOrUnboarding();
    bool isAboard = Main_Data::game_player->IsAboard();
    bool ascending = Game_Map::GetVehicle(Game_Vehicle::Airship)->IsAscending();
    bool descending = Game_Map::GetVehicle(Game_Vehicle::Airship)->IsDescending();
    bool airshipUse = Game_Map::GetVehicle(Game_Vehicle::Airship)->IsInUse();

    auto boatFront = Game_Map::GetVehicle(Game_Vehicle::Boat)->GetDirection();
    auto playerFront = Main_Data::game_player->GetDirection();
    auto airshipFront    = Game_Map::GetVehicle(Game_Vehicle::Airship)->GetDirection();

    auto MapID = Main_Data::game_player->GetMapId();

    if (boarding || ascending || IsJumping() || descending)           // this is to try and stop events from going to NaNland.
    {
        return false;
    }

    if (!GetThrough() && !IsFlying() && !Game_Map::IsPassableTile(this, 0x0F, Game_Map::RoundX(GetX()), Game_Map::RoundY(GetY()))) {
        return false;
    }

    if (!IsFacingLocked()) {
        if (std::abs(vx) > std::abs(vy)) {
            SetDirection(vx > 0 ? Right : Left);
        } else if (std::abs(vy) > 0) {
            SetDirection(vy > 0 ? Down : Up);
        }
    }

    UpdateFacing();
    SetRemainingStep(1); //little hack to make the character step anim
    float last_x = real_x;
    float last_y = real_y;
    real_x += vx;
    real_y += vy;
    if (GetThrough()) {
        return true;
    }
    c2Circle self;
    c2Circle other;
    c2Circle hero;
    self.p = c2V(real_x + 0.5, real_y + 0.5);
    self.r = 0.5;
       other.r = 0.5;
    c2AABB tile;

    c2Manifold manifold;

    /*
    c2Poly poly;
    poly.count = 4;
    poly.verts[0] = c2V(0, 0);
    poly.verts[1] = c2V(1, 0);
    poly.verts[2] = c2V(1, 1);
    poly.verts[3] = c2V(0, 1);
    c2MakePoly(&poly);
    c2x transform = c2xIdentity();
    //
    c2Poly poly;
    poly.count = 3;
    poly.verts[0] = c2V(0, 1);
    poly.verts[1] = c2V(1, 0);
    poly.verts[2] = c2V(1, 1);
    c2MakePoly(&poly);
    c2x transform = c2xIdentity();
    transform.p = c2V(14, 16);
    c2CircletoPolyManifold(self, &poly, &transform, &manifold);
    if (manifold.count > 0) {
        self.p.x -= manifold.n.x * manifold.depths[0];
        self.p.y -= manifold.n.y * manifold.depths[0];
    }
    transform.p = c2V(15, 15);
    c2CircletoPolyManifold(self, &poly, &transform, &manifold);
    if (manifold.count > 0) {
        self.p.x -= manifold.n.x * manifold.depths[0];
        self.p.y -= manifold.n.y * manifold.depths[0];
    }
    transform.p = c2V(16, 14);
    c2CircletoPolyManifold(self, &poly, &transform, &manifold);
    if (manifold.count > 0) {
        self.p.x -= manifold.n.x * manifold.depths[0];
        self.p.y -= manifold.n.y * manifold.depths[0];
    }
    */

    //Test Collision With Events
    for (auto& ev : Game_Map::GetEvents()) {
        if (!Game_Map::WouldCollideWithCharacter(*this, ev, false)) {
            continue;
        }
        other.p.x = ev.real_x + 0.5;
        other.p.y = ev.real_y + 0.5;
        c2CircletoCircleManifold(self, other, &manifold);
        if (manifold.count > 0) {
            self.p.x -= manifold.n.x * manifold.depths[0];
            self.p.y -= manifold.n.y * manifold.depths[0];
        }
    }
    //Test Collision With Player

    if (Game_Map::WouldCollideWithCharacter(*this, *Main_Data::game_player, false) && !Main_Data::game_player->IsFlying()) {
        other.p.x = player->real_x + 0.5;
        other.p.y = player->real_y + 0.5;
        c2CircletoCircleManifold(self, other, &manifold);
        if (manifold.count > 0) {
            self.p.x -= manifold.n.x * manifold.depths[0];
            self.p.y -= manifold.n.y * manifold.depths[0];
// Now, check if this collision should trigger an event.
            // This only applies if 'this' character is an event bumping into the player.
            if (GetType() == Game_Character::Event) {
                Game_Event* self_as_event = static_cast<Game_Event*>(this);

                // Check for "Event Touch" on the "Same as Hero" layer
                if (self_as_event->GetTrigger() == lcf::rpg::EventPage::Trigger_collision &&
                    self_as_event->GetLayer() == lcf::rpg::EventPage::Layers_same &&
                    !Game_Map::GetInterpreter().IsRunning())
                {
                    // Collision is confirmed, trigger the event!
                    self_as_event->ScheduleForegroundExecution(false, true);
                }
            }
        }
    }
//Test Collision With Map - Map collision has high priority, so it is tested last

    // MODIFIED: Airships that are flying should ignore map collision entirely.
    if (IsFlying()) {
        real_x = self.p.x - 0.5f;
        real_y = self.p.y - 0.5f;

    if (Game_Map::LoopHorizontal()) {
        const float map_width_f = static_cast<float>(Game_Map::GetTilesX());
        // Use fmod to wrap the coordinate into the [-map_width, map_width] range
        real_x = fmod(real_x, map_width_f);
        // If the result is negative, add map_width to bring it into the [0, map_width] range
        if (real_x < 0.0f) {
            real_x += map_width_f;
        }
    }

    else if (this == Main_Data::game_player.get()) {
            // If not looping, clamp to map bounds (0 to Width - 1)
            float map_width_f = static_cast<float>(Game_Map::GetTilesX());
            if (real_x < 0.0f) real_x = 0.0f;
            if (real_x > map_width_f - 1.0f) real_x = map_width_f - 1.0f;
        }

    if (Game_Map::LoopVertical()) {
        const float map_height_f = static_cast<float>(Game_Map::GetTilesY());
        real_y = fmod(real_y, map_height_f);
        if (real_y < 0.0f) {
            real_y += map_height_f;
        }
    }
    else if (this == Main_Data::game_player.get()) {
            // If not looping, clamp to map bounds (0 to Height - 1)
            float map_height_f = static_cast<float>(Game_Map::GetTilesY());
            if (real_y < 0.0f) real_y = 0.0f;
            if (real_y > map_height_f - 1.0f) real_y = map_height_f - 1.0f;
        }

        SetX(round(real_x));
        SetY(round(real_y));

        // Check for landing possibility with decision key
        if (Input::IsTriggered(Input::DECISION) && GetType() == Game_Character::Player) {
            Game_Map::GetVehicle(Game_Vehicle::Airship)->StartDescent();
        }

        return true; // Skip all further map collision checks
    }

    int map_width = Game_Map::GetTilesX();
    int map_height = Game_Map::GetTilesY();

    // Clamp the player's position to the map boundaries on non-looping maps.
    // This check applies to the player directly and when in a vehicle.
    if (Player::game_config.allow_pixel_movement.Get() && this == Main_Data::game_player.get()) {
        if (!Game_Map::LoopHorizontal()) {
            float map_width_f = static_cast<float>(Game_Map::GetTilesX());
            // Clamp the center of the character so its edges (radius 0.5) don't go past the map boundary.
            self.p.x = std::max(0.5f, std::min(self.p.x, map_width_f - 0.5f));
        }

        if (!Game_Map::LoopVertical()) {
            float map_height_f = static_cast<float>(Game_Map::GetTilesY());
            self.p.y = std::max(0.5f, std::min(self.p.y, map_height_f - 0.5f));
        }
    }

    int left = floor(self.p.x - 0.5f);
    int right = floor((self.p.x - 0.5f) + 1.0f);
    int top = floor(self.p.y - 0.5f);
    int bottom = floor((self.p.y - 0.5f) + 1.0f);

    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            int tile_x = x;
            int tile_y = y;

            if (Game_Map::LoopHorizontal()) {
                tile_x = (tile_x % map_width + map_width) % map_width;
            }
            if (Game_Map::LoopVertical()) {
                tile_y = (tile_y % map_height + map_height) % map_height;
            }

            // MODIFIED: Use IsPassableTile, which already contains all the logic for
            // different vehicles. We check passability from all cardinal directions (0x0F)
            // as a proxy for "can this character be on this tile at all?".
            if (!Game_Map::IsPassableTile(&(*this), 0x0F, tile_x, tile_y)) {
                c2AABB tile_aabb;
                tile_aabb.min = c2V(x, y);
                tile_aabb.max = c2V(x + 1, y + 1);
                c2CircletoAABBManifold(self, tile_aabb, &manifold);
                if (manifold.count > 0) {
                    // Simplified collision resolution
                    self.p.x -= manifold.n.x * manifold.depths[0];
                    self.p.y -= manifold.n.y * manifold.depths[0];
                }
            }
        }
    }


    real_x = self.p.x - 0.5f;
    real_y = self.p.y - 0.5f;

    if (Game_Map::LoopHorizontal()) {
        const float map_width_f = static_cast<float>(Game_Map::GetTilesX());
        // Use fmod to wrap the coordinate into the [-map_width, map_width] range
        real_x = fmod(real_x, map_width_f);
        // If the result is negative, add map_width to bring it into the [0, map_width] range
        if (real_x < 0.0f) {
            real_x += map_width_f;
        }
    }

    if (Game_Map::LoopVertical()) {
        const float map_height_f = static_cast<float>(Game_Map::GetTilesY());
        real_y = fmod(real_y, map_height_f);
        if (real_y < 0.0f) {
            real_y += map_height_f;
        }
    }

    SetX(round(real_x));
    SetY(round(real_y));

    if (abs(real_x - last_x) <= Epsilon && abs(real_y - last_y) <= Epsilon) {
        SetRemainingStep(0);
        return false; // If there is no expressive change, treat as no movement.
    }

    return true;
}

bool Game_Character::Move(int dir) {
//    	if (true) {
        if (Player::game_config.allow_pixel_movement.Get()){// TODO - PIXELMOVE
        SetDirection(dir);
        c2v vector = c2V(GetDxFromDirection(dir), GetDyFromDirection(dir));
        float step_size = GetStepSize();
        return MoveVector(c2Mulvs(c2Norm(vector), step_size));
    }

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

int Game_Character::GetDirectionToCharacter(const Game_Character& target) {
    int sx = GetDistanceXfromCharacter(target);
    int sy = GetDistanceYfromCharacter(target);

    if ( std::abs(sx) > std::abs(sy) ) {
        return (sx > 0) ? Left : Right;
    } else {
        return (sy > 0) ? Up : Down;
    }
}

int Game_Character::GetDirectionAwayCharacter(const Game_Character& target) {
    int sx = GetDistanceXfromCharacter(target);
    int sy = GetDistanceYfromCharacter(target);

    if ( std::abs(sx) > std::abs(sy) ) {
        return (sx > 0) ? Right : Left;
    } else {
        return (sy > 0) ? Down : Up;
    }
}

void Game_Character::TurnTowardCharacter(const Game_Character& target) {
    SetDirection(GetDirectionToCharacter(target));
}

void Game_Character::TurnAwayFromCharacter(const Game_Character& target) {
    SetDirection(GetDirectionAwayCharacter(target));
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
                    TurnTowardCharacter(GetPlayer());
                    break;
                case Code::move_away_from_hero:
                    TurnAwayFromCharacter(GetPlayer());
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
                    TurnTowardCharacter(GetPlayer());
                    break;
                case Code::face_away_from_hero:
                    TurnAwayFromCharacter(GetPlayer());
                    break;
                default:
                    break;
            }
        }

        if (cmd == Code::end_jump) {
        bool rc;
            if (Player::game_config.allow_pixel_movement.Get()) {
                float new_x = GetRealX() + jdx;
                float new_y = GetRealY() + jdy;
                rc = Jump(new_x, new_y);
            } else {
                int new_x = GetX() + jdx;
                int new_y = GetY() + jdy;
                rc = Jump(new_x, new_y);
            }
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

// --- START: New Pixel-Perfect Jump Function ---
bool Game_Character::Jump(float x, float y) {
    if (!IsStopping()) {
        return true;
    }

    // Store the precise floating-point start and end coordinates
    jump_start_real_x = GetRealX();
    jump_start_real_y = GetRealY();
    jump_end_real_x = x;
    jump_end_real_y = y;

    // For compatibility, also store the tile-based start/end points
    auto begin_x = GetX();
    auto begin_y = GetY();
    const auto final_tile_x = static_cast<int>(round(x));
    const auto final_tile_y = static_cast<int>(round(y));
    const auto dx = final_tile_x - begin_x;
    const auto dy = final_tile_y - begin_y;

    // Determine facing direction based on jump vector
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

        // Pathfinding still uses the tile grid. A pixel jump is only
        // allowed if the underlying tile path is clear.
        if (!MakeWay(begin_x, begin_y, final_tile_x, final_tile_y)) {
            SetJumping(false);
            return false; // Jump failed, path is blocked
        }
    }

    // Update the integer tile coordinates to the final destination tile
    SetBeginJumpX(begin_x);
    SetBeginJumpY(begin_y);
    SetX(final_tile_x);
    SetY(final_tile_y);

    SetRemainingStep(SCREEN_TILE_SIZE);

    return true;
}
// --- END: New Pixel-Perfect Jump Function ---

bool Game_Character::Jump(int x, int y) {


     if (Player::game_config.allow_pixel_movement.Get()) {
        // If pixel movement is on, call the new float-based version
        return Jump(static_cast<float>(x), static_cast<float>(y));
    }

//   		real_x = (float)x;
//		real_y = (float)y;

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

    if (Player::game_config.allow_pixel_movement.Get()) {
        real_x = static_cast<float>(begin_x);
        real_y = static_cast<float>(begin_y);
    }

    SetX(x);
    SetY(y);

//  SetX(real_x);
//  SetY(real_y);
    SetJumping(true);
    SetRemainingStep(SCREEN_TILE_SIZE);

 /*     if (true) { // TODO - PIXELMOVE



//      SetDirection(GetDirection());
        c2v vector = c2V(GetDxFromDirection(GetDirection()), GetDyFromDirection(GetDirection()));
//      c2v vector = c2V(real_x - begin_x, real_y - begin_y);
        float length = c2Len(vector);
        c2v vectorNorm = c2Div(vector, length);
        float step_size = GetStepSize();
//      MoveVector(c2Mulvs(vectorNorm, step_size));
          MoveVector(c2Mulvs(c2Norm(vectorNorm), step_size));
//      SetRemainingStep(0);
}
*/

/* Reference material
    c2v vector = c2V(GetDxFromDirection(GetDirection()), GetDyFromDirection(GetDirection()));
    c2v vector = c2V(target_x - real_x, target_y - real_y);
    float length = c2Len(vector);
    c2v vectorNorm = c2Div(vector, length);
    float step_size = GetStepSize();
    if (length > step_size) {
        move_success = MoveVector(c2Mulvs(vectorNorm, step_size));
    }
    else {
        move_success = MoveVector(vector);
        is_moving_toward_target = false;
    }
    if (!move_success) {
        if (is_move_toward_target_skippable) {
            is_moving_toward_target = false;
        }
        else if (c2Dot(vectorNorm, move_direction) <= 0) {
            is_moving_toward_target = false;
            forced_skip = true;
        }
    }

*/


    return true;
}

int Game_Character::GetDistanceXfromCharacter(const Game_Character& target) const {

// 	if (true) {
    if (Player::game_config.allow_pixel_movement.Get()){        // TODO - PIXELMOVE

        float sx = real_x - Main_Data::game_player->real_x;

        if (Game_Map::LoopHorizontal()) {
            if (std::abs(sx) > Game_Map::GetTilesX() / 2) {
                if (sx > 0)
                    sx -= Game_Map::GetTilesX();
                else
                    sx += Game_Map::GetTilesX();
            }
        }
        return round(sx * SCREEN_TILE_SIZE);
    } //END - PIXELMOVE


    int sx = GetX() - target.GetX();
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

int Game_Character::GetDistanceYfromCharacter(const Game_Character& target) const {

// 	if (true) {
    if (Player::game_config.allow_pixel_movement.Get()){        // TODO - PIXELMOVE
        float sy = real_y - Main_Data::game_player->real_y;

        if (Game_Map::LoopVertical()) {
            if (std::abs(sy) > Game_Map::GetTilesY() / 2) {
                if (sy > 0)
                    sy -= Game_Map::GetTilesY();
                else
                    sy += Game_Map::GetTilesY();
            }
        }
        return round(sy * SCREEN_TILE_SIZE);
    } // END - PIXELMOVE



    int sy = GetY() - target.GetY();
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
    SetMoveFailureCount(0);
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

struct SearchNode {
    int x = 0;
    int y = 0;
    int cost = 0;
    int direction = 0;

    int id = 0;
    int parent_id = -1;
    int parent_x = -1;
    int parent_y = -1;

    friend bool operator==(const SearchNode& n1, const SearchNode& n2)
    {
        return n1.x == n2.x && n1.y == n2.y;
    }

    bool operator()(SearchNode const& a, SearchNode const& b)
    {
        return a.id > b.id;
    }
};

struct SearchNodeHash {
    size_t operator()(const SearchNode &p) const {
        return (p.x ^ (p.y + (p.y >> 12)));
    }
};

bool Game_Character::CalculateMoveRoute(const CalculateMoveRouteArgs& args) {
    CancelMoveRoute();

    // Set up helper variables:
    SearchNode start = {GetX(), GetY(), 0, -1};
    if ((start.x == args.dest_x && start.y == args.dest_y) || args.steps_max == 0) {
        return true;
    }
    std::vector<SearchNode> queue;
    std::unordered_map<int, SearchNode> graph;
    std::map<std::pair<int, int>, SearchNode> graph_by_coord;
    queue.push_back(start);
    int id = 0;
    int idd = 0;
    int steps_taken = 0;
    SearchNode closest_node = {args.dest_x, args.dest_y, std::numeric_limits<int>::max(), -1}; // Initialize with a very high cost.
    int closest_distance = std::numeric_limits<int>::max(); // Initialize with a very high distance.
    std::unordered_set<SearchNode, SearchNodeHash> seen;

    int steps_max = args.steps_max;
    if (steps_max == -1) {
        steps_max = std::numeric_limits<int>::max();
    }

    if (args.debug_print) {
        Output::Debug("Game_Interpreter::CommandSearchPath: "
            "start search, character x{} y{}, to x{}, y{}, "
            "ignored event ids count: {}",
            start.x, start.y, args.dest_x, args.dest_y, args.event_id_ignore_list.size());
    }

    bool loops_horizontal = Game_Map::LoopHorizontal();
    bool loops_vertical = Game_Map::LoopVertical();
    std::vector<SearchNode> neighbour;
    neighbour.reserve(8);
    while (!queue.empty() && steps_taken < args.search_max) {
        SearchNode n = queue[0];
        queue.erase(queue.begin());
        steps_taken++;
        graph[n.id] = n;
        graph_by_coord.insert({{n.x, n.y}, n});

        if (n.x == args.dest_x && n.y == args.dest_y) {
            // Reached the destination.
            closest_node = n;
            closest_distance = 0;
            break;	// Exit the loop to build final route.
        }
        else {
            neighbour.clear();
            SearchNode nn = {n.x + 1, n.y, n.cost + 1, 1}; // Right
            neighbour.push_back(nn);
            nn = {n.x, n.y - 1, n.cost + 1, 0}; // Up
            neighbour.push_back(nn);
            nn = {n.x - 1, n.y, n.cost + 1, 3}; // Left
            neighbour.push_back(nn);
            nn = {n.x, n.y + 1, n.cost + 1, 2}; // Down
            neighbour.push_back(nn);

            if (args.allow_diagonal) {
                nn = {n.x - 1, n.y + 1, n.cost + 1, 6}; // Down Left
                neighbour.push_back(nn);
                nn = {n.x + 1, n.y - 1, n.cost + 1, 4}; // Up Right
                neighbour.push_back(nn);
                nn = {n.x - 1, n.y - 1, n.cost + 1, 7}; // Up Left
                neighbour.push_back(nn);
                nn = {n.x + 1, n.y + 1, n.cost + 1, 5}; // Down Right
                neighbour.push_back(nn);
            }

            for (SearchNode a : neighbour) {
                idd++;
                a.parent_x = n.x;
                a.parent_y = n.y;
                a.id = idd;
                a.parent_id = n.id;

                // Adjust neighbor coordinates for map looping
                if (loops_horizontal) {
                    if (a.x >= Game_Map::GetTilesX())
                        a.x -= Game_Map::GetTilesX();
                    else if (a.x < 0)
                        a.x += Game_Map::GetTilesX();
                }

                if (loops_vertical) {
                    if (a.y >= Game_Map::GetTilesY())
                        a.y -= Game_Map::GetTilesY();
                    else if (a.y < 0)
                        a.y += Game_Map::GetTilesY();
                }

                auto check = seen.find(a);
                if (check != seen.end()) {
                    SearchNode old_entry = graph[(*check).id];
                    if (a.cost < old_entry.cost) {
                        // Found a shorter path to previous node, update & reinsert:
                        if (args.debug_print) {
                            Output::Debug("Game_Interpreter::CommandSearchPath: "
                                "found shorter path to x:{} y:{}"
                                "from x:{} y:{} direction: {}",
                                a.x, a.y, n.x, n.y, a.direction);
                        }
                        graph.erase(old_entry.id);
                        old_entry.cost = a.cost;
                        old_entry.parent_id = n.id;
                        old_entry.parent_x = n.x;
                        old_entry.parent_y = n.y;
                        old_entry.direction = a.direction;
                        graph[old_entry.id] = old_entry;
                    }
                    continue;
                } else if (a.x == start.x && a.y == start.y) {
                    continue;
                }
                bool added = false;
                if (CheckWay(n.x, n.y, a.x, a.y, true, args.event_id_ignore_list) ||
                        (a.x == args.dest_x && a.y == args.dest_y &&
                        CheckWay(n.x, n.y, a.x, a.y, false, {}))) {
                    if (a.direction == 4) {
                        if (CheckWay(n.x, n.y, n.x + 1, n.y,
                                    true, args.event_id_ignore_list) ||
                                CheckWay(n.x, n.y, n.x, n.y - 1,
                                    true, args.event_id_ignore_list)) {
                            added = true;
                            queue.push_back(a);
                            seen.insert(a);
                        }
                    }
                    else if (a.direction == 5) {
                        if (CheckWay(n.x, n.y, n.x + 1, n.y,
                                    true, args.event_id_ignore_list) ||
                                CheckWay(n.x, n.y, n.x, n.y + 1,
                                    true, args.event_id_ignore_list)) {
                            added = true;
                            queue.push_back(a);
                            seen.insert(a);
                        }
                    }
                    else if (a.direction == 6) {
                        if (CheckWay(n.x, n.y, n.x - 1, n.y,
                                    true, args.event_id_ignore_list) ||
                                CheckWay(n.x, n.y, n.x, n.y + 1,
                                    true, args.event_id_ignore_list)) {
                            added = true;
                            queue.push_back(a);
                            seen.insert(a);
                        }
                    }
                    else if (a.direction == 7) {
                        if (CheckWay(n.x, n.y, n.x - 1, n.y,
                                    true, args.event_id_ignore_list) ||
                                CheckWay(n.x, n.y, n.x, n.y - 1,
                                    true, args.event_id_ignore_list)) {
                            added = true;
                            queue.push_back(a);
                            seen.insert(a);
                        }
                    }
                    else {
                        added = true;
                        queue.push_back(a);
                        seen.insert(a);
                    }
                }
                if (added && args.debug_print) {
                    Output::Debug("Game_Interpreter::CommandSearchPath: "
                        "discovered id:{} x:{} y:{} parentX:{} parentY:{}"
                        "parentID:{} direction: {}",
                    queue[queue.size() - 1].id,
                    queue[queue.size() - 1].x, queue[queue.size() - 1].y,
                    queue[queue.size() - 1].parent_x,
                    queue[queue.size() - 1].parent_y,
                    queue[queue.size() - 1].parent_id,
                    queue[queue.size() - 1].direction);
                }
            }
        }
        id++;
        // Calculate the Manhattan distance between the current node and the destination
        int manhattan_dist = abs(args.dest_x - n.x) + abs(args.dest_y - n.y);

        // Check if this node is closer to the destination
        if (manhattan_dist < closest_distance) {
            closest_node = n;
            closest_distance = manhattan_dist;
            if (args.debug_print) {
                Output::Debug("Game_Interpreter::CommandSearchPath: "
                        "new closest node at x:{} y:{} id:{}",
                    closest_node.x, closest_node.y,
                    closest_node.id);
            }
        }
    }

    // Check if a path to the closest node was found.
    if (closest_distance != std::numeric_limits<int>::max()) {
        // Build a route to the closest reachable node.
        if (args.debug_print) {
            Output::Debug("Game_Interpreter::CommandSearchPath: "
                    "trying to return route from x:{} y:{} to "
                    "x:{} y:{} (id:{})",
                start.x, start.y, closest_node.x, closest_node.y,
                closest_node.id);
        }
        std::vector<SearchNode> list_move;

        SearchNode node = closest_node;
        while (static_cast<int>(list_move.size()) < steps_max) {
            list_move.push_back(node);
            if (graph_by_coord.find({node.parent_x,
                    node.parent_y}) == graph_by_coord.end())
                break;
            SearchNode node2 = graph_by_coord[
                {node.parent_x, node.parent_y}
            ];
            if (args.debug_print) {
                Output::Debug(
                    "Game_Interpreter::CommandSearchPath: "
                    "found parent leading to x:{} y:{}, "
                    "it's at x:{} y:{} dir:{}",
                    node.x, node.y,
                    node2.x, node2.y, node2.direction);
            }
            node = node2;
        }

        std::reverse(list_move.rbegin(), list_move.rend());

        std::string debug_output_path("");
        if (list_move.size() > 0) {
            lcf::rpg::MoveRoute route;
            route.skippable = args.skip_when_failed;
            route.repeat = false;

            for (SearchNode node2 : list_move) {
                if (node2.direction >= 0) {
                    lcf::rpg::MoveCommand cmd;
                    cmd.command_id = node2.direction;
                    route.move_commands.push_back(cmd);
                    if (args.debug_print >= 1) {
                        if (debug_output_path.length() > 0)
                            debug_output_path += ",";
                        std::ostringstream dirnum;
                        dirnum << node2.direction;
                        debug_output_path += std::string(dirnum.str());
                    }
                }
            }

            lcf::rpg::MoveCommand cmd;
            cmd.command_id = 23;
            route.move_commands.push_back(cmd);

            ForceMoveRoute(route, args.frequency);
        }
        if (args.debug_print) {
            Output::Debug(
                "Game_Interpreter::CommandSearchPath: "
                "setting route {} for character x{} y{}",
                " (ignored event ids count: {})",
                debug_output_path, start.x, start.y,
                args.event_id_ignore_list.size()
            );
        }
        return true;
    }

    // No path to the destination, return failure.
    return false;
}

int Game_Character::GetSpriteX() const {

//	if (true) {
    if (Player::game_config.allow_pixel_movement.Get()){        // TODO - PIXEL MOVE
        return round(real_x * SCREEN_TILE_SIZE);
    } // END - PIXELMOVE


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

//	if (true) {
    if (Player::game_config.allow_pixel_movement.Get()){        // TODO - PIXEL MOVE
        return round(real_x * SCREEN_TILE_SIZE);
    } // END - PIXELMOVE


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

Game_Character& Game_Character::GetPlayer() {
    assert(Main_Data::game_player);

    return *Main_Data::game_player;
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
