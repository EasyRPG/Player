/* bitmap.cpp, deals with bitmap loading and drawing
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

/******************************************************************************
 *	@file
 *	@brief		Method definitions for the Bitmap class.
 *
 *****************************************************************************/


/*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "SDL.h"
#include "SDL_image.h"
#include "bitmap.h"
/*****************************************************************************/


/******************************************************************************
 *	Default constructor..
 *
 *	Doesn't create SDL_Surface. Do it yourself using load.
 *
 *
 *****************************************************************************/
Bitmap::Bitmap()
{
	m_alpha = 255;
	m_bmp = NULL;
}

/******************************************************************************
 *	A constructor..
 *
 *	Load bitmap from a file.
 *
 *	@param	filename		[in] name of bitmap file.
 *
 *****************************************************************************/
Bitmap::Bitmap(const std::string& filename)
{
	m_alpha = 255;
	load(filename);
}

/******************************************************************************
 *	A constructor..
 *
 *	Create an empty bitmap.
 *
 *	@param	width			[in] width of bitmap.
 *	@param	height			[in] height of bitmap.
 *
 *****************************************************************************/
Bitmap::Bitmap(Sint32 width, Sint32 height)
{
	m_alpha = 255;
	m_bmp = CreateSurface(width, height);
}

/******************************************************************************
 *	A destructor..
 *
 *	Free the SDL_Surface.
 *
 *****************************************************************************/
Bitmap::~Bitmap()
{
	if (m_bmp != NULL) {
		SDL_FreeSurface(m_bmp);
	}
}

/******************************************************************************
 *	Loads the bitmap..
 *
 *	For now, just calls LoadSurface from tools.h
 *
 *	@param	filename		[in] name of bitmap file.
 *
 *****************************************************************************/
void Bitmap::load(const std::string& filename)
{
	m_bmp = LoadSurface(filename);
	if (m_bmp == NULL) {
		printf("Error: Couldn't load bitmap.\n");
		exit(1);
	}
}

/******************************************************************************
 *	Blit from this Bitmap into another surface..
 *
 *	Currently it uses DrawSurface from tools.h, which doesn't allow
 *	scaling. So for now the width and height of the destination surface
 *	are ignored.
 *
 *	@param	dest_surface		[in] the surface to draw to.
 *	@param	dest			[in] the destination rectangle.
 *	@param	src			[in] the source rectangle.
 *
 *****************************************************************************/
void Bitmap::blit(SDL_Surface* dest_surface, Rect dest, Rect src)
{
	DrawSurface(dest_surface, dest.x, dest.y, m_bmp, src.x, src.y, src.w, src.h);
}

/******************************************************************************
 *	Blit from this Bitmap into another Bitmap.
 *
 *	Just call the SDL_Surface constructor~
 *
 *	@param	dest_bmp		[in] the Bitmap to draw to.
 *	@param	dest			[in] the destination rectangle.
 *	@param	src			[in] the source rectangle.
 *
 *****************************************************************************/
void Bitmap::blit(Bitmap& dest_bmp, Rect dest, Rect src)
{
	blit(dest_bmp.surface(), dest, src);
}

/******************************************************************************
 *	Blit from this Bitmap into the display surface.
 *
 *	Just call the SDL_Surface constructor with the current video
 *	surface (the screen) as the SDL_Surface.
 *
 *	@param	dest			[in] the destination rectangle.
 *	@param	src			[in] the source rectangle.
 *
 *****************************************************************************/
void Bitmap::blit(Rect dest, Rect src)
{
	blit(SDL_GetVideoSurface(), dest, src);
}

/******************************************************************************
 *	Blit ALL of this Bitmap into the display surface.
 *
 *	Just call the SDL_Surface constructor with the current video
 *	surface (the screen) as the SDL_Surface and with src as the entire
 *  	bitmap.
 *
 *	@param	dest			[in] the destination rectangle.
 *	@param	src			[in] the source rectangle.
 *
 *****************************************************************************/
void Bitmap::blit(Rect dest) {
	Rect src;
	src.x = src.y = 0;
	src.w = m_bmp->w;
	src.h = m_bmp->h;
	blit(SDL_GetVideoSurface(), dest, src);
}

/******************************************************************************
 *	Draw text on the bitmap
 *
 *	Just a dummy function for the future, does nothing right now.
 *
 *	@param	x			[in] x coordinate.
 *	@param	y			[in] y coordinate.
 *	@param	text			[in] text to draw.
 *
 *****************************************************************************/
void Bitmap::draw_text(Sint32 x, Sint32 y, const string& text)
{
	// Nothing for now
}

/******************************************************************************
 *	Clear this bitmap to some color.
 *
 *	Would probably call SDL_Fill or whatever...
 *
 *	@param	clear_color		[in] Color to clear to.
 *
 *****************************************************************************/
void Bitmap::clear(Color clear_color)
{
	// Nothing for now
}

