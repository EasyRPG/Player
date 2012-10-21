/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FONT_RENDER_8X8_H_
#define _FONT_RENDER_8X8_H_

///////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////
#include <string>
#include "rect.h"
#include "system.h"

///////////////////////////////////////////////////////////
/// FontRender8x8 namespace.
///////////////////////////////////////////////////////////
namespace FontRender8x8 {
	///////////////////////////////////////////////////////
	/// Draw text to pixels.
	/// @param text : Text to draw
	/// @param pixels : Pixels where text is drawn on
	/// @param x : x coord where drawing starts
	/// @param y : y coord where drawing starts
	/// @param surface_width : Width of surface the pixels are belonging to
	/// @param surface_height : Height of surface the pixels are belonging to
	/// @param bpp : Bits per Pixel of surface the pixels are belonging to
	/// @param color : Color to use
	///////////////////////////////////////////////////////
	void TextDraw(const std::string& text, uint8_t* pixels, int x, int y, int surface_width, int surface_height, int bpp, uint32_t color);

	///////////////////////////////////////////////////////
	/// Draw text to pixels in a rect.
	/// @param text : Text to draw
	/// @param pixels : Pixels where text is drawn on
	/// @param dst_rect : Target rect
	/// @param width : Width of surface the pixels are belonging to
	/// @param height : Height of surface the pixels are belonging to
	/// @param bpp : Bits per Pixel of surface the pixels are belonging to
	/// @param color : Color to use
	///////////////////////////////////////////////////////
	void TextDraw(const std::string& text, uint8_t* pixels, Rect dst_rect, int surface_width, int surface_height, int bpp, uint32_t color);
}

#endif
