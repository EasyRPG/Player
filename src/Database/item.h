#ifndef __item__
#define __item__

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
        item_type type;
        int price;
        char n_uses;
        int atk_change;
        int pdef_change;
        int int_change;
        int agi_change;
        bool both_hands;
        int mp_cost;
        char hit;
        int animation_hit;
        bool preventive;
        bool double_attack;
        bool attack_all;
        bool ignore_evasion;
        bool prevent_crits;
        bool raise_evasion;
        bool half_mp;
        bool no_terrain_dmg;
        bool heals_party;
        int recover_hp;
        char recover_hp_rate;
        int recover_mp;
        char recover_mp_rate;
        bool field_only;
        bool dead_only;
        int max_hp_mod;
        int max_mp_mod;
        int str_mod;
        int pdef_mod;
        int int_mod;
        int agi_mod;
        bool use_msg;
        int switch_on;
        bool use_on_field;
        bool use_in_battle;
        std::vector<char> heros_can_use;
        std::vector<char> conditions;
        std::vector<char> attributes;
        char chance_cond;
        
    

};

#endif
