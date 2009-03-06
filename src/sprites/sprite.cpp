/*sprite.cpp, sprite routines.
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

#include "sprite.h"
#define TRUE 1
#define FALSE 0

void Sprite::setx(int posx)
{
	x=posx;
}

void Sprite::sety(int posy)
{
	y=posy;
}

void Sprite::setcols(int icols)
{
	cols=icols;
}

void Sprite::setrows(int irows)
{
	rows=irows;
}

void Sprite::addx(int c)
{
	x+=c;
}

void Sprite::addy(int c)
{
	y+=c;
}

int Sprite::getx()
{
	return x;
}

int Sprite::gety()
{
	return y;
}

int Sprite::getw()
{
	return img->w/cols;
}

int Sprite::geth()
{
	return img->h/rows;
}

int Sprite::getcols()
{
	return cols;
}

int Sprite::getrows()
{
	return rows;
}

void Sprite::setimg(const std::string& string)
{
    SDL_Color color;
    Uint32 colorKey;

	visible=true;
	not_clean =true;
	img = IMG_Load (string.c_str());

	if (img == NULL)
	{
	std::cerr << "Error: Unable to open file: " << string <<  std::endl;
	exit(1);
	}

	if ((img->format->BitsPerPixel)!=8)
	{
	std::cerr << "Error img have more than 8 bits of color: " << string <<  std::endl;
	exit(1);
	}
	SetTransparent(img);
    color = img->format->palette->colors[0];
    colorKey = SDL_MapRGB(img->format, color.r, color.g, color.b);
    SDL_SetColorKey(img, SDL_SRCCOLORKEY, colorKey);

}

void Sprite::set_surface(SDL_Surface * imag)
{
	visible=true;
	not_clean =true;
	img=imag;
}

Sprite::Sprite()
{
not_clean=false;
}
Sprite::~Sprite()
{
//dispose();
}

void Sprite::SetAlpha(int alpha)
{
SDL_SetAlpha(img, SDL_SRCALPHA,alpha);
}

void Sprite::ModRGB(int red,int green,int blue)
{

    if ((img->format->BitsPerPixel)==8)
	{
    SDL_Color * color;
    int maxColors = img->format->palette->ncolors;
    for (int i = 1; i < maxColors; i ++)
    {
        color = & img->format->palette->colors[i];

	color->r+=red;
	color->g+=green;
	color->b+=blue;

            if (color->r > 255)
                color->r= 255;
	    if (color->g > 255)
                color->g= 255;
            if (color->b > 255)
                color->b= 255;

            if (color->r < 0)
                color->r= 0;
	    if (color->g < 0)
                color->g= 0;
            if (color->b < 0)
                color->b= 0;

    }
	SetTransparent(img);
	}else
	{

        SDL_Color color;
        for(int x = 0; x < img->w; x++)
        for(int y = 0; y < img->h; y++)
        {
            //here the negative color is calculated!
     //       color.r = 255 - img->format->[x][y].r;
     //       color.g = 255 - img[x][y].g;
     //       color.b = 255 - img[x][y].b;
             //(x, y, color);
        }


    }
}


void Sprite::dispose()
{
        if(not_clean)
        {
		SDL_FreeSurface(img);
        not_clean =false;
        }
}

void Sprite::draw (SDL_Surface * screen)
{
	if(visible)
	{
	SDL_Rect rect = {x, y, 0, 0};
	SDL_BlitSurface (img, NULL,	screen, &rect);
	}
}

int Sprite::colision(Sprite sp) {

	int w1,h1,w2,h2,x1,y1,x2,y2;
	w1=getw();			// ancho del sprite1
	h1=geth();			// altura del sprite1
	w2=sp.getw();			// ancho del sprite2
	h2=sp.geth();			// alto del sprite2
	x1=getx();			// pos. X del sprite1
	y1=gety();			// pos. Y del sprite1
	x2=sp.getx();			// pos. X del sprite2
	y2=sp.gety();			// pos. Y del sprite2

	if (((x1+w1)>x2)&&((y1+h1)>y2)&&((x2+w2)>x1)&&((y2+h2)>y1)) {
		return TRUE;
	} else {
		return FALSE;
	}
}
