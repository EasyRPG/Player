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
#include "system.h"

///////////////////////////////////////////////////////////
/// FontRender8x8 namespace.
///////////////////////////////////////////////////////////
namespace FontRender8x8 {
	///////////////////////////////////////////////////////
	/// Draw text to pixels.
	/// @param text : 
	/// @param pixels : 
	/// @param x : 
	/// @param y : 
	/// @param w : 
	/// @param h : 
	/// @param bpp : 
	/// @param pitch : 
	///////////////////////////////////////////////////////
	void TextDraw(const std::string text, uint8* pixels, int x, int y, int w, int h, int bpp, uint32 color);
}

#endif
