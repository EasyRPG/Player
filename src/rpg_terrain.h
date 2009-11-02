#ifndef __terrain__
#define __terrain__
#include <string>
namespace RPG {

class Terrain {
    public:
        Terrain();
        
        int id;
        std::string name;
        int damage;
        char encounter_rate;
        std::string battle_background;
        bool ship_pass;
        bool boat_pass;
        bool airship_pass;
        bool airship_land;
        int chara_opacity;    
};

}


#endif
