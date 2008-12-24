#ifndef TITLE_SCENE_H_
#define TITLE_SCENE_H_



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

#include "../interface/Windows/Window_Select.h"





// =============================================================================
// *****************************************************************************


class Title_Scene: public Scene {

private:

	Sprite title;
	Window_Select menu;
	std:: vector <std::string> str_Vector;
	void action();

public:


	//Title_Scene();		///constructor
	//~Title_Scene();		///destructor
	void init(Audio * myaudio,  bool * run,unsigned char * TheScene,Player_Team * TheTeam);
	void update(SDL_Surface* Screen);
	void init_party();
	void updatekey();
	void dispose();
};



#endif
