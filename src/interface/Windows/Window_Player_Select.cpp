/*Window_Player_Select.cpp, Window_Player_Select routines.
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

#include "Window_Player_Select.h"

void Window_Player_Select::init(Player_Team * myteam,bool *run,int ComandX,int ComandY,int SizeX,int SizeY,int PosX,int PosY,int curX,int curY, const char *SysIMg)
{
	menu.init(myteam,run,ComandX,ComandY);
	System.init_Sistem();
	System.setimg(SysIMg);
	tapiz.set_surface(System.Exdraw(SizeX,SizeY));
	tapiz.x=PosX;
	tapiz.y=PosY;
	fuente.init_Font();
	Size_of_Block=((Size_X-10)/(ComandX+1))-5*ComandX;
	cursor.set_surface(System.Cube_select(1,curX,curY));
	Moves_cursorY=curY+12;
	cursor.x=PosX+5;
	cursor.y=PosY+5;
	disposing=false;
	pos_X=PosX;
	pos_Y=PosY;
	Size_X=SizeX;
	Size_Y=SizeY;
	Comand_X=ComandX;
	Comand_Y=ComandY;
	Cur_pos_X=5;
	Cur_pos_Y=5;
	visible=false;//delcursor
	visible_window=true;//delaventana
}

void Window_Player_Select::init_curXY(int x,int y)
{
	Cur_pos_X=x;
	Cur_pos_Y=y;
}

void Window_Player_Select::set_curY(int y)
{
	Cur_pos_Y=y;
}
void Window_Player_Select::add_text(std::string ctext,int x,int y)
{
	text.x=pos_X+x;
	text.y=pos_Y+y;

	sha_text.x=pos_X+x+1;
	sha_text.y=pos_Y+y+1;

	unsigned int l = ctext.size();

	std::string s_tmp;

    SDL_Surface *text_tmp = fuente.create_font_surface(FONT_WIDTH*l, 15);
    SDL_Surface *shadow = fuente.create_font_surface(FONT_WIDTH*l, 15);

    SDL_SetColorKey(text_tmp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(text_tmp->format, 0,0,0));
    SDL_SetColorKey(shadow, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(text_tmp->format, 0,0,0));

	unsigned int i;
	for (i = 0; i < l; i++)
	{
        fuente.blit_background(text_tmp, 0, System.get_img(), i);
        fuente.blit_shadow(shadow, System.get_img(), i);
        s_tmp.push_back(ctext[i]);
	}

    fuente.blit_font(text_tmp, &s_tmp, l, 0);
    fuente.blit_font(shadow, &s_tmp, l, 0);
    SDL_SetColorKey(text_tmp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(text_tmp->format, 0,0,0));
    SDL_SetColorKey(shadow, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(text_tmp->format, 0,0,0));

	text.set_surface(text_tmp);
	sha_text.set_surface(shadow);

    Vtext_Sprite.push_back(sha_text);
	Vtext_Sprite.push_back(text);

}

void Window_Player_Select::dispose()
{
	disposing=true;
	int i,tp;
	System.dispose();
	cursor.dispose();
	tapiz.dispose();
	tp=(V_Sprite).size();
	for(i=0;i<tp;i++)
	{
		(V_Sprite).pop_back();
	}
	tp=(Vtext_Sprite).size();
	for(i=0;i<tp;i++)
	{
        Vtext_Sprite[tp-(i+1)].dispose();
		(Vtext_Sprite).pop_back();
	}
	restarmenu();
}

bool  Window_Player_Select::desition()
{
	return(menu.desition());
}

void Window_Player_Select::restarmenu()
{
	menu.restarmenu();
}

int  Window_Player_Select::getindexY()
{
	return(menu.getindexY());
}

int  Window_Player_Select::getindexX()
{
	return(menu.getindexX());
}

void Window_Player_Select::updatekey()
{
	if(visible)
	{
		menu.updatekey();
	}
}

void Window_Player_Select::add_sprite(Sprite*the_sprite,int x,int y)
{
	(*the_sprite).x=pos_X+x;
	(*the_sprite).y=pos_Y+y;
	V_Sprite.push_back((long)the_sprite);
}

void Window_Player_Select::draw(SDL_Surface*Screen)
{
	unsigned int i;
	int offset=0;
	if(visible_window)
	if(!disposing)
	{
		tapiz.draw(Screen);
		int Max_to_show=((Size_Y-20)/fuente.size);//losquecaben
		if(Max_to_show>Comand_Y)
		{
			Max_to_show=Comand_Y;
		}
		if(getindexY()>Max_to_show)
		{
			offset=getindexY()-Max_to_show;
			System.draw(Screen,25,(pos_X+Size_X/2-8),(pos_Y+3));//flechas
			System.draw(Screen,26,(pos_X+Size_X/2),(pos_Y+3));//flechas
		}
		else
		{
			offset=0;
		}

		if((Comand_Y!=getindexY())&&(Max_to_show<Comand_Y))
		{
			System.draw(Screen,45,(pos_X+Size_X/2-8),(pos_Y+Size_Y-11));//flechas
			System.draw(Screen,46,(pos_X+Size_X/2),(pos_Y+Size_Y-11));
		}

		if(visible)
		{
			cursor.y=pos_Y+(getindexY()-offset)*Moves_cursorY+Cur_pos_Y;
			cursor.x=pos_X+(getindexX())*Size_of_Block+10*getindexX()+Cur_pos_X;
			cursor.draw(Screen);
		}

		for(i=0;i<(Vtext_Sprite).size();i++)//textoadicional
		{
			((Vtext_Sprite).at(i)).draw(Screen);
		}

		Sprite*the_sprite;//dibujodeimagenes
		for(i=0;i<(V_Sprite).size();i++)
		{
			the_sprite=(Sprite*)((V_Sprite).at(i));
			(*the_sprite).draw(Screen);
		}

	}
}
