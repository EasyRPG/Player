/*Stats_Menu_scene.cpp, Stats_Menu_Scene routines.
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

#include "Stats_Menu_scene.h"

/*
Stats_Menu_Scene::~Stats_Menu_Scene()
{
}
Stats_Menu_Scene:: Stats_Menu_Scene()
{
}
*/

void Stats_Menu_Scene::init(General_data * TheTeam)
{
	myteam=TheTeam;

    myaudio=&(TheTeam->musica);
	running=&TheTeam->running;
	NScene=&TheTeam->TheScene;

	   std::string system_string;
    system_string.append("System/");
    system_string.append(myteam->ldbdata->System_dat->System_graphic);
    system_string.append(".png");

	Status_vit.init(200,80,120,0,(char *)system_string.c_str());
	Status.init(200,80,120,80,(char *)system_string.c_str());
	Aramas.init(200,80,120,160,(char *)system_string.c_str());
	Profesion.init(120,240,0,0,(char *)system_string.c_str());
	int i=(*myteam).select;
	int space=17;
	char stringBuffer[255];
	sprintf(stringBuffer, "Hp %d / %d", ((*myteam).Players.get_HP(i)), ((*myteam).Players.get_MaxHP(i)));
	Status_vit.add_text(stringBuffer,10,10);
	sprintf(stringBuffer, "Mp %d / %d", ((*myteam).Players.get_MP(i)), ((*myteam).Players.get_MaxMP(i)));
	Status_vit.add_text(stringBuffer,10,10+(1*space));
	sprintf(stringBuffer, "Exp%d / %d", ((*myteam).Players.get_Exp(i)), ((*myteam).Players.get_MaxExp(i)));
	Status_vit.add_text(stringBuffer,10,10+(2*space));

	sprintf(stringBuffer, "Fuerza%d ", ((*myteam).Players.get_Attack(i)));
	Status.add_text(stringBuffer,10,5);
	sprintf(stringBuffer, "Defenza%d ", ((*myteam).Players.get_Defense(i)));
	Status.add_text(stringBuffer,10,5+(1*space));
	sprintf(stringBuffer, "Intelecto %d ", ((*myteam).Players.get_Spirit(i)));
	Status.add_text(stringBuffer,10,5+(2*space));
	sprintf(stringBuffer, "Agilidad  %d ", ((*myteam).Players.get_Speed(i)));
	Status.add_text(stringBuffer,10,5+(3*space));
	space=40;

	Profesion.add_sprite(((*myteam).Players.get_faceset(i)),10,10);
	Profesion.add_text("Nombre",10,30+(1*space));
	Profesion.add_text(((*myteam).Players.get_name(i)),50,30+(1*space)+16);
	Profesion.add_text("Profesión",10,30+(2*space));
	Profesion.add_text(((*myteam).Players.get_job(i)),50,30+(2*space)+16);
	Profesion.add_text("Estado",10,30+(3*space));
	Profesion.add_text("Normal",50,30+(3*space)+16);//falta el casteo del entero
	Profesion.add_text("Nivel",10,30+(4*space));
	sprintf(stringBuffer, "%d ", ((*myteam).Players.get_Level(i)));

	Profesion.add_text(stringBuffer,50,30+(4*space)+16);//falta el casteo del entero
	space=14;
	Aramas.add_text(  (*((*myteam).Players.get_Weapon(i))).get_name(),75,5);
	Aramas.add_text(  (*((*myteam).Players.get_Shield(i))).get_name(),75,5+(1*space));
	Aramas.add_text(  (*((*myteam).Players.get_Armor(i))).get_name(),75,5+(2*space));
	Aramas.add_text(  (*((*myteam).Players.get_Helmet(i))).get_name(),75,5+(3*space));
	Aramas.add_text(  (*((*myteam).Players.get_Accessory(i))).get_name(),75,5+(4*space));

	Aramas.add_text("Arma",10,5);
	Aramas.add_text("Escudo",10,5+(1*space));
	Aramas.add_text("Armadura",10,5+(2*space));
	Aramas.add_text("Casco",10,5+(3*space));
	Aramas.add_text("Otros",10,5+(4*space));

	retardo=0;
}

void Stats_Menu_Scene::update(SDL_Surface* Screen)
{
	if(retardo==0)
	{
		Status.draw(Screen);
		Status_vit.draw(Screen);
		Profesion.draw(Screen);
		Aramas.draw(Screen);
		retardo++;
		 myteam->screen_got_refresh=true;
	}
}

void Stats_Menu_Scene::updatekey()
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

void Stats_Menu_Scene::dispose()
{
	Status.dispose();
	Status_vit.dispose();
	Profesion.dispose();
	Aramas.dispose();
}
