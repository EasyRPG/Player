/*Font.cpp, Font routines.
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


#include "font.h"

void Font::init_TTF()
{
    TTF_Init();
}

void Font::init_Font()//esto es asi porque no se me ocurre aun algo mejor
{
	Fname="Font/VL-Gothic-Regular.ttf";
	size=12;
	fR=255;
	fG=255;
	fB=255;
	fU=0; //unused

	font = TTF_OpenFont(Fname, size);
	if (font == NULL)
	{
		std::cerr << "Error: Unable to open file: " << Fname << std::endl;
		exit(1);
	}
}

SDL_Surface* Font::drawText(char* string)
{
	SDL_Color foregroundColor = { fR, fG, fB, fU};
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
	return(textSurface);
}

SDL_Surface* Font::drawText(const char* string)
{
	TTF_Font* font = TTF_OpenFont(Fname, size);
	if (font == NULL)
	{
		std::cerr << "Error: Unable to open file: " << Fname << std::endl;
		exit(1);
	}
	SDL_Color foregroundColor = { fR, fG, fB, fU };
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
	return(textSurface);
}

SDL_Surface* Font::drawText(char* string,int r, int b,int g, int u)
{
	SDL_Color foregroundColor = { r, g, b, u };
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
	return(textSurface);
}

void Font::blit_font(SDL_Surface *dst, const char src, int r, int g, int b, int u, int x, int y)
{
	std::string s_tmp;
	s_tmp.push_back(src);
	SDL_Color foregroundColor = { r, g, b, u };
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, s_tmp.c_str(), foregroundColor);
	SDL_Rect textLocation = { x*(textSurface->w), y, (textSurface->w), textSurface->h };
	//printf("%c", src);

//	SDL_Rect fuente = {0,0, (textSurface->w/2), textSurface->h};
//	SDL_BlitSurface(textSurface, &fuente, dst, &textLocation);
	SDL_BlitSurface(textSurface, NULL, dst, &textLocation);

	SDL_FreeSurface(textSurface);
}

Font::~Font()
{
    TTF_CloseFont(font);
}

void Font::Quit()//esto es asi porque no se me ocurre aun algo mejor
{
	TTF_Quit();
}
