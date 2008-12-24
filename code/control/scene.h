#ifndef SCENE_H_
#define SCENE_H_



// *****************************************************************************
// =============================================================================

#include "../tools/audio.h"

#include "../attributes/Player_Team.h"


// =============================================================================
// *****************************************************************************

class Scene {

protected:
	unsigned char * NScene;
	bool * running;
	Audio * myaudio;
	Player_Team * myteam;
	int retardo;

public:


	//Scene();		///constructor
	//~Scene();		///destructor
	virtual void update(SDL_Surface*){} ;
	virtual void updatekey(){} ;
	virtual void dispose(){} ;

};

#endif
