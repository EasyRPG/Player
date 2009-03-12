/*Font.h, EasyRPG player Font class declaration file.
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

#ifndef FONT_H_
#define FONT_H_

#include "SDL.h"
#include "SDL_ttf.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define FONT_WIDTH 6

class Font
{
	private:
		int fR,  fG,  fB,  fU; // fU = unused
		const char * Fname;
		TTF_Font *font_ptr;
	public:

        static void init_TTF();

		int size;
		void init_Font();

		~Font();

		SDL_Surface* create_font_surface(int w, int h);

		SDL_Surface* drawText(char* string);
		SDL_Surface* drawText(char* string,int r, int b,int g, int u);
		SDL_Surface* drawText(const char* string);
		void blit_font(SDL_Surface *dst, std::string *s, int x, int y);

		void blit_background(SDL_Surface *dst, int n, SDL_Surface *back, int x);
        void blit_shadow(SDL_Surface *dst, SDL_Surface *back, int x);

		static void Quit();
};
static Font fuente;

#endif
