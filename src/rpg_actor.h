#ifndef __actor__
#define __actor__
#include <string>
#include <vector>

namespace RPG {
	
typedef struct {
	int level;
	int skill_id;
} Learning;

class Actor {
    public:
        Actor();
        
        int id;
		std::string name;
		std::string title;
		std::string character_name;
		int character_index;
		bool transparent;
		int initial_level;
		int final_level;
		bool critical_hit;
		int critical_hit_chance;
		std::string face_name;
		int face_index;
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
		int weapon_id;
		int shield_id;
		int armor_id;
		int head_id;
		int accessory_id;
		std::vector <Learning> skills;
		int condition_size;
		std::vector<char> condition_effects;
		int attribute_size;
		std::vector<char> attribute_effects;
		
		bool rename_skill; // 2000
		std::string skill_name; // 2000
		
		int class_id; // 2003
		int battler_animation; // 2003
};

}
#endif
