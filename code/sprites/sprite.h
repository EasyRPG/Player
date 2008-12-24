#ifndef SPRITE_H_
#define SPRITE_H_


// *****************************************************************************
// =============================================================================



#include <SDL.h>

#include "../tools/SDL_rotozoom.h"

#include "../tools/tools.h"

#include <SDL_image.h>

#include <stdio.h>

#include <stdlib.h>
//
#define pi2 6.28318


// =============================================================================
// *****************************************************************************




/*
Sprite es la clase en la que se basan una gran parte del apartado gráfico y animaciones
dentro del player, es heredada por Chara, Faceset, Sistem y Animacion.
*/

class Sprite {

	protected:

	    SDL_Surface * img;
	    int cols,rows;
	    bool not_clean;

	public:

	    bool visible;
	    int frame;
	    int x,y;
	    void setx(int posx) {x=posx;}
	    void sety(int posy) {y=posy;}
	    void setimg(const char* string);
	    void setcols(int icols) {cols=icols;}
	    void setrows(int irows) {rows=irows;}
	    void addx(int c) {x+=c;}
	    void addy(int c) {y+=c;}
	    void set_surface(SDL_Surface * imag);
	    int getx() {return x;}
	    int gety() {return y;}
	    int getw() {return img->w/cols;}
	    int geth() {return img->h/rows;}
	    int getcols() {return cols;}
	    int getrows() {return rows;}
	    void draw(SDL_Surface *screen);
	    void dispose();
	    int colision(Sprite sp);

};


#endif
