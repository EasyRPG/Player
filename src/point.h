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
template<typename T>
class PointT {
public:
	/**
	 * Constructor. All values are set to default (0).
	 */
	constexpr PointT() = default;

	/**
	 * Constructor.
	 *
	 * @param x initial x.
	 * @param y initial y.
	 */
	constexpr PointT(T x, T y) : x(x), y(y) {}

	/** X coordinate. */
	T x = T();

	/** Y coordinate. */
	T y = T();
};

using Point = PointT<int>;
using PointF = PointT<float>;

template<typename T>
inline bool operator==(const PointT<T> &l, const PointT<T>& r) {
	return l.x == r.x && l.y == r.y;
}

template<typename T>
inline bool operator!=(const PointT<T> &l, const PointT<T>& r) {
	return !(l == r);
}

template<typename T>
inline bool operator<(const PointT<T> &l, const PointT<T>& r) {
	return l.x < r.x || (l.x == r.x && l.y < r.y);
}

template<typename T>
inline bool operator>(const PointT<T> &l, const PointT<T>& r) {
	return l.x > r.x || (l.x == r.x && l.y > r.y);
}

template<typename T>
inline bool operator<=(const PointT<T> &l, const PointT<T>& r) {
	return !(l > r);
}

template<typename T>
inline bool operator>=(const PointT<T> &l, const PointT<T>& r) {
	return !(l < r);
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, PointT<T> r) {
	os << "Point{" << r.x << "," << r.y << "}";
	return os;
}

#endif
