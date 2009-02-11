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

SDL_Surface* Font::create_font_surface(int w, int h)
{
    SDL_Surface* dummy = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, w, h, 32, 0, 0, 0, SDL_ALPHA_OPAQUE);
	//SDL_SetAlpha(dummy, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_TRANSPARENT);

	return dummy;
}

SDL_Surface* Font::drawText(char* string)
{
	SDL_Color foregroundColor = { fR, fG, fB, fU};
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
	return(textSurface);
}

SDL_Surface* Font::drawText(const char* string)
{
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

void Font::blit_font(SDL_Surface *dst, std::string *s_tmp, int x, int y)
{
    int y1 = x;
    y1 = y;
	SDL_Color foregroundColor = { 255, 255, 255, SDL_ALPHA_OPAQUE};
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, s_tmp->c_str(), foregroundColor);

	SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, textSurface->w, 15, 32, 0,0,0,SDL_ALPHA_OPAQUE);

    SDL_BlitSurface(textSurface, NULL, tmp, NULL);
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(tmp->format, 255, 255, 255));
	SDL_BlitSurface(tmp, NULL, dst, NULL);

	SDL_FreeSurface(textSurface);
	SDL_FreeSurface(tmp);
}

void Font::blit_background(SDL_Surface *dst, int n, SDL_Surface *back, int x)
{
    SDL_Rect clip_system = { 16*(n%10), 48+16*(n/10), 6, 16 };
    SDL_Rect pos_x = { x*6, 0, 10, 16 };

    SDL_BlitSurface(back, &clip_system, dst, &pos_x);
}

Font::~Font()
{
    TTF_CloseFont(font);
}

void Font::Quit()//esto es asi porque no se me ocurre aun algo mejor
{
	TTF_Quit();
}
