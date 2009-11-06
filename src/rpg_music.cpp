#include "rpg_music.h"

RPG::Music::Music()
{
	name = "";
    volume = 100;
    tempo = 100;
    balance = 0;
}

RPG::Music::Music(std::string n, int v, int t, int b)
{
	name = n;
    volume = v;
    tempo = t;
    balance = b;
}
