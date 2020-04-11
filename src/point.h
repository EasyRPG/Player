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

#ifndef EP_POINT_H
#define EP_POINT_H

#include <ostream>

/**
 * Point.
 */
class Point {
public:
	/**
	 * Constructor. All values are set to 0.
	 */
	constexpr Point() = default;

	/**
	 * Constructor.
	 *
	 * @param x initial x.
	 * @param y initial y.
	 */
	constexpr Point(int x, int y) : x(x), y(y) {}

	/** X coordinate. */
	int x = 0;

	/** Y coordinate. */
	int y = 0;
};

inline bool operator==(const Point &l, const Point& r) {
	return l.x == r.x && l.y == r.y;
}

inline bool operator!=(const Point &l, const Point& r) {
	return !(l == r);
}

inline bool operator<(const Point &l, const Point& r) {
	return l.x < r.x || (l.x == r.x && l.y < r.y);
}

inline bool operator>(const Point &l, const Point& r) {
	return l.x > r.x || (l.x == r.x && l.y > r.y);
}

inline bool operator<=(const Point &l, const Point& r) {
	return !(l > r);
}

inline bool operator>=(const Point &l, const Point& r) {
	return !(l < r);
}

inline std::ostream& operator<<(std::ostream& os, Point r) {
	os << "Point{" << r.x << "," << r.y << "}";
	return os;
}

#endif
