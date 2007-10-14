/* window.cpp, basic window box class
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
 *	@brief		Method definitions for the Window class
 *
 *****************************************************************************/


/*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "window.h"
/*****************************************************************************/


/******************************************************************************
 *	The destructor.
 *
 *	Free the window bitmap.
 *
 *****************************************************************************/
Window::~Window()
{
	if (m_window != NULL) {
		delete m_window;
	}
}

/******************************************************************************
 *	Construct the window bitmap.
 *
 *	Create the window bitmap from the window set. This method
 *	is only called when the Window is created or when it is
 *	changed. Probably inefficient at the moment.
 *
 *****************************************************************************/
void Window::construct_window()
{
	// Free the old window
	if (m_window != NULL) {
			delete m_window;
	}
	// Create an empty bitmap of width * height
	m_window = new Bitmap(m_width, m_height);

	Rect dest, src;

	// Draw the background

	src.x = src.y = 0;
	src.w = src.h = 32;
	if (m_width < 32)
		m_width = 32;
	if (m_height < 32)
		m_height = 32;

	int i, j;
	for (j = 0; j <= m_height; j += 32) {
		for (i = 0; i <= m_width; i += 32) {
			dest.x = i;
			dest.y = j;
			m_bitmap->blit(*m_window, dest, src);
		}
	}


}

/******************************************************************************
 *	Draw the window.
 *
 *	Just copy m_window to display surface.
 *
 *****************************************************************************/

void Window::draw()
{
	Rect dest;
	dest.x = m_x;
	dest.y = m_y;
	m_window->blit(dest);
}
