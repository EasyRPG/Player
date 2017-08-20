#ifndef EP_BITMAPFONT_H
#define EP_BITMAPFONT_H

#include <stdint.h>

struct BitmapFontGlyph {
	uint16_t code;
	bool is_full;
	uint16_t data[12];
};

extern BitmapFontGlyph const SHINONOME_GOTHIC[11071];
extern BitmapFontGlyph const SHINONOME_MINCHO[488];
extern BitmapFontGlyph const BITMAPFONT_WQY[20941];
extern BitmapFontGlyph const BITMAPFONT_RMG2000[470];
extern BitmapFontGlyph const BITMAPFONT_TTYP0[3074];

#endif // EP_BITMAPFONT_H
