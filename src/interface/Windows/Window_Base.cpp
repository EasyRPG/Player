/*Window_Base.cpp, Window_Base routines.
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

#include "Window_Base.h"

void Window_Base::init(int SizeX,int SizeY,int PosX,int PosY,const char *SysIMg)
{
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
	disposing=false;
	visible=true;
}
void Window_Base::dispose()
{
	disposing=true;
	System.dispose();
	tapiz.dispose();
	text.dispose();
	int i,tp;
	tp=(V_Sprite).size();
	for (i = 0; i < tp; i ++)
	{
	(V_Sprite).pop_back();
	}
	tp=(Vtext_Sprite).size();
	for (i = 0; i < tp; i ++)
	{
	(Vtext_Sprite).pop_back();
	}
}

/*void Window_Base::add_text(const char * ctext, int x, int y)
{
	text.x=pos_X+x;
	text.y=pos_Y+y;
	text.set_surface(fuente.drawText(ctext));
	Vtext_Sprite.push_back(text);
}*/

void Window_Base::add_text(std::string ctext, int x, int y)
{
	text.x=pos_X+x;
	text.y=pos_Y+y;

	std::string s_tmp;
    int l = ctext.length();
    int i;

    std::stack<char> c_stack;
    char c_tmp;

    int color = 0;

    SDL_Surface *text_tmp = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, 300, 12, 32, 0, 0, 0, 0);
    SDL_SetColorKey(text_tmp, SDL_SRCCOLORKEY, SDL_MapRGB(text_tmp->format, 0, 0, 0));
    //SDL_Surface *text_tmp;

    for (i = 0; i < l; i++)
    {
        if (c_stack.empty())
        {
            switch (ctext[i])
            {
                case '\\':
                    c_stack.push(ctext[i]);
                    break;

                default: ;
                    //s_tmp.push_back(ctext[i]);
                    fuente.blit_font(text_tmp, ctext[i], 255-color, 255-color, 255-color, 0, i, 0);
            }
        }
        else
        {
            c_tmp = c_stack.top();

            c_stack.push(ctext[i]);
            switch (c_stack.top())
            {
                case ']':
                    if (!isdigit(c_tmp)) goto LABEL;
                    else
                    {
                        color = c_tmp - '0';
                    }
                    break;
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': case '8': case '9':
                case '0':
                    if (c_tmp != '[') goto LABEL;
                    break;

                case '[':
                    if (c_tmp != 'c')
                    {
                        goto LABEL;
                    }
                    break;

                case 'c':
                    if (c_tmp != '\\')
                    {
                        goto LABEL;
                    }

                    break;

                default:
                LABEL:
                    if (c_tmp != '\\') fuente.blit_font(text_tmp, ctext[i], 255-color, 255-color, 255-color, 0, i, 0);
                    while (!c_stack.empty()) c_stack.pop(); // Empty stack
            }
        }
    }
    //text_tmp = fuente.drawText(s_tmp.c_str());

	text.set_surface(text_tmp);
	Vtext_Sprite.push_back(text);
}

void Window_Base::add_sprite(Sprite * the_sprite, int x, int y)
{
	(*the_sprite).x=pos_X+x;
	(*the_sprite).y=pos_Y+y;
	V_Sprite.push_back((int)the_sprite);
}

void Window_Base::draw(SDL_Surface* Screen)
{
	if(visible)
	{
		if(  !disposing  )
		{
			tapiz.draw(Screen);
			unsigned int i;
			for (i = 0; i < (Vtext_Sprite).size(); i ++)
			{
				((Vtext_Sprite).at(i)).draw(Screen);
			}
			Sprite * the_sprite;
			for (i = 0; i < (V_Sprite).size(); i ++)
			{
				the_sprite=(Sprite *)((V_Sprite).at(i));
				(*the_sprite).draw(Screen);
			}
		}
	}
}
