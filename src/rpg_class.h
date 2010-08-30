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

#ifndef _RPG_CLASS_H_
#define _RPG_CLASS_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rpg_learning.h"

////////////////////////////////////////////////////////////
/// RPG::Class class
////////////////////////////////////////////////////////////
namespace RPG {
    class Class {
    public:
        Class();
        
        int ID;
        std::string name;
        bool two_swords_style;
        bool fix_equipment;
        bool auto_battle;
        bool super_guard;
        std::vector<short> parameter_maxhp;
        std::vector<short> parameter_maxsp;
        std::vector<short> parameter_attack;
        std::vector<short> parameter_defense;
        std::vector<short> parameter_spirit;
        std::vector<short> parameter_agility;
        int exp_base;
        int exp_inflation;
        int exp_correction;
        int unarmed_animation;
        std::vector<Learning> skills;
        std::vector<unsigned char> state_ranks;
        std::vector<unsigned char> attribute_ranks;
        std::vector<unsigned int> battle_commands;
    };
}

#endif
