/*Batle_scene.cpp, Batle_scene routines.
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

#include"Batle_scene.h"
/*
Batle_scene::~Batle_scene()
{
}
Batle_scene::Batle_scene()
{
}
*/

voidBatle_scene::init(Audio*theaudio,bool*run,unsignedchar*TheScene,Player_Team*TheTeam)
{

	myteam=TheTeam;
	myaudio=theaudio;
	the_run=run;
	(*myaudio).musicload("../Music/Battle2.mid");
	title.x=0;
	title.y=0;
	title.setimg("../Backdrop/Grass.png");
	Window_text.init(320,80,0,160);
	Window_text.visible=false;
	update_window_stats();
	//menu_os.init(theaudio,run,0,4,96,80,96,160);
	//menu_os.visible=false;
	menu.init(theaudio,run,0,4,96,80,0,160);
	str_Vector.push_back("Atacar");
	str_Vector.push_back("Habilidades");
	str_Vector.push_back("Objetos");
	str_Vector.push_back("Defender");
	str_Vector.push_back("Huir");
	menu.setComands(&str_Vector);
	running=run;
	NScene=TheScene;
	MCcomando;
	inti;
	for(i=0;i<(*myteam).get_size();i++)//tantoscomandoscomojugadores.
	{
		Comands.push_back(comando);
	}
	state=0;
	Nmenu_used=0;
	player_in_turn=0;
	moster_in_turn=0;
	turnosp=0;
	update_window_mosterselect();
	moster_select.visible=false;
}

voidBatle_scene::update_window_stats()
{
	window.init(myaudio,the_run,0,3,224,80,96,160,214,16);
	inti=0;
	charstringBuffer[255];
	for(i=0;i<(*myteam).get_size();i++)
	{
		sprintf(stringBuffer,"Hp%d/%dMp%d",(*(*myteam).get_HP(i)),(*(*myteam).get_MaxHP(i)),(*(*myteam).get_MP(i)));
		window.add_text(stringBuffer,110,5+(i*16));
		window.add_text(((*myteam).get_name(i)),10,5+(i*16));
		if((*(*myteam).get_HP(i))>0)
		{
			window.add_text("Normal",60,5+(i*16));
		}
		else
		{
			window.add_text("Muerto",60,5+(i*16));
		}
	}
}

voidBatle_scene::windowtext_showdamange(booltype,intatak,intataked,intdamange)
{
	charstringBuffer[255];
	sprintf(stringBuffer,"%dHPperdidos",damange);
	Window_text.init(320,80,0,160);

	if(type)//sisonlosplayers
	{
		Window_text.add_text(((*myteam).get_name(atak)),5,5);//nombreheroe
		Window_text.add_text("atacaalenemigo",70,5);
		Window_text.add_text((((*myteam).Enemys.at(ataked)).get_name()),5,25);//nombremoustruo
		Window_text.add_text(stringBuffer,70,25);
	}
	else
	{
		Window_text.add_text((((*myteam).Enemys.at(atak)).get_name()),5,5);//nombremoustruo
		Window_text.add_text("ataca",70,5);
		Window_text.add_text(((*myteam).get_name(ataked)),5,25);//nombreheroe
		Window_text.add_text(stringBuffer,70,25);
	}
}

voidBatle_scene::update_window_mosterselect()
{
	inti,j,k=0;
	j=(*myteam).Enemys.size();

	for(i=0;i<j;i++)//dibujatodoslosmoster
	{
		if((*((*myteam).Enemys.at(i)).get_HP())>0)//cambiarporarreglo
		{
			str_Vector2.push_back(((*myteam).Enemys.at(i)).get_name());
			k++;
		}
	}
	moster_select.init(myaudio,the_run,0,k-1,96,80,0,160);
	moster_select.setComands(&str_Vector2);
}


voidBatle_scene::update(SDL_Surface*Screen)
{
	inti,j;
	SDL_FillRect(Screen,NULL,0x0);//Clearscreen
	j=(*myteam).Enemys.size();
	title.draw(Screen);
	window.draw(Screen);
	menu.draw(Screen);
	moster_select.draw(Screen);
	Window_text.draw(Screen);
	for(i=0;i<j;i++)//dibujatodoslosmoster
	{
		(((*myteam).Enemys.at(i)).Batler).draw(Screen);
	}
	if(state==1)//siletocaalosheroes
	{
		atack(Screen,player_in_turn,Comands.at(player_in_turn).selected_moster);
	}
	if(state==2)//siletocaalosmoustruos
	{
		atacked(moster_in_turn);
	}
}

voidBatle_scene::win()
{
	unsignedinti;
	intk=0;
	for(i=0;i<((*myteam).Enemys).size();i++)
	{
		if((*((*myteam).Enemys.at(i)).get_HP())==0)//cambiarporarreglo
		{
			(((*myteam).Enemys.at(i)).Batler).visible=false;//hazqueyanosevea
			k++;
		}
	}
	if(k==(*myteam).get_size())//sitodoslosenemigosmuetros
	{
		*NScene=1;//salalmapa
	}
}

voidBatle_scene::lose()
{
	inti,k=0;
	for(i=0;i<(*myteam).get_size();i++)
	{
		if((*(*myteam).get_HP(i))==0)
		{
			k++;
		}
	}
	if(k==(*myteam).get_size())//sitodoslosheroesmuetros
	{
		*NScene=3;//gameover
	}
}




voidBatle_scene::atack(SDL_Surface*Screen,intnperso,intenemy)
{
	intdamange;
	while((*(((*myteam).Enemys.at(enemy)).get_HP()))==0)//siestamuertoelelgido
	{
		enemy++;//eligeotro
		enemy=(enemy%((*myteam).Enemys).size());
	}

	(*((*myteam).get_Weapon_Anim(nperso))).x=(((*myteam).Enemys.at(enemy)).Batler).x-((((*myteam).Enemys.at(enemy)).Batler).getw())/2;
	(*((*myteam).get_Weapon_Anim(nperso))).y=(((*myteam).Enemys.at(enemy)).Batler).y-((((*myteam).Enemys.at(enemy)).Batler).geth())/2;
	(*((*myteam).get_Weapon_Anim(nperso))).draw(Screen);

	if((*((*myteam).get_Weapon_Anim(nperso))).endanim)//siterminaleatake
	{
		(*((*myteam).get_Weapon_Anim(nperso))).reset();
		damange=(*((*myteam).get_Attack(nperso)));
		(*(((*myteam).Enemys.at(enemy)).get_HP()))=(*(((*myteam).Enemys.at(enemy)).get_HP()))-damange;
		Window_text.dispose();
		windowtext_showdamange(true,nperso,enemy,damange);
		if((*(((*myteam).Enemys.at(enemy)).get_HP()))<0)
		{
			(*(((*myteam).Enemys.at(enemy)).get_HP()))=0;
		}
		if((turnosp+1)<(*myteam).get_size())
		{
			player_in_turn++;//deveriaserunatabla
			turnosp++;
			win();
		}
		else
		{
			win();
			state=2;
		}//lestocaalosmoustruos

	}
}

voidBatle_scene::atacked(intenemy)
{
	inti,j;
	staticintposxt=title.x,flag=0,timer=0,moves=0;
	staticboolfinish=false;
	if((((*myteam).Enemys.at(enemy)).Batler).visible)//siestavivoelenemigo
	{
		timer++;
		if(timer==4)
		{
			flag++;timer=0;
			if(flag%2)
			{
				title.x=posxt+20;
				j=(*myteam).Enemys.size();
				for(i=0;i<j;i++)
				{
					(((*myteam).Enemys.at(i)).Batler).x=(((*myteam).Enemys.at(i)).Batler).x+20;
				}
			}
			else
			{
				flag=0;
				moves++;
				timer=0;
				title.x=posxt-20;
				j=(*myteam).Enemys.size();
				for(i=0;i<j;i++)
				{
					(((*myteam).Enemys.at(i)).Batler).x=(((*myteam).Enemys.at(i)).Batler).x-20;
				}
			}
		}
		if(moves==10)
		{
			moves=11;
			flag=0;
			timer=10;
			title.x=posxt;/////////////////////////restauradodeposiciones
			j=(*myteam).Enemys.size();
			intdamange;
			///////////////////////////////////////////eleciondeplayer
			intk=(rand()%(*myteam).get_size());//eleccionalazar
			while((*(*myteam).get_HP(k))==0)//siestamuertoelelgido
			{
				k++;//eligeotro
				k=(k%(*myteam).get_size());
			}
			///////////////////////////////////////////////////////////////

			damange=*(((*myteam).Enemys.at(enemy)).get_Attack());//calculodedaño
			(*(*myteam).get_HP(k))=(*(*myteam).get_HP(k))-damange;
			if((*(*myteam).get_HP(k))<0)
			{
				(*(*myteam).get_HP(k))=0;
			}
			//////////////////////////////////////////////////////////////////////////
			lose();
			Window_text.dispose();
			windowtext_showdamange(false,enemy,k,damange);
			finish=true;

		}
		if(finish)
		{
			if(timer==120)
			{
				moves=0;
				flag=0;
				timer=0;
				finish=false;
				j=(*myteam).Enemys.size();
				if(moster_in_turn+1<j)
				{
					moster_in_turn++;
				}
				else
				{
					state=0;
					Give_turn();//letocaaloscomandos
				}
			}
		}
	}
	else//sielenemigoestamuerto
	{

		moves=0;
		flag=0;
		timer=0;
		finish=false;
		j=(*myteam).Enemys.size();
		if(moster_in_turn+1<j)//siaunhaymoustruos
		{
			moster_in_turn++;//queletoqueaotro
		}
		else
		{
			state=0;//reinicimaoslabatalla
			Give_turn();//letocaaloscomandos
		}
	}
}

voidBatle_scene::Give_turn()
{
	moster_select.dispose();
	update_window_mosterselect();
	window.dispose();
	update_window_stats();
	menu.restarmenu();
	menu.visible=true;
	Window_text.visible=false;
	moster_select.visible=false;
	window.visible_window=true;
	Nmenu_used=0;
}

voidBatle_scene::action_mosterselect()
{
	inti,j;
	j=(*myteam).Enemys.size();
	for(i=0;i<j;i++)
	{	
		if(moster_select.getindexY()==i)
		{
		Comands.at(Nmenu_used).selected_moster=i;
		Nmenu_used++;
		moster_select.restarmenu();
		moster_select.visible=false;
		menu.visible=true;
		}
	}
	if(Nmenu_used==(*myteam).get_size())//yatodoseligieron
	{
		state=1;
		menu.visible=false;
		Window_text.visible=true;
		window.visible_window=false;
		player_in_turn=0;//noheroesahusadoningunturno
		moster_in_turn=0;//losmoustruostampohanusad
		turnosp=0;
	}
}

voidBatle_scene::action()
{
	inti;
	//Comands.at(num).des2
	if(menu.visible)
	{
		for(i=0;i<4;i++)
		{
			if(menu.getindexY()==i)
			{
				Comands.at(Nmenu_used).des1=i;
				menu.restarmenu();
				menu.visible=false;
				moster_select.visible=true;
			}
			if(menu.getindexY()==4)
			{
				//state=1;
				*NScene=1;
			}
		}
	}
}

voidBatle_scene::updatekey()
{
	if(Nmenu_used<(*myteam).get_size())//siaunnohanelegidotodos
	{
		if(moster_select.visible)
		{
			moster_select.updatekey();
			if(moster_select.desition())
			{
				action_mosterselect();
			}
		}
		if(menu.visible)
		{
			menu.updatekey();
			if(window.visible!=true)//queseveaquepersoelige
			{
				window.visible=true;
			}		
			window.set_curY((16*Nmenu_used)+5);//posicionadoenelperso
			if(menu.desition())
			{
				action();
			}
		}
	}
}

voidBatle_scene::dispose()
{
	title.dispose();
	window.dispose();
	(*myteam).clear_enemy();
	(*myaudio).stopmusic();
	menu.dispose();
}
