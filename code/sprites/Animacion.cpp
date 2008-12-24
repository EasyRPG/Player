#include "Animacion.h"



void Animacion::init_Anim(int the_cols,int the_rows)
{

	x = 0;
	y = 0;
	frame = 0;
	cols=the_cols;
	rows=the_rows;
	maxframe=10;
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

