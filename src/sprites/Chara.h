/*Chara.h, EasyRPG player sprite class declaration file.
    Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef CHARA_H_
#define CHARA_H_



// *****************************************************************************

// =============================================================================

#include "SDL.h"


#include "SDL_rotozoom.h"

#include "../tools/tools.h"

#include "SDL_image.h"

#include <iostream>

#include <stdlib.h>

#include "sprite.h"

// =============================================================================
// *****************************************************************************
#define STATE_IDLE        0x00
#define STATE_MOVING      0x01

#define DIRECTION_UP      0x00
#define DIRECTION_DOWN    0x01
#define DIRECTION_LEFT    0x02
#define DIRECTION_RIGHT   0x03



class Chara: public Sprite {

private:
    int animation[4][4];//up right down left XP
    int delay;
    int move_delay;
    int distance;
    int speed_delay;

public:
    int id;
    int actual_move;
    bool state;
    bool nomalanimation;
    int GridX;
    int GridY;
    int dir;
    int move_dir;
    int layer;
    int anim_frec;
    int move_frec;
    bool move_frec_check();
    void rotationupdate();
    bool move(int direction);
    void init_Chara();
    void setimg(const char* string,int id);
    void setposXY(int xi,int yi);
    void frame_ori();
    void frameupdate();
   	void drawc(SDL_Surface *screen);
};




#endif
