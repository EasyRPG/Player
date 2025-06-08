#ifndef EP_BITMAPFONT_H
#define EP_BITMAPFONT_H

#include "system.h"

#include "bitmapfont_glyph.h"

// Always enabled (otherwise breaks font rendering completely)
#include "generated/shinonome_gothic.h"
#include "generated/shinonome_mincho.h"
#include "generated/bitmapfont_rmg2000.h"
#include "generated/bitmapfont_ttyp0.h"

#ifdef WANT_FONT_BAEKMUK
#include "generated/bitmapfont_baekmuk.h"
#else
constexpr const std::array<BitmapFontGlyph,0> BITMAPFONT_BAEKMUK;
#endif

#ifdef WANT_FONT_WQY
#include "generated/bitmapfont_wqy.h"
#else
constexpr const std::array<BitmapFontGlyph,0> BITMAPFONT_WQY;
#endif

#endif // EP_BITMAPFONT_H
