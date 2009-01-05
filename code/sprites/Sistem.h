/*  Sistem.h, EasyRPG player sprite class declaration file.
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

#ifndef SISTEM_H_
#define SISTEM_H_


// *****************************************************************************
// =============================================================================

#include "SDL.h"

#include "SDL_rotozoom.h"

#include "../tools/tools.h"

#include "SDL_image.h"

#include <iostream>

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
