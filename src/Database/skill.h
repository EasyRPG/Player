#ifndef __skill__
#define __skill__

/* TODO: Move this struct to somewhere else */
typedef struct {
    std::string name = "";
    int volume = 100;
    int tempo = 0;
    int balance = 0;
} sound_effect;
/* ---------------------------------------- */

class Skill {
    
    public:
        Skill();
        
        int id;
        std::string name;
        std::string icon_name;
        std::string description;
        std::string text;
        std::string text2;
        int failure_msg;
        int spell_type;
        int switch_on;
        char scope;
        char occasion;
        int animation1_id;
        int animation2_id;
        sound_effect sound;
        AudioFile menu_se;
        bool field_usg;
        bool combat_usg;
        bool affect_hp;
        bool affect_mp;
        bool affect_str;
        bool affect_pdef;
        //bool affect_mdef; Not available in rm 2k/2k3
        bool affect_int;
        bool affect_agi;
        bool affect_resistance;
        bool absorb_dmg;
        bool ignore_def;
        int common_event_id;
        int sp_cost;
        int power,
            atk_f,
            eva_f,
            str_f,
            dex_f,
            agi_f,
            int_f,
            hit,
            pdef_f,
            mdef_f,
            variance;
        std::vector<char> change_condition;
        std::vector<char> attributes;
        /*std::vector<int> element_set,
                         plus_state_set,
                         minus_state_set;*/
        
        
    
};


#endif
