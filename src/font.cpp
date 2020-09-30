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
#include <iterator>

#include "system.h"
#include "game_system.h"
#include "main_data.h"

#ifdef HAVE_FREETYPE
#	include <ft2build.h>
#	include FT_FREETYPE_H
#	include FT_BITMAP_H
#	include FT_MODULE_H
#endif

#include <lcf/reader_util.h>
#include "bitmapfont.h"

#include "filefinder.h"
#include "output.h"
#include "font.h"
#include "bitmap.h"
#include "utils.h"
#include "cache.h"
#include "player.h"
#include "compiler.h"

// Static variables.
namespace {
    template <typename T>
	BitmapFontGlyph const* find_glyph(const T& glyphset, char32_t code) {
		auto iter = std::lower_bound(std::begin(glyphset), std::end(glyphset), code);
		if(iter != std::end(glyphset) && iter->code == code) {
			return &*iter;
		} else {
			return NULL;
		}
	}

	// This is the last-resort function for finding a glyph, all the other fonts should fallback on it.
	// It tries to display a WenQuanYi glyph, and if it’s not found, returns a replacement glyph.
	BitmapFontGlyph const* find_fallback_glyph(char32_t code) {
		auto* wqy = find_glyph(BITMAPFONT_WQY, code);
		if (wqy != NULL) {
			return wqy;
		}
		else {
			static BitmapFontGlyph const replacement_glyph = { 65533, true, { 96, 240, 504, 924, 1902, 3967, 4031, 1982, 1020, 440, 240, 96 } };
			Output::Debug("glyph not found: {:#x}", uint32_t(code));
			return &replacement_glyph;
		}
	}

	BitmapFontGlyph const* find_gothic_glyph(char32_t code) {
		auto* gothic = find_glyph(SHINONOME_GOTHIC, code);
		return gothic != NULL ? gothic : find_fallback_glyph(code);
	}

	BitmapFontGlyph const* find_mincho_glyph(char32_t code) {
		auto* mincho = find_glyph(SHINONOME_MINCHO, code);
		return mincho == NULL ? find_gothic_glyph(code) : mincho;
	}

	BitmapFontGlyph const* find_rmg2000_glyph(char32_t code) {
		auto* rmg2000 = find_glyph(BITMAPFONT_RMG2000, code);
		if (rmg2000 != NULL) {
			return rmg2000;
		}

		auto* ttyp0 = find_glyph(BITMAPFONT_TTYP0, code);
		return ttyp0 != NULL ? ttyp0 : find_mincho_glyph(code);
	}

	BitmapFontGlyph const* find_ttyp0_glyph(char32_t code) {
		auto* ttyp0 = find_glyph(BITMAPFONT_TTYP0, code);
		return ttyp0 != NULL ? ttyp0 : find_gothic_glyph(code);
	}

	struct BitmapFont : public Font {
		enum { HEIGHT = 12, FULL_WIDTH = HEIGHT, HALF_WIDTH = FULL_WIDTH / 2 };

		using function_type = BitmapFontGlyph const*(*)(char32_t);

		BitmapFont(const std::string& name, function_type func);

		Rect GetSize(StringView txt) const override;
		Rect GetSize(char32_t ch) const override;

		GlyphRet Glyph(char32_t code) override;

	private:
		function_type func;
		BitmapRef glyph_bm;
	}; // class BitmapFont

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

		Rect GetSize(StringView txt) const override;
		Rect GetSize(char32_t ch) const override;

		GlyphRet Glyph(char32_t code) override;

	private:
		static std::weak_ptr<std::remove_pointer<FT_Library>::type> library_checker_;
		std::shared_ptr<std::remove_pointer<FT_Library>::type> library_;
		std::shared_ptr<std::remove_pointer<FT_Face>::type> face_;
		std::string face_name_;
		unsigned current_size_;

		bool check_face();
	}; // class FTFont
#endif

	/* Bitmap fonts used for the official Japanese version.
	   Compatible with MS Gothic and MS Mincho. Feature a closing quote in place of straight quote,
	   double-width Cyrillic letters (unusable for Russian, only useful for smileys and things like that)
	   and ellipsis in the middle of the line.
	*/
	FontRef const gothic = std::make_shared<BitmapFont>("Shinonome Gothic", &find_gothic_glyph);
	FontRef const mincho = std::make_shared<BitmapFont>("Shinonome Mincho", &find_mincho_glyph);

	/* Bitmap fonts used for non-Japanese games.
	   Compatible with RMG2000 and RM2000 shipped with Don Miguel’s unofficial translation.
	   Feature a half-width Cyrillic and half-width ellipsis at the bottom of the line.
	*/
	FontRef const rmg2000 = std::make_shared<BitmapFont>("RMG2000-compatible", &find_rmg2000_glyph);
	FontRef const ttyp0 = std::make_shared<BitmapFont>("ttyp0", &find_ttyp0_glyph);

	struct ExFont : public Font {
		public:
			enum { HEIGHT = 12, WIDTH = 12 };
			ExFont();
			Rect GetSize(StringView txt) const override;
			Rect GetSize(char32_t ch) const override;
			GlyphRet Glyph(char32_t code) override;
		private:
			BitmapRef bm;
	};
} // anonymous namespace

BitmapFont::BitmapFont(const std::string& name, function_type func)
	: Font(name, HEIGHT, false, false), func(func)
{}

Rect BitmapFont::GetSize(char32_t ch) const {
	size_t units = 0;
	if (EP_LIKELY(!Utils::IsControlCharacter(ch))) {
		auto glyph = func(ch);
		units += glyph->is_full? 2 : 1;
	}
	return Rect(0, 0, units * HALF_WIDTH, HEIGHT);
}

Rect BitmapFont::GetSize(StringView txt) const {
	size_t units = 0;
	const auto* iter = txt.data();
	const auto* end = txt.data() + txt.size();
	while (iter != end) {
		auto resp = Utils::UTF8Next(iter, end);
		auto ch = resp.ch;
		iter = resp.next;
		if (EP_LIKELY(!Utils::IsControlCharacter(resp.ch))) {
			auto glyph = func(ch);
			units += glyph->is_full? 2 : 1;
		}
	}
	return Rect(0, 0, units * HALF_WIDTH, HEIGHT);
}

Font::GlyphRet BitmapFont::Glyph(char32_t code) {
	if (EP_UNLIKELY(!glyph_bm)) {
		glyph_bm = Bitmap::Create(nullptr, FULL_WIDTH, HEIGHT, 0, DynamicFormat(8,8,0,8,0,8,0,8,0,PF::Alpha));
	}
	if (EP_UNLIKELY(Utils::IsControlCharacter(code))) {
		return { glyph_bm, Rect(0, 0, 0, HEIGHT) };
	}
	auto glyph = func(code);
	auto width = glyph->is_full? FULL_WIDTH : HALF_WIDTH;

	glyph_bm->Clear();
	uint8_t* data = reinterpret_cast<uint8_t*>(glyph_bm->pixels());
	int pitch = glyph_bm->pitch();
	for(size_t y_ = 0; y_ < HEIGHT; ++y_)
		for(size_t x_ = 0; x_ < width; ++x_)
			data[y_*pitch+x_] = (glyph->data[y_] & (0x1 << x_)) ? 255 : 0;

	return { glyph_bm, Rect(0, 0, width, HEIGHT) };
}

#ifdef HAVE_FREETYPE
std::weak_ptr<std::remove_pointer<FT_Library>::type> FTFont::library_checker_;

FTFont::FTFont(const std::string& name, int size, bool bold, bool italic)
	: Font(name, size, bold, italic), current_size_(0) {}

Rect FTFont::GetSize(StringView txt) const {
	int const s = Font::Default()->GetSize(txt).width;

	if (s == -1) {
		Output::Warning("Text contains invalid chars. Is the encoding correct?");

		return Rect(0, 0, pixel_size() * txt.length() / 2, pixel_size());
	} else {
		return Rect(0, 0, s, pixel_size());
	}
}

Rect FTFont::GetSize(char32_t ch) const {
	int const s = Font::Default()->GetSize(ch).width;

	if (s == -1) {
		Output::Warning("Text contains invalid chars. Is the encoding correct?");

		return Rect(0, 0, pixel_size() / 2, pixel_size());
	} else {
		return Rect(0, 0, s, pixel_size());
	}
}



Font::GlyphRet FTFont::Glyph(char32_t glyph) {
	if(!check_face()) {
		return Font::Default()->Glyph(glyph);
	}

	if (FT_Load_Char(face_.get(), glyph, FT_LOAD_NO_BITMAP) != FT_Err_Ok) {
		Output::Error("Couldn't load FreeType character {:#x}", uint32_t(glyph));
	}

	if (FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_MONO) != FT_Err_Ok) {
		Output::Error("Couldn't render FreeType character {:#x}", uint32_t(glyph));
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

	return { bm, Rect(0, 0, width, height) };
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
				Output::Error("Couldn't initialize FreeType face: {}({})",
					name, face_path);
				return false;
			}

			for (int i = 0; i < face_->num_fixed_sizes; i++) {
				FT_Bitmap_Size* size = &face_->available_sizes[i];
				Output::Debug("Font Size {}: {} {} {} {} {}", i,
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

FontRef Font::Default() {
	const bool mincho = (Main_Data::game_system && Main_Data::game_system->GetFontId() == lcf::rpg::System::Font_mincho);
	return Default(mincho);
}

FontRef Font::Default(bool const m) {
	if (Player::IsCJK()) {
		return m ? mincho : gothic;
	}
	else {
		return m ? rmg2000 : ttyp0;
	}
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
		Output::Debug("possible leak in cached font face {}", i->first);
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

Rect Font::Render(Bitmap& dest, int const x, int const y, const Bitmap& sys, int color, char32_t code) {
	auto gret = Glyph(code);

	auto rect = Rect(x, y, gret.rect.width, gret.rect.height);
	if (EP_UNLIKELY(rect.width == 0)) {
		return rect;
	}

	if(color != ColorShadow) {
		auto shadow_rect = Rect(x + 1, y + 1, rect.width, rect.height);
		dest.MaskedBlit(shadow_rect, *gret.bitmap, 0, 0, sys, 16, 32);
	}

	unsigned const
		src_x = color == ColorShadow? 16 : color % 10 * 16 + 2,
		src_y = color == ColorShadow? 32 : color / 10 * 16 + 48 + 16 - gret.bitmap->height();


	dest.MaskedBlit(rect, *gret.bitmap, 0, 0, sys, src_x, src_y);

	return rect;
}

Rect Font::Render(Bitmap& dest, int x, int y, Color const& color, char32_t code) {
	auto gret = Glyph(code);

	auto rect = Rect(x, y, gret.rect.width, gret.rect.height);
	dest.MaskedBlit(rect, *gret.bitmap, 0, 0, color);

	return rect;
}

ExFont::ExFont() : Font("exfont", 12, false, false) {
}

FontRef Font::exfont = std::make_shared<ExFont>();

Font::GlyphRet ExFont::Glyph(char32_t code) {
	if (EP_UNLIKELY(!bm)) { bm = Bitmap::Create(WIDTH, HEIGHT, true); }
	auto exfont = Cache::Exfont();

	Rect const rect((code % 13) * WIDTH, (code / 13) * HEIGHT, WIDTH, HEIGHT);
	bm->Clear();
	bm->Blit(0, 0, *exfont, rect, Opacity::Opaque());
	return { bm, Rect(0, 0, WIDTH, HEIGHT) };
}

Rect ExFont::GetSize(StringView) const {
	return Rect(0, 0, 12, 12);
}

Rect ExFont::GetSize(char32_t) const {
	return Rect(0, 0, 12, 12);
}
