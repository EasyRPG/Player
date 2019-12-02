#ifndef EP_BITMAPFONT_GLYPH_H
#define EP_BITMAPFONT_GLYPH_H

#include <stdint.h>

struct BitmapFontGlyph {
	uint16_t code;
	bool is_full;
	uint16_t data[12];
};

constexpr bool operator<(const BitmapFontGlyph& lhs, uint32_t const code) {
	return lhs.code < code;
}

constexpr bool operator<(const BitmapFontGlyph& lhs, const BitmapFontGlyph& rhs) {
	return lhs.code < rhs.code;
}

constexpr const BitmapFontGlyph BITMAPFONT_REPLACEMENT_GLYPH = { 65533, true, { 96, 240, 504, 924, 1902, 3967, 4031, 1982, 1020, 440, 240, 96 } };

#endif
