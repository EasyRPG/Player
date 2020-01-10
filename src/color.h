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
#include <ostream>

/**
 * Color class.
 */
class Color {
public:
	/**
	 * Constructor. All components are set to 0.
	 */
	constexpr Color() = default;

	/**
	 * Constructor.
	 *
	 * @param red red component.
	 * @param green green component.
	 * @param blue blue component.
	 * @param alpha alpha component.
	 */
	constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

	/** Red component. */
	uint8_t red = 0;

	/** Green component. */
	uint8_t green = 0;

	/** Blue component. */
	uint8_t blue = 0;

	/** Alpha component. */
	uint8_t alpha = 0;
};

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

inline std::ostream& operator<<(std::ostream& os, const Color& c) {
	os << "Color{"
		<< static_cast<int>(c.red) << ", "
		<< static_cast<int>(c.green) << ", "
		<< static_cast<int>(c.blue) << ", "
		<< static_cast<int>(c.alpha) << "}";
	return os;
}

constexpr Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
	red(red),
	green(green),
	blue(blue),
	alpha(alpha) {
}

#endif
