#ifndef __music__
#define __music__

#include <string>

namespace RPG {
    
class Music {
    public:
        Music();
        Music(std::string n, int v, int t, int b);
        
        std::string name;
        int volume;
        int tempo;
        int balance;
};
}
#endif
