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

    int color = 0,color_R = 214,color_G = 255,color_B = 255, lost_space=0;

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
                   lost_space++;
                    c_stack.push(ctext[i]);
                    break;

                default:
                    //s_tmp.push_back(ctext[i]);
                    fuente.blit_font(text_tmp, ctext[i], color_R, color_G, color_B, 0, (i-lost_space), 0);
            }
        }
        else
        {
            c_tmp = c_stack.top();

            c_stack.push(ctext[i]);
            switch (c_stack.top())
            {
                case ']':
                lost_space++;
                    if (!isdigit(c_tmp)) goto LABEL;
                    else
                    {
                        color = c_tmp - '0';

                        switch (color)
                        {

                            case 0:
                                color_R= 165;
                                color_G= 211;
                                color_B= 255;
                            break;
                            case 1:
                                color_R= 82;
                                color_G= 121;
                                color_B= 206;
                            break;
                            case 2:
                                color_R= 247;
                                color_G= 186;
                                color_B= 132;
                            break;
                            case 3:
                                color_R= 123;
                                color_G= 125;
                                color_B= 132;
                            break;
                            case 4:
                                color_R= 247;
                                color_G= 223;
                                color_B= 90;
                            break;
                            case 5:
                                color_R= 206;
                                color_G= 142;
                                color_B= 140;
                            break;
                            case 6:
                                color_R= 189;
                                color_G= 170;
                                color_B= 247;
                            break;
                            case 7:
                                color_R= 231;
                                color_G= 158;
                                color_B= 231;
                            break;
                            case 8:
                                color_R= 255;
                                color_G= 195;
                                color_B= 107;
                            break;
                            case 9:
                                color_R= 165;
                                color_G= 235;
                                color_B= 123;
                            break;
                            case 10:
                                color_R= 99;
                                color_G= 166;
                                color_B= 247;
                            break;
                            case 11:
                                color_R= 239;
                                color_G= 166;
                                color_B= 165;
                            break;
                            case 12:
                                color_R= 206;
                                color_G= 235;
                                color_B= 99;
                            break;
                            case 13:
                                color_R= 214;
                                color_G= 146;
                                color_B= 247;
                            break;
                            case 14:
                                color_R= 255;
                                color_G= 182;
                                color_B= 49;
                            break;
                            case 15:
                                color_R= 148;
                                color_G= 231;
                                color_B= 181;
                            break;
                            case 16:
                                color_R= 156;
                                color_G= 125;
                                color_B= 181;
                            break;
                            case 17:
                                color_R= 90;
                                color_G= 134;
                                color_B= 165;
                            break;
                            case 18:
                                color_R= 66;
                                color_G= 158;
                                color_B= 107;
                            break;
                            case 19:
                                color_R= 181;
                                color_G= 125;
                                color_B= 66;
                            break;



                        }
                    }
                    break;
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': case '8': case '9':
                case '0':
                lost_space++;
                    if (c_tmp != '[') goto LABEL;
                    break;

                case '[':
                lost_space++;
                    if (c_tmp != 'c')
                    {
                        goto LABEL;
                    }
                    break;

                case 'c':
                lost_space++;
                    if (c_tmp != '\\')
                    {
                        goto LABEL;
                    }

                    break;

                default:
                LABEL:
                    if (c_tmp != '\\') fuente.blit_font(text_tmp, ctext[i], color_R, color_G, color_B, 0, (i-lost_space), 0);
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
