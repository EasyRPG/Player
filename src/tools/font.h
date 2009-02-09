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


class Font
{
	private:
		int fR,  fG,  fB,  fU; // fU = unused
		const char * Fname;
		TTF_Font *font;
	public:

        static void init_TTF();

		int size;
		void init_Font();

		~Font();

		SDL_Surface* drawText(char* string);
		SDL_Surface* drawText(char* string,int r, int b,int g, int u);
		SDL_Surface* drawText(const char* string);
		void blit_font(SDL_Surface *dst, const char src, int r, int g, int b, int u, int x, int y);
		static void Quit();
};

#endif
