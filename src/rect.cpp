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
#include "rect.h"

void Rect::Adjust(int max_width, int max_height) {
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x < max_width && y < max_height) {
		if (max_width < x + width) width = max_width - x;
		if (max_height < y + height) height = max_height - y;
	}
}

void Rect::Adjust(const Rect& rect) {
	if (x < rect.x) {
		width += x - rect.x;
		x = rect.x;
	}

	if (y < rect.y) {
		height += y - rect.y;
		y = rect.y;
	}

	if (rect.x + rect.width < x + width)
		width = rect.x + rect.width - x;

	if (rect.y + rect.height < y + height)
		height = rect.y + rect.height - y;
}

bool Rect::IsEmpty() const {
	return width <= 0 || height <= 0;
}

bool Rect::IsOutOfBounds(int max_width, int max_height) const {
	if (width <= 0 || height <= 0) return true;
	if (x >= max_width || y >= max_height) return true;
	if (x + width <= 0 || y + height <= 0) return true;
	return false;
}

bool Rect::IsOutOfBounds(const Rect &src_rect) const {
	if (width <= 0 || height <= 0) return true;
	if (x >= src_rect.x + src_rect.width || y >= src_rect.y + src_rect.height) return true;
	if (x + width <= src_rect.x || y + height <= src_rect.y) return true;
	return false;
}

Rect Rect::GetSubRect(const Rect src_rect) const {
	Rect rect = src_rect;

	rect.x += x;
	rect.y += y;

	if (rect.x < x) {
		rect.width -= x - rect.x;
		rect.x = x;
	}

	if (rect.y < y) {
		rect.height -= y - rect.y;
		rect.y = y;
	}

	if (rect.x + rect.width > x + width) {
		rect.width = x + width - rect.x;
	}

	if (rect.y + rect.height > y + height) {
		rect.height = y + height - rect.y;
	}

	return rect;
}

bool Rect::AdjustRectangles(Rect& src, Rect& dst, const Rect& ref) {
	if (src.x < ref.x) {
		int dx = ref.x - src.x;
		src.x += dx;
		dst.x += dx;
		src.width -= dx;
	}

	if (src.y < ref.y) {
		int dy = ref.y - src.y;
		src.y += dy;
		dst.y += dy;
		src.height -= dy;
	}

	if (src.x + src.width > ref.x + ref.width) {
		int dx = (src.x + src.width) - (ref.x + ref.width);
		src.width -= dx;
	}

	if (src.y + src.height > ref.y + ref.height) {
		int dy = (src.y + src.height) - (ref.y + ref.height);
		src.height -= dy;
	}

	dst.width = src.width;
	dst.height = src.height;

	return src.width > 0 && src.height > 0;
}

