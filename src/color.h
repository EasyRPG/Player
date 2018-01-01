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

// Headers
#include "system.h"

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
	 * Equality operator.
	 */
	bool operator==(const Color &other) const;

	/**
	 * Inequality operator.
	 */
	bool operator!=(const Color &other) const;

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

#endif
