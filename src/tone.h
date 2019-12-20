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

#ifndef EP_TONE_H
#define EP_TONE_H

#include <tuple>
#include <ostream>
#include <algorithm>

/**
 * Tone class.
 */
class Tone {
public:
	/**
	 * Constructor. All values are set to 128.
	 */
	constexpr Tone() = default;

	/**
	 * Constructor.
	 *
	 * @param red red component.
	 * @param green green component.
	 * @param blue blue component.
	 * @param gray gray component.
	 */
	constexpr Tone(int red, int green, int blue, int gray);

	/** Red component. */
	int red = 128;

	/** Green component. */
	int green = 128;

	/** Blue component. */
	int blue = 128;

	/** Gray component. */
	int gray = 128;
};

inline Tone Blend(const Tone& l, const Tone& r) {
	return Tone((l.red * r.red) / 128,
			(l.green * r.green) / 128,
			(l.blue * r.blue) / 128,
			(l.gray * r.gray) / 128);
}

inline bool operator==(const Tone &l, const Tone& r) {
	return l.red == r.red
		&& l.green == r.green
		&& l.blue == r.blue
		&& l.gray == r.gray;
}

inline bool operator!=(const Tone &l, const Tone& r) {
	return !(l == r);
}

inline bool operator<(const Tone &l, const Tone& r) {
	return
		std::tie(l.red, l.green, l.blue, l.gray) <
		std::tie(r.red, r.green, r.blue, r.gray);
}

inline std::ostream& operator<<(std::ostream& os, const Tone& t) {
	os << "Tone{" << t.red << ", " << t.green << ", " << t.blue << ", " << t.gray << "}";
	return os;
}

constexpr Tone::Tone(int red, int green, int blue, int gray) :
	red(std::min(255, std::max(0, red))),
	green(std::min(255, std::max(0, green))),
	blue(std::min(255, std::max(0, blue))),
	gray(std::min(255, std::max(0, gray))) {
}

#endif
