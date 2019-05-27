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

#ifndef EP_COLOR_H
#define EP_COLOR_H

#include <cstdint>
#include <tuple>

/**
 * Color class.
 */
class Color {
public:
	/**
	 * Constructor. All components are set to 0, but alpha
	 * that is set to 255.
	 */
	Color();

	/**
	 * Constructor.
	 *
	 * @param red red component.
	 * @param green green component.
	 * @param blue blue component.
	 * @param alpha alpha component.
	 */
	Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

	/**
	 * Sets all color properties.
	 *
	 * @param red red component.
	 * @param green green component.
	 * @param blue blue component.
	 * @param alpha alpha component.
	 */
	void Set(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

	/** Red component. */
	uint8_t red;

	/** Green component. */
	uint8_t green;

	/** Blue component. */
	uint8_t blue;

	/** Alpha component. */
	uint8_t alpha;
};

/**
 * Create a Color object from RPG_RT flash values [0,31]
 *
 * @param r red color
 * @param g green color
 * @param b blue color
 * @param current_level strength of flash
 */
inline Color MakeFlashColor(int r, int g, int b, double current_level) {
	return Color(r * 8, g * 8, b * 8, current_level * 8);
}

inline bool operator==(const Color &l, const Color& r) {
	return l.red == r.red
		   && l.green == r.green
		   && l.blue == r.blue
		   && l.alpha == r.alpha;
}

inline bool operator!=(const Color &l, const Color& r) {
	return !(l == r);
}

inline bool operator<(const Color &l, const Color& r) {
	return
		std::tie(l.red, l.green, l.blue, l.alpha) <
		std::tie(r.red, r.green, r.blue, r.alpha);
}

#endif
