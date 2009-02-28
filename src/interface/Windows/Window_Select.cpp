/*Window_Select.cpp, Window_Select routines.
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

#include "Window_Select.h"

void Window_Select::init(Player_Team * myteam, bool * run,int ComandX,int ComandY,int SizeX,int SizeY,int PosX,int PosY, const char *SysIMg)
{
    menu.init(myteam,run,ComandX,ComandY);
	System.init_Sistem();
	System.setimg(SysIMg);
	tapiz.set_surface(System.Exdraw(SizeX,SizeY));
	tapiz.x=PosX;
	tapiz.y=PosY;
	fuente.init_Font();
	pos_X=PosX;
	pos_Y=PosY;
	Size_X=SizeX;
	Size_Y=SizeY;
	Comand_X=ComandX;
	Comand_Y=ComandY;
	visible=true;
	Size_of_Block=((Size_X-10)/(ComandX+1))-5*ComandX;
	cursor.set_surface(System.Cube_select(1,Size_of_Block,16));
	cursor.x=PosX+5;
	cursor.y=PosY+5;
	disposing=false;
	Init_text_X=10;
	on_use=true;
}

void Window_Select::add_text(std::string ctext, int x, int y)
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

void Window_Select::dispose()
{
	disposing = true;
	int i,tp;
	System.dispose();
	cursor.dispose();
	tapiz.dispose();
	//   text.dispose();

	tp=(* My_vector).size();
	for (i = 0; i < tp; i ++)
	{
		(* My_vector).pop_back();
	}
	tp = command_sprites.size();
	for (i = 0; i < tp; i ++)
	{
		command_sprites.pop_back();
		sha_command_sprites.pop_back(); // They've got the same size
	}
	tp=(Vtext_Sprite).size();
	for (i = 0; i < tp; i ++)
	{
		(Vtext_Sprite).pop_back();
	}
}

bool Window_Select::desition()
{
     return (menu.desition());
}

void Window_Select::restarmenu()
{
	menu.restarmenu();
}

int Window_Select::getindexY()
{
	return (menu.getindexY());
}

int Window_Select::getindexX()
{
	return (menu.getindexX());
}

void Window_Select::updatekey()
{

	if(visible)
	{
		if(on_use)
		{
			menu.updatekey();
		}
	}
}

void Window_Select::set_posx_text(int x)
{
	Init_text_X=x;
}


void Window_Select::setComands(vector <std::string> * str_Vec)
{
	My_vector=str_Vec;
	unsigned int i;
	std::string ctext;
	for (i = 0; i < My_vector->size(); i ++)
	{
		ctext = (*My_vector)[i];

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

		command_sprites.push_back(text);
		sha_command_sprites.push_back(sha_text);
	}
}

void Window_Select::draw(SDL_Surface* Screen)
{
	unsigned int i;
	int j;
	int offset=0;
	if(!disposing)
	if(visible)
	{
		tapiz.draw(Screen);
		int Max_to_show= ((Size_Y-20)/fuente.size);//los que caben
		if(Max_to_show>Comand_Y)
		{
			Max_to_show=Comand_Y;
		}

		if(getindexY()>Max_to_show)
		{
			offset=getindexY()-Max_to_show;
			System.draw(Screen,25, (pos_X+ Size_X/2-8), (pos_Y+3));//flechas
			System.draw(Screen,26, (pos_X+ Size_X/2), (pos_Y+3));//flechas
		}
		else
		{
			offset=0;
		}

		if((Comand_Y!=getindexY())&&(Max_to_show<Comand_Y))
		{

			System.draw(Screen,45, (pos_X+ Size_X/2-8), (pos_Y+Size_Y-11));//flechas
			System.draw(Screen,46, (pos_X+ Size_X/2), (pos_Y+Size_Y-11));
		}

		cursor.y=pos_Y+(getindexY()-offset)*fuente.size+5;
		cursor.x=pos_X+(getindexX())*Size_of_Block+10*getindexX()+5;
		cursor.draw(Screen);
		for (j = offset; j <= ((Max_to_show+offset+1)*(Comand_X+1)-1); j ++)  //comandos
		{
			command_sprites[j].x=pos_X+Init_text_X;
			sha_command_sprites[j].x = command_sprites[j].x+1;
			if(Comand_X!=0)
			{
				command_sprites[j].x= command_sprites[j].x+((Size_of_Block+10)*((j)%(Comand_X+1)));
				sha_command_sprites[j].x = command_sprites[j].x+1;
				command_sprites[j].y=((pos_Y+5)+((j-offset)/(Comand_X+1))*fuente.size);
                sha_command_sprites[j].y = command_sprites[j].y+1;
			}
			else
			{
				command_sprites[j].y=((pos_Y+5)+((j-offset)*fuente.size));
				sha_command_sprites[j].y = command_sprites[j].y+1;
			}
                sha_command_sprites[j].draw(Screen);
				command_sprites[j].draw(Screen);

		}

		for (i = 0; i < Vtext_Sprite.size(); i++) //textoadiconal
		{
			Vtext_Sprite[i].draw(Screen);
		}

	}

}
