/*Stats_Menu_scene.h, EasyRPG player Stats_Menu_Scene class declaration file.
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

#ifndef STATS_MENU_SCENE_H_
#define STATS_MENU_SCENE_H_

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "../../sprites/sprite.h"
#include "../../tools/key.h"
#include "../../tools/font.h"
#include "../../tools/audio.h"
#include "../../sprites/map.h"
#include <vector>
#include <string>
#include "../../attributes/skill.h"
#include "../../attributes/item.h"
#include "../../attributes/Enemy.h"
#include "../../attributes/Player.h"
#include "../../tools/math-sll.h"
#include "../../attributes/CActor.h"
#include "../../control/scene.h"
#include "../Windows/Window_Base.h"

class Stats_Menu_Scene: public Scene 
{

private:
	Window_Base Status;
	Window_Base Status_vit;
	Window_Base Profesion;
	Window_Base Aramas;

public:
	//Stats_Menu_Scene();		///constructor
	//~Stats_Menu_Scene();		///destructor
	void init(Audio * myaudio,  bool * run,unsigned char * TheScene,Player_Team * TheTeam);
	void update(SDL_Surface* Screen);
	void updatekey();
	void dispose();

};

#endif
