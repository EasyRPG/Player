/*sprite.h, EasyRPG player sprite class declaration file.
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


#ifndef SPRITE_H_
#define SPRITE_H_


// *****************************************************************************
// =============================================================================



#include "SDL.h"

#include "SDL_rotozoom.h"

#include "../tools/tools.h"

#include "SDL_image.h"

#include <iostream>
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

        Sprite();
        ~Sprite();
	    bool visible;
	    int frame;
	    int x,y;
	    void setx(int posx);
	    void sety(int posy);
	    void setimg(const std::string& string);
	    void setcols(int icols);
	    void setrows(int irows);
	    void addx(int c);

	    SDL_Surface* get_img() { return img; }

	    void addy(int c);
	    void set_surface(SDL_Surface * imag);
	    int getx();
	    int gety();
	    int getw();
	    int geth();
	    int getcols();
	    int getrows();
	    void draw(SDL_Surface *screen);
	    void dispose();
	    int colision(Sprite sp);

};


#endif
