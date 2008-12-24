#ifndef MAP_SCENE_H_
#define MAP_SCENE_H_


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

class Map_Scene: public Scene {

private:

	int SCREEN_SIZE_X, SCREEN_SIZE_Y;
	CActor Actor;
	Chara * player;
	Chara npc;
	Faceset alexface;
	Sprite red;
	Font fuente;
	bool moving;
	int to_move;
	stMap Map;
	char stringBuffer[255];

public:


	//Map_Scene();		///constructor
	//~Map_Scene();		///destructor
	void init(Audio * audio,int SCREEN_X, int SCREEN_Y,unsigned char * TheScene,Player_Team * TheTeam);
	void update(SDL_Surface* Screen);

	void Scroll();
	void updatekey();
	void slow_move();
	void mapnpc();
	void dispose();

};



#endif
