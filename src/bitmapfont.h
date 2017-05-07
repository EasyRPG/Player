#ifndef _INC_BITMAPFONT_H_
#define _INC_BITMAPFONT_H_

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

#endif // _INC_BITMAPFONT_H_
