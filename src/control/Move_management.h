#ifndef MOVE_MANAGEMENT_H_
#define MOVE_MANAGEMENT_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include "../sprites/sprite.h"
#include "../sprites/chipset.h"
#include "../sprites/Pre_Chipset.h"
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include "../readers/map.h"
#include "../readers/lmt.h"
#include "../readers/ldb.h"
#include <vector>
#include <string>
#include "../interface/Windows/Window_Base.h"
#include "../attributes/skill.h"
#include "../attributes/item.h"
#include "../attributes/Enemy.h"
#include "../attributes/Player.h"
#include "../tools/math-sll.h"
#include "../attributes/CActor.h"
#include "../attributes/Player_Team.h"

#define DIRECTION_UP      0x00
#define DIRECTION_DOWN    0x01
#define DIRECTION_LEFT    0x02
#define DIRECTION_RIGHT   0x03

class Mv_management{

private:

	std:: vector <Chara> * Charas_nps;//agregar apuntador a vector de eventos
	CActor * Actor;
	Chipset * chip;
	map_data * data;


public:

    void init(Player_Team * TheTeam);
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
