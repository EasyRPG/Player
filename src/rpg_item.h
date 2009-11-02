#ifndef __item__
#define __item__
#include <string>
#include <vector>

namespace RPG {

typedef enum {
    COMMON = 0,
    WEAPON, 
    SHIELD,
    ARMOR,
    HELMET,
    ACCESSORY,
    RECOVERY,
    LEARN_SKILL,
    CHANGE_STATS,
    INVOKE_SKILL,
    SWITCH
} item_type;

class Item {
    public:
        Item();
        
        int id;
        std::string name;
        std::string description;
        int type;
        int price;
		std::vector<char> actor_set;
		int uses; // Consumables

		// Equipable		
		int atk_points;
        int pdef_points;
        int int_points;
        int agi_points;
        std::vector<char> attribute_set;
		std::vector<char> state_set;
        char state_chance;
				
		// Weapon
		bool two_handed;
		int sp_cost;
        char hit;
        int critical_hit;
        int animation_id;
        bool preemptive;
        bool dual_attack;
        bool attack_all;
        bool ignore_evasion;
		
		// Shield Armor Head Other
		bool prevent_critical;
        bool raise_evasion;
		bool half_sp_cost;
		bool no_terrain_damage;
		
		// Medicine
		bool entire_party;
        int recover_hp;
        char recover_hp_rate;
        int recover_sp;
        char recover_sp_rate;
        bool ocassion_field; // Switch
        bool ko_only;
		
        // Book - Unique
        int skill_id;
		bool using_messsage;
		
        // Material (See also Equipable points)
        int max_hp_points;
        int max_sp_points;
		
		// Switch
		int switch_id;
        bool ocassion_battle;
};
}

#endif
