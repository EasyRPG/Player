#ifndef _H_STATE_
#define _H_STATE_
#include <string>

namespace RPG {

class State {
    public:
        State();
        
        int id;
        std::string name;
        bool battle_only;
        int color;
        int priority;
        int restriction; //Sets restrictions (0: none, 1: can't use magic, 2: always attack enemies, 3: always attack allies, 4: can't move).
        int A_chance;
        int B_chance;
        int C_chance;
        int D_chance;
        int E_chance;
        int hold_turn;
        int auto_release_prob;
        int shock_release_prob;
        int type_change; //0: reducir a la mitad 1: doble 2: No cambiar
        bool atk_mod;
        bool pdef_mod;
        bool int_mod;
        bool agi_mod;
        int rate_change;
        bool prevent_skill_use;
        int min_skill_lvl;
        bool prevent_magic_use;
        int min_magic_lvl;
        std::string ally_enter_state;
        std::string enemy_enter_state;
        std::string already_in_state;
        std::string affected_by_state;
        std::string status_recovered;
        int hp_loss;
        int hp_loss_value;
        int hp_map_loss;
        int hp_map_steps;
        int mp_loss;
        int mp_loss_value;
        int mp_map_loss;
        int mp_map_steps;
        
        
        
        
        
};
    
}

#endif 
