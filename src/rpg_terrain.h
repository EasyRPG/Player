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
        std::string battle_bkg_name;
        bool skiff_may_pass;
        bool boat_may_pass;
        bool airship_may_pass;
        bool airship_may_land;
        int hero_opacity;    
};

}


#endif
