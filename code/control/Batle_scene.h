#ifndef BATLE_SCENE_H_
#define BATLE_SCENE_H_


// *****************************************************************************
// =============================================================================

#include <SDL/SDL.h>

#include <SDL/SDL_image.h>

#include <SDL/SDL_mixer.h>

#include "../sprites/sprite.h"

#include "../tools/key.h"

#include "../tools/font.h"

#include "../tools/audio.h"

#include "../sprites/map.h"

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
