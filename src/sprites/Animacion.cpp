/*Animacion.cpp, sprite routines.
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

#include "Animacion.h"



void Animacion::init_Anim()
{

	x = 0;
	y = 0;
	frame = 0;
	if(img!=NULL)
	{
	cols=img->w/96;
	rows=img->h/96;
	maxframe=cols*rows;
	}
	endanim=false;
	delay=0;

}
void Animacion::frameupdate()
{
        delay++;
        if(delay==10)
        {
		if(frame<maxframe)
		{
			frame++;
		}
		else
		{
			endanim=true;
		}
	delay=0;
	}
}

void Animacion::reset()
{
	delay=0;
	frame=0;
	endanim=false;
}

void Animacion::draw (SDL_Surface * screen)
{

	int w = getw();
	int h = geth();
	frameupdate();
	if(!endanim)
	{
		SDL_Rect fuente = {(frame%cols)* w,(frame/cols) * h, w, h};
		SDL_Rect rect = {x, y, 0, 0};
		SDL_BlitSurface (img, & fuente,	screen, &rect);
	}

}

