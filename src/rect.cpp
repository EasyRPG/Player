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
Rect::Rect() :
	x(0),
	y(0),
	width(0),
	height(0) {
}

Rect::Rect(Sint16 x, Sint16 y, Uint16 width, Uint16 height) :
	x(x),
	y(y),
	width(width),
	height(height) {
}

////////////////////////////////////////////////////////////
bool Rect::operator==(const Rect &other) const {
	return	x == other.x && y == other.y &&
			width == other.width && height == other.height;
}

////////////////////////////////////////////////////////////
bool Rect::operator!=(const Rect &other) const {
	return	x != other.x || y != other.y ||
			width != other.width || height != other.height;
}

////////////////////////////////////////////////////////////
void Rect::Set(Sint16 new_x, Sint16 new_y, Uint16 new_width, Uint16 new_height) {
	x = new_x;
	y = new_y;
	width = new_width;
	height = new_height;
}

////////////////////////////////////////////////////////////
void Rect::Adjust(Uint16 max_width, Uint16 max_height) {
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x < max_width && y < max_height) {
		if (max_width < x + width) width = max_width - x;
		if (max_height < y + height) height = max_height - y;
	}
}

////////////////////////////////////////////////////////////
bool Rect::IsOutOfBounds(Uint16 max_width, Uint16 max_height) const {
	if (width <= 0 || height <= 0) return true;
	if (x >= max_width || y >= max_height) return true;
	if (x + width <= 0 || y + height <= 0) return true;
	return false;
}
