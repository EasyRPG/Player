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
#include <lcf/data.h>
#include "cache.h"
#include "output.h"
#include "utils.h"
#include "bitmap.h"
#include "font.h"
#include "text.h"
#include "compiler.h"

#include <cctype>
#include <iterator>

Point Text::Draw(Bitmap& dest, int x, int y, Font& font, const Bitmap& system, int color, char32_t ch, bool is_exfont) {
	if (is_exfont) {
		return Font::exfont->Render(dest, x, y, system, color, ch);
	} else {
		return font.Render(dest, x, y, system, color, ch);
	}
}

Point Text::Draw(Bitmap& dest, int x, int y, Font& font, Color color, char32_t ch, bool is_exfont) {
	if (is_exfont) {
		return Font::exfont->Render(dest, x, y, color, ch);
	} else {
		return font.Render(dest, x, y, color, ch);
	}
}

Point Text::Draw(Bitmap& dest, const int x, const int y, Font& font, const Bitmap& system, const int color, StringView text, const Text::Alignment align) {
	if (text.length() == 0) return { 0, 0 };

	Rect dst_rect = font.GetSize(text);

	const int ih = dst_rect.height;

	switch (align) {
	case Text::AlignCenter:
		dst_rect.x = x - dst_rect.width / 2; break;
	case Text::AlignRight:
		dst_rect.x = x - dst_rect.width; break;
	case Text::AlignLeft:
		dst_rect.x = x; break;
	default: assert(false);
	}

	dst_rect.y = y;
	dst_rect.width += 1; dst_rect.height += 1; // Need place for shadow
	if (dst_rect.IsOutOfBounds(dest.GetWidth(), dest.GetHeight())) return { 0, 0 };

	const int iy = dst_rect.y;
	const int ix = dst_rect.x;

	// Where to draw the next glyph (x pos)
	int next_glyph_pos = 0;

	// This loops always renders a single char, color blends it and then puts
	// it onto the text_surface (including the drop shadow)
	auto iter = text.data();
	const auto end = iter + text.size();
	while (iter != end) {
		auto ret = Utils::TextNext(iter, end, 0);

		iter = ret.next;
		if (EP_UNLIKELY(!ret)) {
			continue;
		}
		next_glyph_pos += Text::Draw(dest, ix + next_glyph_pos, iy, font, system, color, ret.ch, ret.is_exfont).x;
	}
	return { next_glyph_pos, ih };
}

Point Text::Draw(Bitmap& dest, const int x, const int y, Font& font, const Color color, StringView text) {
	if (text.length() == 0) return { 0, 0 };

	int dx = x;
	int mx = x;

	int dy = y;
	int ny = 0;

	auto iter = text.data();
	const auto end = iter + text.size();
	while (iter != end) {
		auto ret = Utils::UTF8Next(iter, end);

		iter = ret.next;
		if (EP_UNLIKELY(!ret)) {
			continue;
		}

		if (ret.ch == U'\n') {
			if (ny == 0) {
				ny = font.GetSize(ret.ch).height;
			}
			dy += ny;
			mx = std::max(mx, dx);
			dx = x;
			ny = 0;
			continue;
		}

		auto rect = font.Render(dest, dx, dy, color, ret.ch);
		dx += rect.x;
		assert(ny == 0 || ny == rect.y);
		ny = rect.y;
	}
	dy += ny;
	mx = std::max(mx, dx);

	return { mx - x , dy - y };
}
