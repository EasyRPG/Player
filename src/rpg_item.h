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

#ifndef _RPG_ITEM_H_
#define _RPG_ITEM_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>

////////////////////////////////////////////////////////////
/// RPG::Item class
////////////////////////////////////////////////////////////
namespace RPG {
    class Item {
    public:
        enum Type {
            Type_normal = 0,
            Type_weapon = 1,
            Type_shield = 2,
            Type_armor = 3,
            Type_helmet = 4,
            Type_accesssory = 5,
            Type_medicine = 6,
            Type_book = 7,
            Type_material = 8,
            Type_special = 9,
            Type_switch = 10
        };

        Item();
        
        int ID;
        std::string name;
        std::string description;
        int type;
        int price;
        std::vector<bool> actor_set;
        std::vector<bool> class_set; // RPG2003
        int uses; // Consumables only

        // Equipable        
        int atk_points;
        int def_points;
        int spi_points;
        int agi_points;
        std::vector<bool> attribute_set;
        std::vector<bool> state_set;
        bool state_chance;
        bool cursed; // RPG2003
                
        // Weapon
        bool two_handed;
        int sp_cost;
        int hit;
        int critical_hit;
        int animation_id;
        bool preemptive;
        bool dual_attack;
        bool attack_all;
        bool ignore_evasion;
        int weapon_animation; // RPG2003
        bool use_skill; // RPG2003
        
        // Shield Armor Head Other
        bool prevent_critical;
        bool raise_evasion;
        bool half_sp_cost;
        bool no_terrain_damage;
        
        // Medicine
        bool entire_party;
        int recover_hp;
        int recover_hp_rate;
        int recover_sp;
        int recover_sp_rate;
        bool ocassion_field; // Switch
        bool ko_only;
        
        // Book - Unique
        int skill_id;
        int using_messsage;
        
        // Material (See also Equipable *_points)
        int max_hp_points;
        int max_sp_points;
        
        // Switch
        int switch_id;
        bool ocassion_battle;
    };
};

#endif
