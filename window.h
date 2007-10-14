/* window.h, basic window box class
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
 *	@brief		A class to represent message windows as well as
 *				menus and any other windows.
 *
 *****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

/*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "bitmap.h"
/*****************************************************************************/


/******************************************************************************
 *	Window class.
 *
 *	Handles the creation and rendering of windows.
 *
 *****************************************************************************/
class Window
{
public:
// Constructor & Destructor
	Window(std::string filename, Sint32 x, Sint32 y, Sint32 width, Sint32 height)
		: m_window(NULL)
		, m_bitmap(new Bitmap(filename))
		, m_filename(filename)
		, m_x(x), m_y(y)
		, m_width(width), m_height(height)
		, m_visible(true)
	{
		construct_window();
	}
	~Window();
// Accessors
	std::string filename() { return m_filename; }	///< Get filename
	/// Set filename
	void filename(std::string nfilename)
	{
		if (m_filename != nfilename) {
			m_filename = nfilename;
			// Reload the bitmap
			if (m_bitmap != NULL)
				delete m_bitmap;
			m_bitmap = new Bitmap(m_filename);
			construct_window();
		}
	}
	Sint32 x() { return m_x; }			///< Get x coordinate
	void x(Sint32 nx) { m_x = nx; }			///< Set x coordinate
	Sint32 y() { return m_y; }			///< Get y coordinate
	void y(Sint32 ny) { m_y = ny; }			///< Set y coordinate
	Sint32 width() { return m_width; }		///< Get width
	/// Set width
	void width(Sint32 nwidth)
	{
		if (m_width != nwidth) {
			m_width = nwidth;
			construct_window();
		}
	}
	Sint32 height() { return m_height; }		///< Get height
	/// Set height
	void height(Sint32 nheight)
	{
		if (m_height != nheight) {
			m_height = nheight;
			construct_window();
		}
	}
// Methods
	void draw();
private:
// Private Methods
	void construct_window();
// Member Variables
	Bitmap* m_window;		///< Window is drawn on this
	Bitmap* m_bitmap;		///< System set image
	std::string m_filename;		///< Filename of the window skin
	Sint32 m_x, m_y;		///< Display coordinates
	Sint32 m_width, m_height;	///< Display dimensions
	bool m_visible;			///< Is the window visible?
};

#endif
