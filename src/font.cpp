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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <map>

#include <boost/next_prior.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include "reader_util.h"
#include "shinonome.hxx"

#include "filefinder.h"
#include "output.h"
#include "font.h"
#include "bitmap.h"
#include "utils.h"
#include "wcwidth.h"

////////////////////////////////////////////////////////////
/// Static Variables
////////////////////////////////////////////////////////////
namespace {
	struct ShinonomeFont : public Font {
		enum { HEIGHT = 12, FULL_WIDTH = HEIGHT, HALF_WIDTH = FULL_WIDTH / 2, };

		ShinonomeFont(uint16_t const (&ascii)[0x100][HEIGHT],
					  uint16_t const (&font)[94][94][HEIGHT]);

		Rect GetSize(std::string const& txt) const;

		void Render(Bitmap& bmp, int x, int y, Bitmap const& sys, int color, unsigned glyph);
		void Render(Bitmap& bmp, int x, int y, Color const& color, unsigned glyph);

	private:
		uint16_t const (&ascii_)[0x100][HEIGHT];
		uint16_t const (&font_)[94][94][HEIGHT];

		uint16_t const* get_font(unsigned glyph, bool& full) const;
	}; // class ShinonomeFont

	FontRef const gothic = EASYRPG_MAKE_SHARED<ShinonomeFont>(SHINONOME_HANKAKU, SHINONOME_GOTHIC);
	FontRef const mincho = EASYRPG_MAKE_SHARED<ShinonomeFont>(SHINONOME_HANKAKU, SHINONOME_MINCHO);

} // anonymous namespace

ShinonomeFont::ShinonomeFont(uint16_t const (&ascii)[0x100][HEIGHT],
							 uint16_t const (&font)[94][94][HEIGHT])
	: Font("Shinonome", HEIGHT, false, false)
	, ascii_(ascii), font_(font) {}

Rect ShinonomeFont::GetSize(std::string const& txt) const {
	std::string const sjis_txt = ReaderUtil::Recode(txt, "UTF-8", "Shift_JIS");
	int c = 0;
	for(std::string::size_type pos = 0;
		(pos = txt.find("$$", pos)) != std::string::npos;
		pos += 2) { ++c; }
	return Rect(0, 0, (sjis_txt.size() - c) * HALF_WIDTH, HEIGHT);
}

uint16_t const* ShinonomeFont::get_font(unsigned const glyph, bool& full) const {
    typedef boost::u32_to_u8_iterator<unsigned const*> iterator;
	std::string const txt = ReaderUtil::Recode(std::string(iterator(&glyph),
														   iterator(&glyph + 1)),
											   "UTF-8", "Shift_JIS");

	unsigned const f = txt[0], s = txt.size() == 2? txt[1] : 0;

	full = txt.size() == 2;
	return txt.size() == 1
		? ascii_[f]
		: font_[(0x81 <= f && f < 0xA0? f - 0x80:
				 0xE0 <= f && f < 0xF0? f - 0xE0 + 0x20:
				 0x00
				 ) + (0x9f <= s && s < 0xFD? 1 : 0)
				][0x40 <= s && s < 0x7F? s - 0x40:
				  0x80 <= s && s < 0x9F? s - 0x80 + 0x3F:
				  0x9F <= s && s < 0xFD? s - 0x9F:
				  0x00]
		;
}

void ShinonomeFont::Render(Bitmap& bmp, int const x, int const y, Bitmap const& sys, int color, unsigned glyph) {
	if(color != ColorShadow) {
		Render(bmp, x + 1, y + 1, sys, ColorShadow, glyph);
	}

	bool full = true;
	uint16_t const* fnt_bmp = get_font(glyph, full);

	size_t const width = full? FULL_WIDTH : HALF_WIDTH;

	unsigned const
		src_x = color == ColorShadow? 16 : color % 10 * 16 + (16 - width) / 2,
	    src_y = color == ColorShadow? 32 : color / 10 * 16 + 48 + (16 - HEIGHT) / 2;

	for(size_t y_ = 0; y_ < HEIGHT; ++y_) {
		for(size_t x_ = 0; x_ < width; ++x_) {
			if(fnt_bmp[y_] & (0x1 << x_)) {
				bmp.SetPixel(x + x_, y + y_, sys.GetPixel(src_x + x_, src_y + y_));
			}
		}
	}
}

void ShinonomeFont::Render(Bitmap& bmp, int x, int y, Color const& color, unsigned glyph) {
	bool full = true;
	uint16_t const* fnt_bmp = get_font(glyph, full);

	size_t const width = full? FULL_WIDTH : HALF_WIDTH;

	for(size_t y_ = 0; y_ < HEIGHT; ++y_) {
		for(size_t x_ = 0; x_ < width; ++x_) {
			if(fnt_bmp[y] & (0x1 << x_)) {
				bmp.SetPixel(x + x_, y + y_, color);
			}
		}
	}
}

FontRef Font::Default(bool const m) {
	return m? mincho : gothic;
}

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Font::Font(const std::string& name, int size, bool bold, bool italic)
	: name(name)
	, size(size)
	, bold(bold)
	, italic(italic)
{
}
