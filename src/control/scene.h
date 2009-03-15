/*Scene.h, EasyRPG player Scene class declaration file.
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

#ifndef SCENE_H_
#define SCENE_H_



// *****************************************************************************
// =============================================================================

#include "../tools/audio.h"

#include "General_data.h"

// =============================================================================
// *****************************************************************************

class Scene {

protected:
	unsigned char * NScene;
	bool * running;
	Audio * myaudio;
	General_data * myteam;
	int retardo;

public:


	//Scene();		///constructor
	//~Scene();		///destructor
	virtual void update(SDL_Surface*){} ;
	virtual void updatekey(){} ;
	virtual void dispose(){} ;

};

#endif
