#ifndef __enemy__
#define __enemy__
#include "rpg_e_action.h"
#include <string>
#include <vector>

namespace RPG {
class Enemy {
    public:
        Enemy();
        
        int id;
        std::string name;
        std::string battler_name;
        int battler_hue;
        int maxhp;
        int maxmp;
        int str;
        int agi;
        int iint;
        int pdef;
        bool transparent;
        int exp;
        int gold;
        int item_id;
        char treasure_prob;
        bool critical_hit;
        char critical_hit_chance;
        bool miss;
        bool levitate;
        std::vector<char> state_ranks;
        std::vector<char> attribute_ranks;
        std::vector<E_Action*> actions;
};

}
#endif
