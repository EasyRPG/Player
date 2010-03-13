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
#include "rpg_trooppagecondition.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
RPG::TroopPageCondition::TroopPageCondition() {
    switch_a = false;
    switch_b = false;
    variable = false;
    turn = false;
    fatigue = false;
    enemy_hp = false;
    actor_hp = false;
    switch_a_id = 1;
    switch_b_id = 1;
    variable_id = 1;
    variable_value = 0;
    turn_a = 0;
    turn_b = 0;
    fatigue_min = 0;
    fatigue_max = 100;
    enemy_id = 1;
    enemy_hp_min = 0;
    enemy_hp_max = 100;
    actor_id = 1;
    actor_hp_min = 0;
    actor_hp_max = 100;

    turn_enemy = false;
    turn_actor = false;
    command_actor = false;
    turn_enemy_id = 1;
    turn_enemy_a = 0;
    turn_enemy_b = 0;
    turn_actor_id = 1;
    turn_actor_a = 0;
    turn_actor_b = 0;
    command_actor_id = 1;
    command_id = 1;
}
