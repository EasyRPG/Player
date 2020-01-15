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

#include <tuple>
#include <ostream>

/**
 * Rect.
 */
class Rect {
public:
	/**
	 * Constructor. All values are set to 0.
	 */
	constexpr Rect() = default;

	/**
	 * Constructor.
	 *
	 * @param x initial x.
	 * @param y initial y.
	 * @param width initial width.
	 * @param height initial height.
	 */
	constexpr Rect(int x, int y, int width, int height);

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
	Rect GetSubRect(Rect rect) const;

	/** X coordinate. */
	int x = 0;

	/** Y coordinate. */
	int y = 0;

	/** Width. */
	int width = 0;

	/** Height. */
	int height = 0;

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

inline bool operator==(const Rect &l, const Rect& r) {
	return l.x == r.x
		   && l.y == r.y
		   && l.width == r.width
		   && l.height == r.height;
}

inline bool operator!=(const Rect &l, const Rect& r) {
	return !(l == r);
}

inline bool operator<(const Rect &l, const Rect& r) {
	return
		std::tie(l.x, l.y, l.width, l.height) <
		std::tie(r.x, r.y, r.width, r.height);
}

inline std::ostream& operator<<(std::ostream& os, Rect r) {
	os << "Rect{" << r.x << "," << r.y << "," << r.width << "," << r.height << "}";
	return os;
}

constexpr Rect::Rect(int x, int y, int width, int height)
	: x(x), y(y), width(width), height(height) { }

inline void Rect::Double() {
	x *= 2;
	y *= 2;
	width *= 2;
	height *= 2;
}

inline void Rect::Halve() {
	x /= 2;
	y /= 2;
	width /= 2;
	height /= 2;
}

#endif
