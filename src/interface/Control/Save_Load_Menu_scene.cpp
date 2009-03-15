/*Save_Load_Menu_scene.cpp, Save_Load_Menu_Scene routines.
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

#include "Save_Load_Menu_scene.h"

/*
Save_Load_Menu_Scene::~ Save_Load_Menu_Scene()
{
}
Save_Load_Menu_Scene:: Save_Load_Menu_Scene()
{
}
*/

void Save_Load_Menu_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,General_data * TheTeam)
{
	myteam=TheTeam;
	myaudio=theaudio;
   std::string system_string;
    system_string.append("System/");
    system_string.append(myteam->data2.System_dat.System_graphic);
    system_string.append(".png");

	descripcion.init(320,30,0,0,(char *)system_string.c_str());
	descripcion.add_text("¿Dónde deseas guardar la partida? ",10,5);
	Save_pos_1.init(320,70,0,30,(char *)system_string.c_str());
	Save_pos_2.init(320,70,0,100,(char *)system_string.c_str());
	Save_pos_3.init(320,70,0,170,(char *)system_string.c_str());
	running=  run;
	NScene=TheScene;
	retardo=0;
}

void Save_Load_Menu_Scene::update(SDL_Surface* Screen)
{
	retardo++;
	if(retardo==6)
	{
		Save_pos_1.draw(Screen);
		Save_pos_2.draw(Screen);
		Save_pos_3.draw(Screen);
		descripcion.draw(Screen);
		retardo=0;
		 myteam->screen_got_refresh=true;
	}
}

void Save_Load_Menu_Scene::updatekey()
{
int temp,i;
        temp = Control::pop_action();
        switch (temp)
        {
        case CANCEL:
        i=myteam->S_manager.load_sound("Sound/Cancelar.wav");
        myteam->S_manager.play_sound(i);
		* NScene=4;
        break;
        default:
        break;

        }
}

void Save_Load_Menu_Scene::dispose()
{
	Save_pos_1.dispose();
	Save_pos_2.dispose();
	Save_pos_3.dispose();
	descripcion.dispose();
}
