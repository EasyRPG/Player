/*GameOver_Scene.h, EasyRPG player GO_Scene class declaration file.
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

#ifndef GO_SCENE_H_
#define GO_SCENE_H_

// *****************************************************************************
// =============================================================================




#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "../sprites/sprite.h"

#include "../tools/key.h"

#include "../tools/font.h"

#include "../tools/audio.h"


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
