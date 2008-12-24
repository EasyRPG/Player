#ifndef SISTEM_H_
#define SISTEM_H_


// *****************************************************************************
// =============================================================================

#include <SDL.h>

#include "../tools/SDL_rotozoom.h"

#include "../tools/tools.h"

#include <SDL_image.h>

#include <stdio.h>

#include <stdlib.h>

#include "sprite.h"


// =============================================================================
// *****************************************************************************



class Sistem: public Sprite {

	private:
	   SDL_Surface *CubeDraw (SDL_Surface * screen,int sizeX,int sizeY);

	public:
	   void init_Sistem();
	   void draw (SDL_Surface * screen,int iframe,int posx,int posy);
	   SDL_Surface * Exdraw (int sizeX,int sizeY);
	   SDL_Surface * ExdrawT (int sizeX,int sizeY,int tipe);
	   SDL_Surface * Cube_select(int type,int sizeX,int sizeY);

};



#endif
