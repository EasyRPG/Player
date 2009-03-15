#ifndef MOVE_MANAGEMENT_H_
#define MOVE_MANAGEMENT_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include "General_data.h"

#define DIRECTION_UP      0x00
#define DIRECTION_DOWN    0x01
#define DIRECTION_LEFT    0x02
#define DIRECTION_RIGHT   0x03

class General_data;// a promise to the compiler that eventually supply a complete definition of that class.

class Mv_management{

private:

	std:: vector <Chara> * Charas_nps;//agregar apuntador a vector de eventos
	CActor * Actor;
	Chipset * chip;
	map_data * data;


public:

    void init(General_data * TheTeam);
    bool npc_colision(int x, int y,int e);
    int get_dir(int i);
    int custom(int i);
    int escape_hero(int i);
    int to_hero(int i);
    int left_right(int i);
    int up_down(int i);
    int random_move(int i);

};

#endif
