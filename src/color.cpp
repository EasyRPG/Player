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
#include "color.h"

////////////////////////////////////////////////////////////
Color::Color() :
	red(0),
	green(0),
	blue(0),
	alpha(255) {
}

Color::Color(uint red, uint green, uint blue, uint alpha) :
	red((uint8)red),
	green((uint8)green),
	blue((uint8)blue),
	alpha((uint8)alpha) {
}

Color::Color(int red, int green, int blue, int alpha) :
	red((uint8)red),
	green((uint8)green),
	blue((uint8)blue),
	alpha((uint8)alpha) {
}

////////////////////////////////////////////////////////////
bool Color::operator==(const Color &other) const {
	return red == other.red && green == other.green && blue == other.blue && alpha == other.alpha;
}

////////////////////////////////////////////////////////////
bool Color::operator!=(const Color &other) const {
	return red != other.red || green != other.green || blue != other.blue || alpha != other.alpha;
}

////////////////////////////////////////////////////////////
void Color::Set(uint red, uint green, uint blue, uint alpha) {
	red = (uint8)red;
	green = (uint8)green;
	blue = (uint8)blue;
	alpha = (uint8)alpha;
}
