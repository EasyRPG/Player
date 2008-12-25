/*GameOver_Scene.cpp, GO_Scene routines.
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



#include "GameOver_scene.h"


/*GO_Scene::~GO_Scene()
{

}
GO_Scene:: GO_Scene()
{

}
*/

void GO_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{    
	myteam=TheTeam;
	(*myteam).clear_team();
	(*myteam).clear_obj();
	myaudio=theaudio;
	(*myaudio).musicload("../Music/2003sorrow.mid");
	title.x=0;
	title.y=0;
	title.setimg("../GameOver/gameover.png");
	running=  run;
	NScene=TheScene;
}

void GO_Scene::update(SDL_Surface* Screen)
{
	title.draw(Screen);
}

void GO_Scene::action()
{
	*NScene=0;
}

void GO_Scene::updatekey() 
{
	unsigned char * keyData;
	static int delay=0;
	delay++;
	if(delay==40)
	{
		keyData = SDL_GetKeyState(NULL);
		if ( keyData[SDLK_ESCAPE] ) 
		{
			* running = false;
		}
		if ( keyData[LMK_Z]  ) 
		{
			action();
		}
		delay=0;
	}
}

void GO_Scene::dispose() {
	title.dispose();
}
