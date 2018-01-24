/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include "color.h"

Color::Color() :
	red(0),
	green(0),
	blue(0),
	alpha(0) {
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
	red(red),
	green(green),
	blue(blue),
	alpha(alpha) {
}


bool Color::operator==(const Color &other) const {
	return red == other.red && green == other.green && blue == other.blue && alpha == other.alpha;
}

bool Color::operator!=(const Color &other) const {
	return red != other.red || green != other.green || blue != other.blue || alpha != other.alpha;
}

void Color::Set(uint8_t nred, uint8_t ngreen, uint8_t nblue, uint8_t nalpha) {
	red = nred;
	green = ngreen;
	blue = nblue;
	alpha = nalpha;
}
