#ifndef __testbattler__
#define __testbattler__
#include <string>

namespace RPG {
    
class TestBattler {
    public:
        TestBattler();
        
        int actor_id;
        int level;
        int weapon_id;
        int shield_id;
        int armor_id;
        int head_id;
        int accessory_id;
};

}

#endif
