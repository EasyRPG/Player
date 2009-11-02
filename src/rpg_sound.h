#ifndef __sound__
#define __sound__

#include <string>

namespace RPG {
	
class Sound {

public:
	Sound();
	Sound(std::string n, int v, int t, int b);
	
	std::string name;
    int volume;
    int tempo;
    int balance;
};
}
#endif // __sound__
