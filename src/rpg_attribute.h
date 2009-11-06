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
        int a_rate;
        int b_rate;
        int c_rate;
        int d_rate;
        int e_rate;
};
    
}

#endif
