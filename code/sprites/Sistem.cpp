/*Sistem.cpp, sprite routines.
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

#include "Sistem.h"
#include "SDL_rotozoom.h"


void Sistem::init_Sistem()
{
	cols=20;
	rows=10;
}
void Sistem::draw (SDL_Surface * screen,int iframe,int posx,int posy)
{
	int w = getw();
	int h = geth();
	SDL_Rect fuente = {(iframe%cols)* w,(iframe/cols) * h, w, h};
	SDL_Rect rect = {posx,posy, 0, 0};
	SDL_BlitSurface (img, & fuente,	screen, &rect);
}

SDL_Surface * Sistem::CubeDraw (SDL_Surface * screen,int sizeX,int sizeY)
{

	int w = getw();
	int h = geth();
	int i;
	int cubesX=((sizeX-2*w)/w+1);
	int cubesY=((sizeY-1*h)/h);

	draw (screen,4,0, 0);//primera parte del cuadrado

	for(i=0;i<cubesX;i++)
	{
		draw (screen,5,w*(i+1), 0);//primera parte del cuadrado
	}

	for(i=0;i<cubesY;i++)
	{
		draw (screen,24,0,h*(i+1));
	}

	draw (screen,7,(sizeX-w), 0);//segunda parte del cuadrado

	for(i=0;i<cubesY;i++)
	{
		draw (screen,27,(sizeX-w), h*(i+1));
	}

	draw (screen,64,0, (sizeY-h));//segunda parte del cuadrado

	for(i=0;i<cubesX;i++)
	{
	draw (screen,65,w*(i+1), (sizeY-h));//primera parte del cuadrado
	}

	draw (screen,67,(sizeX-w), (sizeY-h));//segunda parte del cuadrado
	return (screen);
}
SDL_Surface * Sistem::Exdraw (int sizeX,int sizeY)
{

	SDL_Surface * Eximg;
	SDL_Surface * Eximg2;
	double zoomX =((double)sizeX/32);
	double zoomY = (((double)sizeY)/32);
	Eximg =SDL_CreateRGBSurface(SDL_SWSURFACE,32, 32, 16,0, 0,0, 0);// IMG_Load ("../System/system2.PNG");
	SDL_Rect fuente = {0 , 0,32,32};
	SDL_Rect rect = {0,0, 0, 0};
	SDL_BlitSurface (img, &fuente,	Eximg, &rect);
	Eximg2=zoomSurface(Eximg, zoomX,zoomY,0);
	SDL_FreeSurface(Eximg);

	return (CubeDraw ( Eximg2, sizeX, sizeY));
}

SDL_Surface * Sistem::Cube_select(int type,int sizeX,int sizeY)
{

	SDL_Surface * screen;
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, sizeX, sizeY, 16,0, 0,0, 0x000000ff);
	int w = getw();
	int h = geth();
	int i,j;
	int cubesX=((sizeX-2*w)/w+1);
	int cubesY=((sizeY-1*h)/h);

	for(j=1;j<cubesX+1;j++)
	{
		for(i=1;i<cubesY+1;i++)
		{
			draw (screen,(25+(4*type)),w*(j), h*(i));//cuadro
		}
	}

	draw (screen,(4+(4*type)),0, 0);//esquina izq arriva

	for(i=0;i<cubesX;i++)
	{
		draw (screen,(5+(4*type)),w*(i+1), 0);//recta horisontal arriva
	}

	for(i=0;i<cubesY;i++)
	{
		draw (screen,(24+(4*type)),0,h*(i+1));//recta vertical izq
	}

	draw (screen,(7+(4*type)),(sizeX-w), 0);//esquina derecha arriva

	for(i=0;i<cubesY;i++)
	{
		draw (screen,(27+(4*type)),(sizeX-w), h*(i+1));//recta vertical derecha
	}

	draw (screen,(64+(4*type)),0, (sizeY-h));//esquina izquierda abajo

	for(i=0;i<cubesX;i++)
	{
		draw (screen,(65+(4*type)),w*(i+1), (sizeY-h));//recta horisontal abajo
	}

	draw (screen,(67+(4*type)),(sizeX-w), (sizeY-h));//esquina derecha abajo
	return (screen);
}

SDL_Surface * Sistem::ExdrawT (int sizeX,int sizeY,int tipe)
{
	SDL_Surface * Eximg;
	SDL_Surface * Eximg2;
	double zoomX =((double)sizeX/32);
	double zoomY = (((double)sizeY)/32);
	Eximg =SDL_CreateRGBSurface(SDL_SWSURFACE,32, 32, 16,0, 0,0, 0);// IMG_Load ("../System/system2.PNG");
	SDL_Rect fuente = {(32*tipe) , 0,32,32};
	SDL_Rect rect = {0,0, 0, 0};
	SDL_BlitSurface (img, &fuente,	Eximg, &rect);
	Eximg2=zoomSurface (Eximg, zoomX,zoomY,0);
	SDL_FreeSurface(Eximg);
	return (Eximg2);
}
