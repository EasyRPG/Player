#ifndef GO_SCENE_H_
#define GO_SCENE_H_

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

// =============================================================================
// *****************************************************************************



class GO_Scene: public Scene {

private:

	Sprite title;
	void action();

public:


	//GO_Scene();		///constructor
	//~GO_Scene();		///destructor
	void init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam);
	void update(SDL_Surface* Screen);
	void updatekey();
	void dispose();
};


#endif