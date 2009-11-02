#include "rpg_sound.h"

namespace RPG {
	
Sound::Sound()
{
	name = "";
    volume = 100;
    tempo = 100;
    balance = 0;
}

Sound::Sound(std::string n, int v, int t, int b)
{
	name = n;
    volume = v;
    tempo = t;
    balance = b;
}
}
