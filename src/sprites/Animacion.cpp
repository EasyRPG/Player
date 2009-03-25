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
    red=0;
    green=0;
    blue=0;
	x = 0;
	y = 0;
	frame = 0;
	alpha=255;
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

            SDL_Surface * temp = NULL;
            SDL_Surface * temp3 = CreateSurface(w,h);
           SDL_Surface * temp2;
           temp2=SDL_ConvertSurface(temp3, img->format,NULL);
            SDL_FreeSurface(temp3);

            SDL_Rect fuente = {(frame%cols)* w,(frame/cols) * h, w, h};
            SDL_BlitSurface (img, & fuente,	temp2, NULL);

    SDL_Color * color;
    int maxColors = temp2->format->palette->ncolors;
    for (int i = 1; i < maxColors; i ++)
    {
        color = & temp2->format->palette->colors[i];

	color->r+=red;
	color->g+=green;
	color->b+=blue;
    }



            float scaleFactor;
            scaleFactor=(float)zoom/100;
            if(img!=NULL)
            {
            temp=rotozoomSurface(temp2, angle, scaleFactor, 1);
            }
            SDL_FreeSurface(temp2);

            temp2=SDL_ConvertSurface(temp, screen->format,NULL);

            SDL_SetAlpha(temp2, SDL_SRCALPHA,alpha);

            if(temp!=NULL)
            {
            SDL_Rect rect = {(x-(temp2->w/2)), (y-(temp2->h/2)), 0, 0};
		    SDL_BlitSurface(temp2, NULL,	screen, &rect);
            SDL_FreeSurface(temp);
            }
            SDL_FreeSurface(temp2);

	}

}

