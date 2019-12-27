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

#ifndef EP_OPACITY_H
#define EP_OPACITY_H

#include <cassert>

/** Opacity class.  */
struct Opacity {
	static constexpr Opacity Opaque() { return Opacity(); }

	int top = 255;
	int bottom = 255;
	int split = 0;

	constexpr Opacity() = default;

	constexpr Opacity(int opacity) :
		Opacity(opacity, opacity, 0) {}

	constexpr Opacity(int top_opacity, int bottom_opacity, int split) :
		top(top_opacity), bottom(bottom_opacity), split(split) {}

	int Value() const {
		assert(!IsSplit());
		return top;
	}

	constexpr bool IsSplit() const {
		return split > 0 && top != bottom;
	}

	constexpr bool IsTransparent() const {
		return IsSplit() ? top <= 0 && bottom <= 0 : top <= 0;
	}

	constexpr bool IsOpaque() const {
		return IsSplit() ? top >= 255 && bottom >= 255 : top >= 255;
	}

	private:
};

#endif
