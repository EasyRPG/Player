
#include "sprite.h"
#define TRUE 1
#define FALSE 0


void Sprite::setimg(const char* string)
{ 
	visible=true;
	not_clean =true;
	img = IMG_Load (string);
}

void Sprite::set_surface(SDL_Surface * imag)
{    
	visible=true;
	not_clean =true;
	img=imag;
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
