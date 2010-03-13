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
#include "rpg_skill.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
RPG::Skill::Skill() {
    ID = 0;
    name = "";
    type = 0;
    sp_type = 0;
    sp_cost = 0;
    sp_percent = 1;
    description = "";
    scope = 0;
    animation_id = 0;
    using_message1 = "";
    using_message2 = "";
    failure_message = 0;
    pdef_f = 0;
    mdef_f = 3;
    variance = 4;
    power = 0;
    hit = 100;
    affect_hp = false;
    affect_sp = false;
    affect_attack = false;
    affect_defense = false;
    affect_spirit = false;
    affect_agility = false;
    absorb_damage = false;
    ignore_defense = false;
    state_effect = false;
    affect_attr_defence = false;
    battler_animation = 1;
    switch_id = 1;
    occasion_field = true;
    occasion_battle = false;
}
