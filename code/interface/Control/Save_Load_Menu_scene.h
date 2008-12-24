#ifndef SAVE_LOAD_MENU_SCENE_H_
#define SAVE_LOAD_MENU_SCENE_H_


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

class Save_Load_Menu_Scene: public Scene 
{

private:
	Window_Base descripcion;
	Window_Base Save_pos_1;
	Window_Base Save_pos_2;
	Window_Base Save_pos_3;

public:
	//Save_Load_Menu_Scene();		///constructor
	//~Save_Load_Menu_Scene();		///destructor
	void init(Audio * myaudio,  bool * run,unsigned char * TheScene,Player_Team * TheTeam);
	void update(SDL_Surface* Screen);
	void updatekey();
	void dispose();

};

#endif
