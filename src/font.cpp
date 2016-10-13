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
#include <type_traits>
#include <vector>

#include "system.h"

#ifdef HAVE_FREETYPE
#	include <ft2build.h>
#	include FT_FREETYPE_H
#	include FT_BITMAP_H
#	include FT_MODULE_H
#endif

#include "reader_util.h"
#include "shinonome.h"

#include "filefinder.h"
#include "output.h"
#include "font.h"
#include "bitmap.h"
#include "utils.h"
#include "cache.h"

bool operator<(ShinonomeGlyph const& lhs, uint32_t const code) {
	return lhs.code < code;
}

// Static variables.
namespace {
	ShinonomeGlyph const* find_glyph(ShinonomeGlyph const* data, size_t size, char32_t code) {
		ShinonomeGlyph const* ret = std::lower_bound(data, data + size, code);
		if(ret != (data + size) && ret->code == code) {
			return ret;
		} else {
			static ShinonomeGlyph const replacement_glyph = { 65533, true, { 96, 240, 504, 924, 1902, 3967, 4031, 1982, 1020, 440, 240, 96 } };
			Output::Debug("glyph not found: 0x%04x", code);
			return &replacement_glyph;
		}
	}

	ShinonomeGlyph const* find_fallback_glyph(char32_t code) {
		return find_glyph(SHINONOME_WQY,
					   sizeof(SHINONOME_WQY) / sizeof(ShinonomeGlyph), code);
	}

	ShinonomeGlyph const* find_gothic_glyph(char32_t code) {
		ShinonomeGlyph const* const gothic =
			find_glyph(SHINONOME_GOTHIC,
					   sizeof(SHINONOME_GOTHIC) / sizeof(ShinonomeGlyph), code);
		return (gothic != NULL && gothic->code == code)? gothic : find_fallback_glyph(code);
	}

	ShinonomeGlyph const* find_mincho_glyph(char32_t code) {
		ShinonomeGlyph const* const mincho =
			find_glyph(SHINONOME_MINCHO,
					   sizeof(SHINONOME_MINCHO) / sizeof(ShinonomeGlyph), code);
		return mincho == NULL? find_gothic_glyph(code) : mincho;
	}

	struct ShinonomeFont : public Font {
		enum { HEIGHT = 12, FULL_WIDTH = HEIGHT, HALF_WIDTH = FULL_WIDTH / 2 };

		using function_type = ShinonomeGlyph const*(*)(char32_t);

		ShinonomeFont(function_type func);

		Rect GetSize(std::u32string const& txt) const override;

		BitmapRef Glyph(char32_t code) override;

	private:
		function_type const func_;
	}; // class ShinonomeFont

#ifdef HAVE_FREETYPE
	typedef std::map<std::string, std::weak_ptr<std::remove_pointer<FT_Face>::type>> face_cache_type;
	face_cache_type face_cache;

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

		Rect GetSize(std::u32string const& txt) const override;

		BitmapRef Glyph(char32_t code) override;

	private:
		static std::weak_ptr<std::remove_pointer<FT_Library>::type> library_checker_;
		std::shared_ptr<std::remove_pointer<FT_Library>::type> library_;
		std::shared_ptr<std::remove_pointer<FT_Face>::type> face_;
		std::string face_name_;
		unsigned current_size_;

		bool check_face();
	}; // class FTFont
#endif

	FontRef const gothic = std::make_shared<ShinonomeFont>(&find_gothic_glyph);
	FontRef const mincho = std::make_shared<ShinonomeFont>(&find_mincho_glyph);

	struct ExFont : public Font {
		ExFont();
		Rect GetSize(std::u32string const& txt) const override;
		BitmapRef Glyph(char32_t code) override;
	};
} // anonymous namespace

ShinonomeFont::ShinonomeFont(ShinonomeFont::function_type func)
	: Font("Shinonome", HEIGHT, false, false), func_(func) {}

Rect ShinonomeFont::GetSize(std::u32string const& txt) const {
	size_t units = 0;
	for (char32_t c : txt) {
		ShinonomeGlyph const* const glyph = func_(c);
		assert(glyph);
		units += glyph->is_full? 2 : 1;
	}
	return Rect(0, 0, units * HALF_WIDTH, HEIGHT);
}

BitmapRef ShinonomeFont::Glyph(char32_t code) {
	ShinonomeGlyph const* const glyph = func_(code);
	assert(glyph);
	size_t const width = glyph->is_full? FULL_WIDTH : HALF_WIDTH;

	BitmapRef bm = Bitmap::Create(nullptr, width, HEIGHT, 0, DynamicFormat(8,8,0,8,0,8,0,8,0,PF::Alpha));
	uint8_t* data = reinterpret_cast<uint8_t*>(bm->pixels());
	int pitch = bm->pitch();
	for(size_t y_ = 0; y_ < HEIGHT; ++y_)
		for(size_t x_ = 0; x_ < width; ++x_)
			data[y_*pitch+x_] = (glyph->data[y_] & (0x1 << x_)) ? 255 : 0;

	return bm;
}

#ifdef HAVE_FREETYPE
std::weak_ptr<std::remove_pointer<FT_Library>::type> FTFont::library_checker_;

FTFont::FTFont(const std::string& name, int size, bool bold, bool italic)
	: Font(name, size, bold, italic), current_size_(0) {}

Rect FTFont::GetSize(std::u32string const& txt) const {
	int const s = Font::Default()->GetSize(txt).width;

	if (s == -1) {
		Output::Warning("Text contains invalid chars.\n"\
			"Is the encoding correct?");

		return Rect(0, 0, pixel_size() * txt.length() / 2, pixel_size());
	} else {
		return Rect(0, 0, s, pixel_size());
	}
}

BitmapRef FTFont::Glyph(char32_t glyph) {
	if(!check_face()) {
		return Font::Default()->Glyph(glyph);
	}

	if (FT_Load_Char(face_.get(), glyph, FT_LOAD_NO_BITMAP) != FT_Err_Ok) {
		Output::Error("Couldn't load FreeType character %d", glyph);
	}

    if (FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_MONO) != FT_Err_Ok) {
		Output::Error("Couldn't render FreeType character %d", glyph);
	}

	FT_Bitmap const& ft_bitmap = face_->glyph->bitmap;
	assert(face_->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);

	size_t const pitch = std::abs(ft_bitmap.pitch);
	int const width = ft_bitmap.width;
	int const height = ft_bitmap.rows;

	BitmapRef bm = Bitmap::Create(nullptr, width, height, 0, DynamicFormat(8,8,0,8,0,8,0,8,0,PF::Alpha));
	uint8_t* data = reinterpret_cast<uint8_t*>(bm->pixels());
	int dst_pitch = bm->pitch();

	for(int row = 0; row < height; ++row) {
		for(int col = 0; col < width; ++col) {
			unsigned c = ft_bitmap.buffer[pitch * row + (col/8)];
			unsigned bit = 7 - (col%8);
			data[row * dst_pitch + col] = (c & (0x01 << bit)) ? 255 : 0;
		}
	}

	return bm;
}


bool FTFont::check_face() {
	if (!library_) {
		if (library_checker_.expired()) {
			FT_Library lib;
			if (FT_Init_FreeType(&lib) != FT_Err_Ok) {
				Output::Error("Couldn't initialize FreeType");
				return false;
			}
			library_.reset(lib, delete_library);
			library_checker_ = library_;
		} else {
			library_ = library_checker_.lock();
		}
	}

	if (!face_ || face_name_ != name) {
		face_cache_type::const_iterator it = face_cache.find(name);
		if (it == face_cache.end() || it->second.expired()) {
			std::string const face_path = FileFinder::FindFont(name);
			FT_Face face;
			if (FT_New_Face(library_.get(), face_path.c_str(), 0, &face) != FT_Err_Ok) {
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
		(bold ? FT_STYLE_FLAG_BOLD : 0) |
		(italic ? FT_STYLE_FLAG_ITALIC : 0);

	if (current_size_ != size) {
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
#endif

FontRef Font::Default(bool const m) {
	return m? mincho : gothic;
}

FontRef Font::Create(const std::string& name, int size, bool bold, bool italic) {
#ifdef HAVE_FREETYPE
	return std::make_shared<FTFont>(name, size, bold, italic);
#else
	(void)name; (void)size; (void)bold; (void)italic;
	return Font::Default();
#endif
}

void Font::Dispose() {
#ifdef HAVE_FREETYPE
	for(face_cache_type::const_iterator i = face_cache.begin(); i != face_cache.end(); ++i) {
		if(i->second.expired()) { continue; }
		Output::Debug("possible leak in cached font face %s", i->first.c_str());
	}
	face_cache.clear();
#endif
}

// Constructor.
Font::Font(const std::string& name, int size, bool bold, bool italic)
	: name(name)
	, size(size)
	, bold(bold)
	, italic(italic)
{
}

Rect Font::GetSize(std::string const& txt) const {
	return GetSize(Utils::DecodeUTF32(txt));
}

void Font::Render(Bitmap& bmp, int const x, int const y, Bitmap const& sys, int color, char32_t code) {
	if(color != ColorShadow) {
		BitmapRef system = Cache::System();
		Render(bmp, x + 1, y + 1, system->GetShadowColor(), code);
	}

	BitmapRef bm = Glyph(code);

	unsigned const
		src_x = color == ColorShadow? 16 : color % 10 * 16 + 2,
		src_y = color == ColorShadow? 32 : color / 10 * 16 + 48 + 16 - bm->height();

	bmp.MaskedBlit(Rect(x, y, bm->width(), bm->height()), *bm, 0, 0, sys, src_x, src_y);
}

void Font::Render(Bitmap& bmp, int x, int y, Color const& color, char32_t code) {
	BitmapRef bm = Glyph(code);

	bmp.MaskedBlit(Rect(x, y, bm->width(), bm->height()), *bm, 0, 0, color);
}

ExFont::ExFont() : Font("exfont", 12, false, false) {
}

FontRef Font::exfont = std::make_shared<ExFont>();

BitmapRef ExFont::Glyph(char32_t code) {
	BitmapRef exfont = Cache::Exfont();
	Rect const rect((code % 13) * 12, (code / 13) * 12, 12, 12);
	return Bitmap::Create(*exfont, rect, true);
}

Rect ExFont::GetSize(std::u32string const& /* txt */) const {
	return Rect(0, 0, 12, 12);
}
