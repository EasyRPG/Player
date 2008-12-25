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
Main_Menu_Scene::Main_Menu_Scene()
{
}
*/

void Main_Menu_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{  
	int i;
	myteam=TheTeam;
	myaudio=theaudio;
	menu.init( theaudio, run, 0,5, 96, 125, 0, 0);
	players.init(theaudio, run,0,((*myteam).get_size()-1),224,240,96,0,166,48);
	players.init_curXY(55,5); //ya eran muchos comandos
	gold.init(96,40,0,200);
	menu_exit.init( theaudio, run, 0,2, 96, 67, 112, 86);
	str_Vector.push_back("Objetos ");
	str_Vector.push_back("Técnicas ");
	str_Vector.push_back("Equipamiento");
	str_Vector.push_back("Estados");
	str_Vector.push_back("Guardar");
	str_Vector.push_back("Fin partida");
	menu.setComands(& str_Vector);
	str_Vector_2.push_back("Ir a titulo");
	str_Vector_2.push_back("Salir");
	str_Vector_2.push_back("Cancelar");
	menu_exit.setComands(& str_Vector_2);
	menu_exit.visible=false;
	running=  run;
	NScene=TheScene;

	gold.add_text("Gold",5,5);
	char stringBuffer[255];
	sprintf(stringBuffer, "$ %d", (*(*myteam).get_Gold()));
	gold.add_text(stringBuffer,5,20);
	int space=60;

	for(i=0;i<(*myteam).get_size();i++)
	{
		players.add_sprite(((*myteam).get_faceset(i)),5,5+(i*space));
		players.add_text(((*myteam).get_name(i)),55,2+(i*space));
		players.add_text(((*myteam).get_job(i)),150,2+(i*space));

		sprintf(stringBuffer, "Level %d  Normal", (*(*myteam).get_Level(i)));
		players.add_text(stringBuffer,55,20+(i*space));
		sprintf(stringBuffer, "Exp %d / %d", (*(*myteam).get_Exp(i)), (*(*myteam).get_MaxExp(i)));
		players.add_text(stringBuffer,55,37+(i*space));
		sprintf(stringBuffer, "Hp %d / %d", (*(*myteam).get_HP(i)), (*(*myteam).get_MaxHP(i)));
		players.add_text(stringBuffer,150,20+(i*space));
		sprintf(stringBuffer, "Mp %d / %d", (*(*myteam).get_MP(i)), (*(*myteam).get_MaxMP(i)));
		players.add_text(stringBuffer,150,37+(i*space));
	}
	retardo =0;
}

void Main_Menu_Scene::update(SDL_Surface* Screen)
{  
	// static int retardo =0;
	if(retardo==0)
	{
		SDL_FillRect(Screen, NULL, 0x0);// Clear screen
		gold.draw(Screen);
		players.draw(Screen);
		menu.draw(Screen);
	}
	retardo++;

	if(retardo==5)
	{
		players.draw(Screen);
		menu.draw(Screen);
		menu_exit.draw(Screen);
		retardo=1;
	}
}

void Main_Menu_Scene::action()
{ 
	int i;
	for(i=1;i<4;i++)
	{
		if(menu.getindexY()==i)
		{ 
			players.visible=true;
		}
	}

	if(menu.getindexY()==0)
	{
		* NScene=5;
	}
	if(menu.getindexY()==4)
	{
		* NScene=9;
	}
	if(menu.getindexY()==5)
	{
		menu_exit.visible=true;
	}
}

void Main_Menu_Scene::action2()
{
	if(menu_exit.getindexY()==0)
	{
		* NScene=0;
	}
	if(menu_exit.getindexY()==1)
	{	
		(*running)=false;
	}
	if(menu_exit.getindexY()==2)
	{
		menu_exit.visible=false;
		menu_exit.restarmenu();
		menu.restarmenu();
		retardo=0;
 	}
}

void Main_Menu_Scene::action3()
{ 
	int i;
	(*myteam).select=players.getindexY();
	for(i=1;i<4;i++)
	{
		if(menu.getindexY()==i)
		{	
			* NScene=5+i;
		}
	}
}

void Main_Menu_Scene::updatekey() 
{

	if(players.visible)
	{
		players.updatekey();
		if(players.desition())
		action3();
	}

	if(menu_exit.visible)
	{
		menu_exit.updatekey();
		if(menu_exit.desition())
		action2();
	}
	else
	{
		menu.updatekey();
		if(menu.desition())
		{
			action();
		}
	}
	if (Key_press_and_realsed(LMK_X ))
	{
		(*myaudio).soundload("../Sound/Cansel2.wav");
		* NScene=1; 
	}
}
void Main_Menu_Scene::dispose() {
	menu.dispose();
	players.dispose();
	gold.dispose();
	menu_exit.dispose();
}
