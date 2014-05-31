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
#include <map>
#include <ciso646>

#include <boost/next_prior.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_MODULE_H

#include "reader_util.h"
#include "shinonome.hxx"

#include "filefinder.h"
#include "output.h"
#include "font.h"
#include "bitmap.h"
#include "utils.h"

bool operator<(ShinonomeGlyph const& lhs, uint32_t const code) {
	return lhs.code < code;
}

// Static variables.
namespace {
	typedef std::map<std::string, EASYRPG_WEAK_PTR<boost::remove_pointer<FT_Face>::type> > face_cache_type;
	face_cache_type face_cache;
	ShinonomeGlyph const* find_glyph(ShinonomeGlyph const* data, size_t size, uint32_t code) {
		ShinonomeGlyph const* ret = std::lower_bound(data, data + size, code);
		if(ret != (data + size) and ret->code == code) {
			return ret;
		} else {
			static ShinonomeGlyph const empty_glyph = { 0, true, {0} };
			Output::Debug("glyph not found: 0x%04x", code);
			return &empty_glyph;
		}
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
		enum { HEIGHT = 12, FULL_WIDTH = HEIGHT, HALF_WIDTH = FULL_WIDTH / 2 };

		typedef ShinonomeGlyph const*(*function_type)(uint32_t);

		ShinonomeFont(function_type func);

		Rect GetSize(std::string const& txt) const;

		void Render(Bitmap& bmp, int x, int y, Bitmap const& sys, int color, unsigned glyph);
		void Render(Bitmap& bmp, int x, int y, Color const& color, unsigned glyph);

	private:
		function_type const func_;
	}; // class ShinonomeFont


	void delete_face(FT_Face f) {
		if(FT_Done_Face(f) != FT_Err_Ok) {
			Output::Warning("FT_Face deleting error.");
		}
	}

	void delete_library(FT_Library f) {
		if(FT_Done_Library(f) != FT_Err_Ok) {
			Output::Warning("FT_Library deleting error.");
		}
	}

	struct FTFont : public Font  {
		FTFont(const std::string& name, int size, bool bold, bool italic);

		Rect GetSize(std::string const& txt) const;

		void Render(Bitmap& bmp, int x, int y, Bitmap const& sys, int color, unsigned glyph);
		void Render(Bitmap& bmp, int x, int y, Color const& color, unsigned glyph);

	private:
		static EASYRPG_WEAK_PTR<boost::remove_pointer<FT_Library>::type> library_checker_;
		EASYRPG_SHARED_PTR<boost::remove_pointer<FT_Library>::type> library_;
		EASYRPG_SHARED_PTR<boost::remove_pointer<FT_Face>::type> face_;
		std::string face_name_;
		unsigned current_size_;

		bool check_face();
	}; // class FTFont

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
			if(glyph->data[y_] & (0x1 << x_)) {
				bmp.SetPixel(x + x_, y + y_, color);
			}
		}
	}
}

EASYRPG_WEAK_PTR<boost::remove_pointer<FT_Library>::type> FTFont::library_checker_;

FTFont::FTFont(const std::string& name, int size, bool bold, bool italic)
	: Font(name, size, bold, italic), current_size_(0) {}

Rect FTFont::GetSize(std::string const& txt) const {
	Utils::wstring tmp = Utils::ToWideString(txt);
	int const s = Font::Default()->GetSize(txt).width;

	if (s == -1) {
		Output::Warning("Text contains invalid chars.\n"\
			"Is the encoding correct?");

		return Rect(0, 0, pixel_size() * txt.size() / 2, pixel_size());
	} else {
		return Rect(0, 0, s, pixel_size());
	}
}

void FTFont::Render(Bitmap& bmp, int x, int y, Bitmap const& /* sys */, int /* color */, unsigned glyph) {
	Render(bmp, x, y, Color(), glyph);
}

void FTFont::Render(Bitmap& bmp, int const x, int const y, Color const& color, unsigned const glyph) {
	if(!check_face()) {
		Font::Default()->Render(bmp, x, y, color, glyph);
		return;
	}

	if (FT_Load_Char(face_.get(), glyph, FT_LOAD_NO_BITMAP) != FT_Err_Ok) {
		Output::Error("Couldn't load FreeType character %d", glyph);
		return;
	}

    if (FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_MONO) != FT_Err_Ok) {
		Output::Error("Couldn't render FreeType character %d", glyph);
		return;
	}

	FT_Bitmap const& ft_bitmap = face_->glyph->bitmap;
	assert(face_->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);

	size_t const pitch = std::abs(ft_bitmap.pitch);

	for(int row = 0; row < ft_bitmap.rows; ++row) {
		for(size_t col = 0; col < pitch; ++col) {
			unsigned c = ft_bitmap.buffer[pitch * row + col];
			for(int bit = 7; bit >= 0; --bit) {
				if(c & (0x01 << bit)) {
					bmp.SetPixel(x + col * 8 + (7 - bit), y + row, color);
				}
			}
		}
	}
}

FontRef Font::Default(bool const m) {
	return m? mincho : gothic;
}

FontRef Font::Create(const std::string& name, int size, bool bold, bool italic) {
	return EASYRPG_MAKE_SHARED<FTFont>(name, size, bold, italic);
}

void Font::Dispose() {
	for(face_cache_type::const_iterator i = face_cache.begin(); i != face_cache.end(); ++i) {
		if(i->second.expired()) { continue; }
		Output::Debug("possible leak in cached font face %s", i->first.c_str());
	}
	face_cache.clear();
}

// Constructor.
Font::Font(const std::string& name, int size, bool bold, bool italic)
	: name(name)
	, size(size)
	, bold(bold)
	, italic(italic)
{
}

bool FTFont::check_face() {
	if(!library_) {
		if(library_checker_.expired()) {
			FT_Library lib;
			if(FT_Init_FreeType(&lib) != FT_Err_Ok) {
				Output::Error("Couldn't initialize FreeType");
				return false;
			}
			library_.reset(lib, delete_library);
			library_checker_ = library_;
		} else {
			library_ = library_checker_.lock();
		}
	}

	if(!face_ || face_name_ != name) {
	    face_cache_type::const_iterator it = face_cache.find(name);
		if(it == face_cache.end() || it->second.expired()) {
			std::string const face_path = FileFinder::FindFont(name);
			FT_Face face;
			if(FT_New_Face(library_.get(), face_path.c_str(), 0, &face) != FT_Err_Ok) {
				Output::Error("Couldn't initialize FreeType face: %s(%s)",
							  name.c_str(), face_path.c_str());
				return false;
			}

			for (int i = 0; i < face_->num_fixed_sizes; i++) {
				FT_Bitmap_Size* size = &face_->available_sizes[i];
				Output::Debug("Font Size %d: %d %d %f %f %f", i,
							  size->width, size->height, size->size / 64.0,
							  size->x_ppem / 64.0, size->y_ppem / 64.0);
			}

			face_.reset(face, delete_face);
			face_cache[name] = face_;
		} else {
			face_ = it->second.lock();
		}
		face_name_ = name;
	}

	face_->style_flags =
		(bold? FT_STYLE_FLAG_BOLD : 0) |
		(italic? FT_STYLE_FLAG_ITALIC : 0);

	if(current_size_ != size) {
		int sz, dpi;
		if (face_->num_fixed_sizes == 1) {
			sz = face_->available_sizes[0].size;
			dpi = 96;
		} else {
			sz = size * 64;
			dpi = 72;
		}

		if (FT_Set_Char_Size(face_.get(), sz, sz, dpi, dpi) != FT_Err_Ok) {
			Output::Error("Couldn't set FreeType face size");
			return false;
		}
		current_size_ = size;
	}

	return true;
}
