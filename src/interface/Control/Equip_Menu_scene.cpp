/*Equip_Menu_scene.cpp, Equip_Menu_Scene routines.
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

#include "Equip_Menu_scene.h"
/*Equip_Menu_Scene::~Euip_Menu_Scene()
{
}
Equip_Menu_Scene:: Euip_Menu_Scene()
{
}*/
void Equip_Menu_Scene::init(General_data * TheTeam)
{
	myteam=TheTeam;

	myaudio=&(TheTeam->musica);
	running=&TheTeam->running;
	NScene=&TheTeam->TheScene;

	int j;
	int item_tipe2;
    std::string system_string;
    system_string.append("System/");
    system_string.append(myteam->ldbdata->System_dat.System_graphic);
    system_string.append(".png");

	menu.init(TheTeam, running, 0,4, 190, 105, 130, 30,(char *)system_string.c_str());
	Armas.init(TheTeam, running, 1,0, 320, 105, 0, 135,(char *)system_string.c_str());
	descripcion.init(320,30,0,0,(char *)system_string.c_str());
	stats.init(130,105,0,30,(char *)system_string.c_str());
	int i=(*myteam).select;
	int space=16,Size_of_Block=150;
	char stringBuffer[255];

	str_Vector.push_back(( (*((*myteam).Players.get_Weapon(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Shield(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Armor(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Helmet(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Accessory(i))).get_name()));
	menu.set_posx_text(90);

	menu.add_text("Arma",10,5);
	menu.add_text("Escudo",10,5+(1*space));
	menu.add_text("Armadura",10,5+(2*space));
	menu.add_text("Casco",10,5+(3*space));
	menu.add_text("Otros",10,5+(4*space));
	menu.setComands(& str_Vector);

  	int item_tipe= *myteam->Players.get_Weapon(i)->get_type();

	for(j=0;j<(*myteam).get_num_items();j++)
	{
		item_tipe2= (*((*myteam).get_type(j)));
		if(item_tipe==item_tipe2)
		{
			str_Vector2.push_back( (const char *) ((*myteam).get_item_name(j)) );
			sprintf(stringBuffer, "%d ", (*(*myteam).get_NOI(j)));
			Armas.add_text(stringBuffer,Size_of_Block-10+((Size_of_Block+10)*((j)%(2))),5+((j/2)*space));
		}
	}

	while(str_Vector2.size()<2)//para que no truene
	str_Vector2.push_back( " " );
	Armas.setComands(& str_Vector2);
	Armas.on_use=false;
	space=17;

	stats.add_text(((*myteam).Players.get_name(i)),10,5);

	sprintf(stringBuffer, "Fuerza  %d ", ((*myteam).Players.get_Attack(i)));
	stats.add_text(stringBuffer,10,5+(1*space));
	sprintf(stringBuffer, "Defenza %d ", ((*myteam).Players.get_Defense(i)));
	stats.add_text(stringBuffer,10,5+(2*space));
	sprintf(stringBuffer, "Intelecto  %d ", ((*myteam).Players.get_Spirit(i)));
	stats.add_text(stringBuffer,10,5+(3*space));
	sprintf(stringBuffer, "Agilidad%d ", ((*myteam).Players.get_Speed(i)));
	stats.add_text(stringBuffer,10,5+(4*space));
	retardo=0;

}

void Equip_Menu_Scene::update_1_menu()
{
	int i=(*myteam).select;
	int space=16;
	//int Size_of_Block=150;
	menu.dispose();
	menu.init(myteam, therun, 0,4, 190, 105, 130, 30,"System/System.png");

	str_Vector.push_back(( (*((*myteam).Players.get_Weapon(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Shield(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Armor(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Helmet(i))).get_name()));
	str_Vector.push_back(( (*((*myteam).Players.get_Accessory(i))).get_name()));
	menu.set_posx_text(90);

	menu.add_text("Arma",10,5);
	menu.add_text("Escudo",10,5+(1*space));
	menu.add_text("Armadura",10,5+(2*space));
	menu.add_text("Casco",10,5+(3*space));
	menu.add_text("Otros",10,5+(4*space));
	menu.setComands(& str_Vector);
}

void Equip_Menu_Scene::update_2_menu()
{
	int k=0,i=menu.getindexY();
	char stringBuffer[255];
	int space=16,Size_of_Block=150;
	Armas.dispose();

	Armas.init(myteam, therun, 1,0, 320, 105, 0, 135,"System/System.png");
	int j=(*myteam).select;
	unsigned char item_tipe2,item_tipe=4;

	if (i==0)
	{
		item_tipe= (*((*myteam).Players.get_Weapon_type(j)));
	}
	if (i==1)
	{
		item_tipe= (*((*myteam).Players.get_Shield_type(j)));
	}
	if (i==2)
	{
		item_tipe= (*((*myteam).Players.get_Armor_type(j)));
	}
	if (i==3)
	{
		item_tipe= (*((*myteam).Players.get_Helmet_type(j)));
	}
	if (i==4)
	{
		item_tipe= (*((*myteam).Players.get_Accessory_type(j)));
	}
	for(j=0;j<(*myteam).get_num_items();j++)
	{
		item_tipe2= (*((*myteam).get_type(j)));
		if(item_tipe==item_tipe2)
		{
			str_Vector2.push_back( (const char *) ((*myteam).get_item_name(j)) );
			sprintf(stringBuffer, "%d ", (*(*myteam).get_NOI(j)));
			k=str_Vector2.size()-1;
			Armas.add_text(stringBuffer,Size_of_Block-10+((Size_of_Block+10)*((k)%(2))),5+((k/2)*space));
		}
	}
	while(str_Vector2.size()<2)
	{
		str_Vector2.push_back( " " );
	}
	Armas.setComands(& str_Vector2);
	Armas.on_use=false;
}

void Equip_Menu_Scene::update(SDL_Surface* Screen)
{
	int static lastcusor=0;
	retardo++;
	if(retardo==6)
	{
		menu.draw(Screen);
		if(lastcusor !=menu.getindexY())
		{
			update_2_menu();lastcusor=menu.getindexY();
		}
		Armas.draw(Screen);
		stats.draw(Screen);
		descripcion.draw(Screen);
		retardo=0;
		 myteam->screen_got_refresh=true;
	}
}

void Equip_Menu_Scene::action()
{
	Armas.on_use=true;
}


void Equip_Menu_Scene::action2()
{
	int i=(*myteam).select;
	std::string strd;
	const char * cadena;
	int k=0,j;
	if(((*((*myteam).Players.get_Weapon(i))).id)!=0)// si el objeto del heroe no es vacio
	{
		if(menu.getindexY()==0)
		{
			(*myteam).add_item((*((*myteam).Players.get_Weapon(i))));
		}
		if(menu.getindexY()==1)
		{
			(*myteam).add_item((*((*myteam).Players.get_Shield(i))));
		}
		if(menu.getindexY()==2)
		{
			(*myteam).add_item((*((*myteam).Players.get_Armor(i))));
		}
		if(menu.getindexY()==3)
		{
			(*myteam).add_item((*((*myteam).Players.get_Helmet(i))));
		}
		if(menu.getindexY()==4)
		{
			(*myteam).add_item((*((*myteam).Players.get_Accessory(i))));
		}
	}//agregalo a la lista de objetos

	k=Armas.getindexX()+2*Armas.getindexY();//k igal al indice elegido
	strd = str_Vector2.at(k);

	for(j=0;j<(*myteam).get_num_items();j++)
	{
		cadena= (((*myteam).get_item_name(j)));
		if(strd.compare(cadena)==0)//si son iguales
		{
		break;//j es elindice real
		}
	}//si no lo encuetra pones el nulo

	if(j==(*myteam).get_num_items())
	{
		Item nulo;
		nulo.set_name(" ");
		nulo.set_NOI(0);
		nulo.set_type(0);
		nulo.id=0;
		if(menu.getindexY()==0)
		{
			(*myteam).Players.set_Weapon(i ,nulo);
		}
		if(menu.getindexY()==1)
		{
			(*myteam).Players.set_Shield(i ,nulo);
		}
		if(menu.getindexY()==2)
		{
			(*myteam).Players.set_Armor(i ,nulo);
		}
		if(menu.getindexY()==3)
		{
			(*myteam).Players.set_Helmet(i ,nulo);
		}
		if(menu.getindexY()==4)
		{
			(*myteam).Players.set_Accessory(i ,nulo);
		}
	}
	else
	{
		if(menu.getindexY()==0)
		{
			(*myteam).Players.set_Weapon(i ,(*myteam).get_item(j)  );
		}
		if(menu.getindexY()==1)
		{
			(*myteam).Players.set_Shield(i ,(*myteam).get_item(j)  );
		}
		if(menu.getindexY()==2)
		{
			(*myteam).Players.set_Armor(i ,(*myteam).get_item(j)  );
		}
		if(menu.getindexY()==3)
		{
			(*myteam).Players.set_Helmet(i ,(*myteam).get_item(j)  );
		}
		if(menu.getindexY()==4)
		{
			(*myteam).Players.set_Accessory(i ,(*myteam).get_item(j)  );
		}
		(*(*myteam).get_NOI(j))=(*(*myteam).get_NOI(j))-1;
		if((*(*myteam).get_NOI(j))==0)
		{
			(*myteam).erase_item(j);
		}
	}

	Armas.restarmenu();
	menu.restarmenu();
	Armas.on_use=false;
	update_1_menu();
	update_2_menu();

}

void Equip_Menu_Scene::updatekey()
{
	if(Armas.on_use)
	{
		Armas.updatekey();
		if(Armas.desition())
		{
		action2();
		}
	}
	else
	{
		menu.updatekey();
		if(menu.desition())
		{
		action();
		}
	}


         if(menu.menu.cancel)
        {
                    *NScene = 4;
        }
}

void Equip_Menu_Scene::dispose()
{
	menu.dispose();
	Armas.dispose();
	descripcion.dispose();
	stats.dispose();
}
