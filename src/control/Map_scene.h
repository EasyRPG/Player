/*Map_Scene.h, EasyRPG player Map_Scene class declaration file.
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

#ifndef MAP_SCENE_H_
#define MAP_SCENE_H_


// *****************************************************************************
// =============================================================================


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
#include "scene.h"
#include "Event_management.h"
#include "Move_management.h"


// =============================================================================
// *****************************************************************************
#define DIRECTION_UP      0x00
#define DIRECTION_DOWN    0x01
#define DIRECTION_LEFT    0x02
#define DIRECTION_RIGHT   0x03

class Map_Scene: public Scene {

private:

	int SCREEN_SIZE_X, SCREEN_SIZE_Y,actual_map;
	CActor Actor;
	Chara * player;
    E_management Ev_management;
    Mv_management Mov_management;
	std:: vector <Chara> Charas_nps;//agregar apuntador a vector de eventos

	bool moving;
	int to_move;
	Pre_Chipset pre_chip;
	Chipset chip;
	map_reader Map;
	map_data data;
    std:: vector <stEventMap> * Events;//agregar apuntador a vector de eventos
	char stringBuffer[255];
public:
	//Map_Scene();		///constructor
	//~Map_Scene();		///destructor
	void init(Audio * audio,int SCREEN_X, int SCREEN_Y,unsigned char * TheScene,Player_Team * TheTeam);
    void load_map();
    void update(SDL_Surface* Screen);
	void Scroll();
	void updatekey();
	void init_npc();
	void mapnpc();
	void dispose();

};



#endif
