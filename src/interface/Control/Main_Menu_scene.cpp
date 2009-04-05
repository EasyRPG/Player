/*Main_Menu_scene.cpp, Main_Menu_Scene routines.
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

#include "Main_Menu_scene.h"
/*
Main_Menu_Scene::~Main_Menu_Scene()
{
}
*/

Main_Menu_Scene::Main_Menu_Scene()
{
    players=NULL;
	gold=NULL;
	menu=NULL;
	menu_exit=NULL;
}


void Main_Menu_Scene::init(General_data * TheTeam)
{
	int i;
	myteam=TheTeam;

	myaudio=&(TheTeam->musica);
	running=&TheTeam->running;
	NScene=&TheTeam->TheScene;

    players=new Window_Player_Select();
	gold=new Window_Base();
	menu=new Window_Select();
	menu_exit=new Window_Select();

    std::string system_string;
    system_string.append("System/");
    system_string.append(TheTeam->ldbdata->System_dat->System_graphic);
    system_string.append(".png");

	menu->init(myteam, running, 0,5, 96, 125, 0, 0,(char *)system_string.c_str());
	players->init(myteam,  running,0,((*myteam).Players.get_size()-1),224,240,96,0,166,48,(char *)system_string.c_str());
	players->init_curXY(55,5); //ya eran muchos comandos
	gold->init(96,40,0,200,(char *)system_string.c_str());
	menu_exit->init(myteam,  running, 0,2, 96, 67, 112, 86,(char *)system_string.c_str());
	str_Vector.push_back("Objetos ");
	str_Vector.push_back("Técnicas ");
	str_Vector.push_back("Equipamiento");
	str_Vector.push_back("Estados");
	str_Vector.push_back("Guardar");
	str_Vector.push_back("Fin partida");
	menu->setComands(& str_Vector);
	str_Vector_2.push_back("Ir a titulo");
	str_Vector_2.push_back("Salir");
	str_Vector_2.push_back("Cancelar");
	menu_exit->setComands(& str_Vector_2);
	menu_exit->visible=false;

	gold->add_text("Gold",5,5);
	char stringBuffer[255];
	sprintf(stringBuffer, "$ %d", ((*myteam).get_Gold()));
	gold->add_text(stringBuffer,5,20);
	int space=60;

	for(i=0;i<(*myteam).Players.get_size();i++)
	{
		players->add_sprite(((*myteam).Players.get_faceset(i)),5,5+(i*space));
		players->add_text(((*myteam).Players.get_name(i)),55,2+(i*space));
		players->add_text(((*myteam).Players.get_job(i)),150,2+(i*space));

		sprintf(stringBuffer, "Level %d  Normal", ((*myteam).Players.get_Level(i)));
		players->add_text(stringBuffer,55,20+(i*space));
		sprintf(stringBuffer, "Exp %d / %d", ((*myteam).Players.get_Exp(i)), ((*myteam).Players.get_MaxExp(i)));
		players->add_text(stringBuffer,55,37+(i*space));
		sprintf(stringBuffer, "Hp %d / %d", ((*myteam).Players.get_HP(i)), ((*myteam).Players.get_MaxHP(i)));
		players->add_text(stringBuffer,150,20+(i*space));
		sprintf(stringBuffer, "Mp %d / %d", ((*myteam).Players.get_MP(i)), ((*myteam).Players.get_MaxMP(i)));
		players->add_text(stringBuffer,150,37+(i*space));
	}
	retardo =0;
}

void Main_Menu_Scene::update(SDL_Surface* Screen)
{
	// static int retardo =0;
	if(retardo==0)
	{
		SDL_FillRect(Screen, NULL, 0x0);// Clear screen
		gold->draw(Screen);
		players->draw(Screen);
		menu->draw(Screen);
	}
	retardo++;

	if(retardo==5)
	{
		players->draw(Screen);
		menu->draw(Screen);
		menu_exit->draw(Screen);
		retardo=1;
        myteam->screen_got_refresh=true;

	}
}

void Main_Menu_Scene::action()
{
	int i;
	for(i=1;i<4;i++)
	{
		if(menu->getindexY()==i)
		{
			players->visible=true;
		}
	}

	if(menu->getindexY()==0)
	{
		* NScene=5;
	}
	if(menu->getindexY()==4)
	{
		* NScene=9;
	}
	if(menu->getindexY()==5)
	{
		menu_exit->visible=true;
	}
}

void Main_Menu_Scene::action2()
{
	if(menu_exit->getindexY()==0)
	{
	    menu_exit->restarmenu();
	    menu->restarmenu();
		 myteam->Players.clear_team();
         myteam->clear_obj();
		 * NScene=0;
	}
	if(menu_exit->getindexY()==1)
	{
		(*running)=false;
	}
	if(menu_exit->getindexY()==2)
	{
		menu_exit->visible=false;
		menu_exit->restarmenu();
		menu->restarmenu();
		retardo=0;
 	}
}

void Main_Menu_Scene::action3()
{
	int i;
	(*myteam).select=players->getindexY();
	for(i=1;i<4;i++)
	{
		if(menu->getindexY()==i)
		{
			* NScene=5+i;
		}
	}
}

void Main_Menu_Scene::updatekey()
{

	if(players->visible)
	{
		players->updatekey();
		if(players->desition())
		action3();
	}

	if(menu_exit->visible)
	{
		menu_exit->updatekey();
		if(menu_exit->desition())
		action2();
	}
	else
	{
		menu->updatekey();
		if(menu->desition())
		{
			action();
		}
	}

       if(menu->menu.cancel)
        {
       // myaudio->load("Sound/Cansel2.wav");
		*NScene = 1;
        }

}
void Main_Menu_Scene::dispose() {
	menu->dispose();
	delete menu;
	menu=NULL;

	players->dispose();
	delete players;
    players=NULL;

	gold->dispose();
	delete gold;
	gold=NULL;

	menu_exit->dispose();
	delete menu_exit;
    menu_exit=NULL;
}
