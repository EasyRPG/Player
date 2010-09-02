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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "rect.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Rect::Rect() {
	x = 0;
	y = 0;
	width = 0;
	height = 0;
}
Rect::Rect(int ix, int iy, int iwidth, int iheight) {
	x = ix;
	y = iy;
	width = iwidth;
	height = iheight;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Rect::~Rect() {
}

////////////////////////////////////////////////////////////
/// != operator
////////////////////////////////////////////////////////////
bool Rect::operator!=(const Rect &other) const {
	return x != other.x || y != other.y || width != other.width || height != other.height;
}

////////////////////////////////////////////////////////////
/// Set rect values
////////////////////////////////////////////////////////////
void Rect::Set(int nx, int ny, int nwidth, int nheight) {
	x = nx;
	y = ny;
	width = nwidth;
	height = nheight;
}

////////////////////////////////////////////////////////////
/// Get rect
////////////////////////////////////////////////////////////
SDL_Rect Rect::Get() {
	SDL_Rect rect = {x, y, width, height};
	return rect;
}

////////////////////////////////////////////////////////////
/// Adjust Rect
////////////////////////////////////////////////////////////
void Rect::Adjust(int awidth, int aheight) {
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x < awidth && y < aheight) {
		if (awidth < x + width) width = awidth - x;
		if (aheight < y + height) height = aheight - y;
	}
}

////////////////////////////////////////////////////////////
/// Adjust Rect
////////////////////////////////////////////////////////////
bool Rect::IsOutOfBounds(int awidth, int aheight) {
	if (width <= 0 || height <= 0) return true;
	if (x >= awidth || y >= aheight) return true;
	if (x + width <= 0 || y + height <= 0) return true;
	return false;
}
