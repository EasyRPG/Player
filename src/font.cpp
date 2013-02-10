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

bool operator<(ShinonomeGlyph const& lhs, uint32_t const code) {
	return lhs.code < code;
}

////////////////////////////////////////////////////////////
/// Static Variables
////////////////////////////////////////////////////////////
namespace {
	ShinonomeGlyph const* find_glyph(ShinonomeGlyph const* data, size_t size, uint32_t code) {
		ShinonomeGlyph const* ret = std::lower_bound(data, data + size, code);
		return ret != (data + size)? ret : NULL;
	}

	ShinonomeGlyph const* find_gothic_glyph(uint32_t code) {
		return find_glyph(SHINONOME_GOTHIC,
						  sizeof(SHINONOME_GOTHIC) / sizeof(ShinonomeGlyph), code);
	}

	ShinonomeGlyph const* find_mincho_glyph(uint32_t code) {
		ShinonomeGlyph const* const mincho =
			find_glyph(SHINONOME_MINCHO,
					   sizeof(SHINONOME_MINCHO) / sizeof(ShinonomeGlyph), code);
		return mincho == NULL? find_gothic_glyph(code) : mincho;
	}

	struct ShinonomeFont : public Font {
		enum { HEIGHT = 12, FULL_WIDTH = HEIGHT, HALF_WIDTH = FULL_WIDTH / 2, };

		typedef ShinonomeGlyph const*(*function_type)(uint32_t);

		ShinonomeFont(function_type func);

		Rect GetSize(std::string const& txt) const;

		void Render(Bitmap& bmp, int x, int y, Bitmap const& sys, int color, unsigned glyph);
		void Render(Bitmap& bmp, int x, int y, Color const& color, unsigned glyph);

	private:
		function_type const func_;
	}; // class ShinonomeFont

	FontRef const gothic = EASYRPG_MAKE_SHARED<ShinonomeFont>(&find_gothic_glyph);
	FontRef const mincho = EASYRPG_MAKE_SHARED<ShinonomeFont>(&find_mincho_glyph);

} // anonymous namespace

ShinonomeFont::ShinonomeFont(ShinonomeFont::function_type func)
	: Font("Shinonome", HEIGHT, false, false), func_(func) {}

Rect ShinonomeFont::GetSize(std::string const& txt) const {
	typedef boost::u8_to_u32_iterator<std::string::const_iterator> iterator;
	size_t units = 0;
	iterator i(txt.begin(), txt.begin(), txt.end());
	iterator const end(txt.end(), txt.begin(), txt.end());
	for(; i != end; ++i) {
		ShinonomeGlyph const* const glyph = func_(*i);
		assert(glyph);
		units += glyph->is_full? 2 : 1;
	}
	return Rect(0, 0, units * HALF_WIDTH, HEIGHT);
}

void ShinonomeFont::Render(Bitmap& bmp, int const x, int const y, Bitmap const& sys, int color, unsigned code) {
	if(color != ColorShadow) {
		Render(bmp, x + 1, y + 1, sys, ColorShadow, code);
	}

	ShinonomeGlyph const* const glyph = func_(code);
	assert(glyph);
	size_t const width = glyph->is_full? FULL_WIDTH : HALF_WIDTH;

	unsigned const
		src_x = color == ColorShadow? 16 : color % 10 * 16 + (16 - width) / 2,
	    src_y = color == ColorShadow? 32 : color / 10 * 16 + 48 + (16 - HEIGHT) / 2;

	for(size_t y_ = 0; y_ < HEIGHT; ++y_) {
		for(size_t x_ = 0; x_ < width; ++x_) {
			if(glyph->data[y_] & (0x1 << x_)) {
				bmp.SetPixel(x + x_, y + y_, sys.GetPixel(src_x + x_, src_y + y_));
			}
		}
	}
}

void ShinonomeFont::Render(Bitmap& bmp, int x, int y, Color const& color, unsigned code) {
	ShinonomeGlyph const* const glyph = func_(code);
	assert(glyph);
	size_t const width = glyph->is_full? FULL_WIDTH : HALF_WIDTH;

	for(size_t y_ = 0; y_ < HEIGHT; ++y_) {
		for(size_t x_ = 0; x_ < width; ++x_) {
			if(glyph->data[y] & (0x1 << x_)) {
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
