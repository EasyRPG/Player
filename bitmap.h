/* bitmap.h, deals with image loading and drawing
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
 *	@brief		A Bitmap class to load and draw images.
 *
 *****************************************************************************/

#ifndef BITMAP
#define BITMAP

/*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "SDL.h"
#include "tools.h"
/*****************************************************************************/


/******************************************************************************
 *	Rectangle structure.
 *
 *	Like SDL_Rect but with extra stuff~
 *
 *****************************************************************************/
struct Rect
{
	Sint16 x, y;		///< x and y coordinates.
	Uint16 w, h;		///< width and height.
	Rect() : x(0), y(0), w(0), h(0) { }	///< Default constructor (0)
	/// Construct with specific values.
	Rect(Sint16 nx, Sint16 ny, Uint16 nw, Uint16 nh)
		: x(nx), y(ny), w(nw), h(nh) { }
	/// Check if the point is within the rect.
	bool point_inside(Sint16 nx, Sint16 ny)
	{
		return (nx > x && ny > y && nx < x + w && ny < y + h);
	}
};

/******************************************************************************
 *	Bitmap class.
 *
 *	A class that wraps SDL_Surface and allows easily loading
 *	and drawing images.
 *
 *****************************************************************************/
class Bitmap
{
public:
// Constructors & Destructor
	Bitmap();
	Bitmap(const std::string& filename);
	Bitmap(Sint32 width, Sint32 height);
	~Bitmap();
// Accessors
	SDL_Surface* surface() { return m_bmp; }	///< Get the surface (for blitting)
	Sint32 width() { return m_bmp->w; }		///< Get width
	Sint32 height() { return m_bmp->h; }		///< Get height
// Methods
	void load(const std::string& filename);
	void blit(SDL_Surface* dest_surface, Rect dest, Rect src);
	void blit(Bitmap& dest_bmp, Rect dest, Rect src);
	void blit(Rect dest, Rect src);
	void blit(Rect dest);
	void draw_text(Sint32 x, Sint32 y, const string& text);
	void clear(Color clear_color);
private:
// Member Variables
	SDL_Surface* m_bmp;		///< Surface to hold the image
	Uint8 m_alpha;			///< Alpha 0(transparent)..255(opaque)
};

#endif
