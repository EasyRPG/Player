#ifndef EQUIP_MENU_SCENE_H_
#define EQUIP_MENU_SCENE_H_

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
#include "../Windows/Window_Select.h"

class Equip_Menu_Scene: public Scene 
{

private:
	bool * therun;
	Window_Base descripcion;
	Window_Base stats;
	Window_Select Armas;
	Window_Select menu;
	std:: vector <std::string> str_Vector;
	std:: vector <std::string> str_Vector2;
	void action();

public:
	//Euip_Menu_Scene(); 	///constructor
	//~Euip_Menu_Scene();		///destructor
	void init(Audio * myaudio,  bool * run,unsigned char * TheScene,Player_Team * TheTeam);
	void update(SDL_Surface* Screen);
	void updatekey();
	void update_1_menu();
	void update_2_menu();
	void action2();
	void dispose();

};

#endif
