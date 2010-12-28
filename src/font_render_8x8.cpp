/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////
#include "font_render_8x8.h"
#include <cstring>

///////////////////////////////////////////////////////////
const uint8* GetPearl8x8FontData();

///////////////////////////////////////////////////////////
void DrawChr(uint8 chr, uint8* pixels, int x, int y, int w, int bpp, uint32 color) {
	static const uint8* font = GetPearl8x8FontData();

	const uint8* pchr = font + chr * 8;

	if (bpp == 1) {
		uint8* dst_pixels = pixels + x + y * w;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (pchr[i] & (0x80 >> j)) dst_pixels[0] = (uint8)color;
				dst_pixels += 1;
			}
			dst_pixels += w - 8;
		}
	} else if (bpp == 2) {
		uint16* dst_pixels = (uint16*)pixels + x + y * w;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (pchr[i] & (0x80 >> j)) dst_pixels[0] = (uint16)color;
				dst_pixels += 1;
			}
			dst_pixels += w - 8;
		}
	} else if (bpp == 3) {
		uint8* dst_pixels = pixels + x + y * w;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (pchr[i] & (0x80 >> j)) memcpy(dst_pixels, &color, 3);
				dst_pixels += 3;
			}
			dst_pixels += w - 8;
		}
	} else if (bpp == 4) {
		uint32* dst_pixels = (uint32*)pixels + x + y * w;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (pchr[i] & (0x80 >> j)) dst_pixels[0] = color;
				dst_pixels += 1;
			}
			dst_pixels += w - 8;
		}
	}
}

///////////////////////////////////////////////////////////
void FontRender8x8::TextDraw(const std::string& text, uint8* pixels, int x, int y, int surface_width, int surface_height, int bpp, uint32 color) {
	if (x + 8 > surface_width || y + 8 > surface_height || text.empty()) return;

	int dst_x = x;
	int dst_y = y;

	// Invert the text color if the first background pixel matches the text color
	if (bpp == 1) {
		uint8* dst_pixels = pixels + x + y * surface_width;
		if (dst_pixels[0] == (uint8)color) {
			color = color ^ 0xFFFFFFFF;
		}
	} else if (bpp == 2) {
		uint16* dst_pixels = (uint16*)pixels + x + y * surface_width;
		if (dst_pixels[0] == (uint16)color) {
			color = color ^ 0xFFFFFFFF;
		}
	} else if (bpp == 4) {
		uint32* dst_pixels = (uint32*)pixels + x + y * surface_width;
		if (dst_pixels[0] == color) {
			color = color ^ 0xFFFFFFFF;
		}
	}

	for (size_t i = 0; i < text.size(); i++) {
		switch (text[i]) {
			case '\n':
				dst_x = x;
				dst_y += 10;
				if (dst_y > surface_height) return;
				break;
			case '\r':
				dst_x = x;
				break;
			case '\t':
				dst_x += 8 * 4;
				break;
			case '\0':
				return;
			default:
				DrawChr((uint8)text[i], pixels, dst_x, dst_y, surface_width, bpp, color);
				dst_x += 8;
		}

		if (dst_x + 8 >= surface_width) {
			dst_x = x;
			dst_y += 10;
			if (dst_y > surface_height) return;
		}
	}
}

///////////////////////////////////////////////////////////
void FontRender8x8::TextDraw(const std::string& text, uint8* pixels, Rect dst_rect, int surface_width, int surface_height, int bpp, uint32 color) {
	int x = dst_rect.x;
	int y = dst_rect.y;
	int w = dst_rect.width;
	int h = dst_rect.height;
	
	if (x + w > surface_width || y + h > surface_height || text.empty()) return;

	int dst_x = x;
	int dst_y = y;

	for (size_t i = 0; i < text.size(); i++) {
		switch (text[i]) {
			case '\n':
				dst_x = x;
				dst_y += 8;
				if (dst_y > h) return;
				break;
			case '\r':
				dst_x = x;
				break;
			case '\t':
				dst_x += 8 * 4;
				break;
			case '\0':
				return;
			default:
				DrawChr((uint8)text[i], pixels, dst_x, dst_y, surface_width, bpp, color);
				dst_x += 8;
		}

		if (dst_x + 8 >= w) {
			dst_x = x;
			dst_y += 8;
			if (dst_y > h) return;
		}
	}
}
