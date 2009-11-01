#ifndef _h_attribute__
#define _h_attribute__
#include <string>

namespace RPG {

class Attribute {
    public:
        Attribute();
        
        int id;
        std::string name;
        int type;
        int A_damage;
        int B_damage;
        int C_damage;
        int D_damage;
        int E_damage;
};
    
}

#endif
