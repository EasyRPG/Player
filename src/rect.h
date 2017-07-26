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

#ifndef EP_RECT_H
#define EP_RECT_H

#include "system.h"

/**
 * Rect.
 */
class Rect {
public:
	/**
	 * Constructor. All values are set to 0.
	 */
	Rect();

	/**
	 * Constructor.
	 *
	 * @param x initial x.
	 * @param y initial y.
	 * @param width initial width.
	 * @param height initial height.
	 */
	Rect(int x, int y, int width, int height);

	/**
	 * Equality operator.
	 */
	bool operator==(Rect const& other) const;

	/**
	 * Inequality operator.
	 */
	bool operator!=(Rect const& other) const;

	/**
	 * Sets all rect values simultaneously.
	 *
	 * @param x new x.
	 * @param y new y.
	 * @param width new width.
	 * @param height new height.
	 */
	void Set(int x, int y, int width, int height);

	/**
	 * Adjusts rect bounds, trimming it so it is contained
	 * inside the given size.
	 *
	 * @param width max width.
	 * @param height max height.
	 */
	void Adjust(int width, int height);

	/**
	 * Adjusts rect bounds, trimming it so it is contained
	 * inside the given rectangle.
	 *
	 * @param rect bounding rectangle.
	 */
	void Adjust(const Rect& rect);

	/**
	 * Checks if rect is empty.
	 *
	 * @return whether the rect is empty.
	 */
	bool IsEmpty() const;

	/**
	 * Gets if rect is totally outside the given size.
	 *
	 * @param width max width.
	 * @param height max height.
	 * @return whether the rect is outside of the scope.
	 */
	bool IsOutOfBounds(int width, int height) const;

	/**
	 * Gets if rect is totally outside the given rect.
	 *
	 * @param rect rect.
	 * @return whether the rect is outside of the scope.
	 */
	bool IsOutOfBounds(Rect const& rect) const;

	/**
	 * Gets a sub rect from a given rect.
	 *
	 * @param rect rect.
	 */
	Rect GetSubRect(Rect const& rect);

	/** X coordinate. */
	int x;

	/** Y coordinate. */
	int y;

	/** Width. */
	int width;

	/** Height. */
	int height;

	/**
	 * Adjusts a source/destination pair of rectangles.
	 * src is adjusted to fit within ref.
	 * dst will have the same width and height as src.
	 * Any adjustments made to src are also made to dst.
	 *
	 * @param src rect.
	 * @param dst rect.
	 * @param ref rect.
	 * @return true if the rectangles are non-empty.
	 */
	static bool AdjustRectangles(Rect& src, Rect& dst, const Rect& ref);

	/**
	 * Doubles the coordinates of a rectangle.
	 */
	void Double();

	/**
	 * Halves the coordinates of a rectangle.
	 */
	void Halve();
};

#endif
