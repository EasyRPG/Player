/*Batle_scene.h, EasyRPG player Batle_scene class declaration file.
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

#ifndef BATLE_SCENE_H_
#define BATLE_SCENE_H_


// *****************************************************************************
// =============================================================================

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "../sprites/sprite.h"

#include "../tools/key.h"

#include "../tools/font.h"

#include "../tools/audio.h"


#include <vector>

#include <string>

#include "../attributes/skill.h"

#include "../attributes/item.h"

#include "../attributes/Enemy.h"

#include "../attributes/Player.h"

#include "../tools/math-sll.h"

#include "../attributes/CActor.h"

#include "scene.h"

#include "../interface/Windows/Window_Base.h"

#include "../interface/Windows/Window_Player_Select.h"

#include "../interface/Windows/Window_Select.h"



// =============================================================================


// *****************************************************************************




class Batle_scene: public Scene {


private:


	bool * the_run;

	int state;

	int Nmenu_used;

	int turnosp;

	int player_in_turn;

	int moster_in_turn;

	Sprite title;

	Window_Base Window_text;

	Window_Player_Select window;

	Window_Select menu;

	Window_Select menu_os;

	Window_Select moster_select;

	std:: vector <std::string> str_Vector;

	std:: vector <std::string> str_Vector2;

	std:: vector <MC> Comands;

	void action();


public:


	//Batle_scene();		///constructor

	//~Batle_scene();		///destructor
	void init(Audio * myaudio,  bool * run,unsigned char * TheScene,Player_Team * TheTeam);

	void update(SDL_Surface* Screen);

	void updatekey();

	void atack(SDL_Surface* Screen,int nperso,int enemy);

	void update_window_stats();

	void update_window_mosterselect();

	void action_mosterselect();

	void windowtext_showdamange(bool type,int atak,int ataked,int damange);

	void Give_turn();

	void win();

	void lose();

	void atacked(int enemy);

	void dispose();


};


#endif
