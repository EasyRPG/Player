#ifndef __attribute__
#define __attribute__

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
