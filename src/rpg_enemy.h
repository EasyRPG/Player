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

#ifndef _RPG_ENEMY_H_
#define _RPG_ENEMY_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rpg_enemyaction.h"

////////////////////////////////////////////////////////////
/// RPG::Enemy class
////////////////////////////////////////////////////////////
namespace RPG {
    class Enemy {
    public:
        Enemy();
        
        int ID;
        std::string name;
        std::string battler_name;
        int battler_hue;
        int max_hp;
        int max_sp;
        int attack;
        int defense;
        int spirit;
        int agility;
        bool transparent;
        int exp;
        int gold;
        int drop_id;
        int drop_prob;
        bool critical_hit;
        int critical_hit_chance;
        bool miss;
        bool levitate;
        std::vector<unsigned char> state_ranks;
        std::vector<unsigned char> attribute_ranks;
        std::vector<EnemyAction> actions;
    };
};

#endif
