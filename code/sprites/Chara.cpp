#include "Chara.h"

void 

Chara::init_Chara()
{
	x = 12;
	y = 12;
	frame = 2;
	dir = 0;
	cols=3;
	rows=4;
	animation[0][0] = 1;
	animation[0][1] = 0;
	animation[0][2] = 1;
	animation[0][3] = 2;
	animation[1][0] = 4;
	animation[1][1] = 3;
	animation[1][2] = 4;
	animation[1][3] = 5;
	animation[2][0] = 7;
	animation[2][1] = 6;
	animation[2][2] = 7;
	animation[2][3] = 8;
	animation[3][0] = 10;
	animation[3][1] = 9;
	animation[3][2] = 10;
	animation[3][3] = 11;
}

void Chara::frameupdate()
{
	static long delay=0;
        delay++;
        if(delay==10)
        {
	        frame= (frame +1)%4;
	        delay=0;
        }
}

void Chara::frame_ori()
{
        if(frame!=0)
        	frame= 0;
}

void Chara::drawc (SDL_Surface * screen)
{    
	int realframe;
	int w = 24;//getw();
	int h =32;//geth();
	realframe=animation [dir][frame];
	SDL_Rect fuente = {(realframe%cols)* w,(realframe/cols) * h, w, h};
	SDL_Rect rect = {x, y, 0, 0};
	SDL_BlitSurface (img, & fuente,	screen, &rect);
}
