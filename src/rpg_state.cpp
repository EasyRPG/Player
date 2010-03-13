//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "rpg_state.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
RPG::State::State() {
    ID = 0;
    name = "";
    type = 0;
    color = 6;
    priority = 50;
    restriction = 0;
    a_rate = 100;
    b_rate = 80;
    c_rate = 60;
    d_rate = 30;
    e_rate = 0;
    hold_turn = 0;
    auto_release_prob = 0;
    release_by_damage = 0,
    affect_type = 0;
    affect_attack = false;
    affect_defense = false;
    affect_spirit = false;
    affect_agility = false;
    reduce_hit_ratio = 100;
    avoid_attacks = false;
    reflect_magic = false;
    cursed = false;
    battler_animation_id = 100;
    restrict_skill = false;
    restrict_skill_level = 0;
    restrict_magic = false;
    restrict_magic_level = 0;
    hp_change_type = 0;
    sp_change_type = 0;
    message_actor = "";
    message_enemy = "";
    message_already = "";
    message_affected = "";
    message_recovery = "";
    hp_change_max = 0;
    hp_change_val = 0;
    hp_change_map_val = 0;
    hp_change_map_steps = 0;
    sp_change_max = 0;
    sp_change_val = 0;
    sp_change_map_val = 0;
    sp_change_map_steps = 0;
}
