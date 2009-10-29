#ifndef __actor__
#define __actor__
#include <string>

typedef struct {
    int level;
    int skill_id;
} skill_block;

class Actor {
    public:
        Actor();
        
        int id; // The actor ID.
        std::string name; // The actor name.
        std::string title_name;
        int class_id; // The actor class ID.
        int initial_level; // The actor's initial level.
        int final_level; // The actor's final level.
        int exp_basis; // The value on which the experience curve is based (10..50).
        int exp_inflation; // The amount of experience curve inflation (10..50).
        int critical_hit;
        int critical_chance;
        int graphic_index;
        std::string face_name;
        int face_index;
        std::string character_name; // The actor's character graphic file name.
        bool transparent;
        int character_hue; // The adjustment value for the character graphic's hue (0..360).
        std::string battler_name; // The actor's battler graphic file name.
        int battler_hue; // The adjustment value for the battler graphic's hue (0..360).
        /*std::vector<int, int> parameters; // 2-dimensional array containing base parameters for each level.*/
        std::vector<short> stats[6];
        bool dual_weapon;
        bool lock_equipment;
        bool ia_control;
        bool high_defense;
        int exp_base_line;
        int exp_additional;
        int exp_correction:
        short weapon_id;
        short armor1_id; // Shield
        short armor2_id; //Helmet
        short armor3_id; // Armor
        short armor4_id; // Accessory
        bool weapon_fix;
        bool armor1_fix;
        bool armor2_fix;
        bool armor3_fix;
        bool armor4_fix;      
        std::vector<skill_block> skills;
        bool skill_rename;
        std::string skill_name;
        std::vector<int> combat_command; // ???? 2003
        std::vector<char> attribute_effects; // ????
    
};



#endif
