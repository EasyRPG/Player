#include "Faceset.h"



void Faceset::init_Faceset(int posx,int posy,int theframe)
{
	x = posx;
	y = posy;
	frame = theframe;
	cols=4;//redefinir
	rows=4;//redefinir
	int w = 48;
	int h = 48;
	SDL_Surface * Eximg;
	Eximg = SDL_CreateRGBSurface(SDL_SWSURFACE,w, h, 16,0, 0,0, 0);
	SDL_Rect fuente = {(frame%cols)* w,(frame/cols) * h, w, h};
	SDL_Rect rect = {0,0, 0, 0};
	SDL_BlitSurface (img, & fuente,	Eximg, &rect);
	dispose();
	set_surface(Eximg);

}

void Faceset::drawf (SDL_Surface * screen)
{
	int w = getw();
	int h = geth();

	SDL_Rect fuente = {(frame%cols)* w,(frame/cols) * h, w, h};
	SDL_Rect rect = {x,y, 0, 0};
	SDL_BlitSurface (img, & fuente,	screen, &rect);
}

