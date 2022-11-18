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

#include "filesystem_stream.h"
#include "system.h"
#include "game_system.h"
#include "main_data.h"

#ifdef HAVE_FREETYPE
#	include <ft2build.h>
#	include FT_FREETYPE_H
#	include FT_BITMAP_H
#	include FT_MODULE_H
#endif

#ifdef HAVE_HARFBUZZ
#   include <hb-ft.h>
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
		if (Player::IsCP936()) {
			auto* wqy = find_glyph(BITMAPFONT_WQY, code);
			if (wqy != NULL) {
				return wqy;
			}
		}
		auto* gothic = find_glyph(SHINONOME_GOTHIC, code);
		return gothic != NULL ? gothic : find_fallback_glyph(code);
	}

	BitmapFontGlyph const* find_mincho_glyph(char32_t code) {
		if (Player::IsCP936()) {
			auto* wqy = find_glyph(BITMAPFONT_WQY, code);
			if (wqy != NULL) {
				return wqy;
			}
		}
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

	struct BitmapFont final : public Font {
		enum { HEIGHT = 12, FULL_WIDTH = HEIGHT, HALF_WIDTH = FULL_WIDTH / 2 };

		using function_type = BitmapFontGlyph const*(*)(char32_t);

		BitmapFont(StringView name, function_type func);

		Rect GetSize(StringView txt) const override;
		Rect GetSize(char32_t ch) const override;

		GlyphRet Glyph(char32_t code) override;

	private:
		function_type func;
		BitmapRef glyph_bm;
	}; // class BitmapFont

#ifdef HAVE_FREETYPE
	FT_Library library = nullptr;

	struct FTFont final : public Font  {
		FTFont(Filesystem_Stream::InputStream is, int size, bool bold, bool italic);
		~FTFont() override;

		Rect GetSize(StringView txt) const override;
		Rect GetSize(char32_t ch) const override;

		GlyphRet Glyph(char32_t code) override;

	private:
		FT_Face face = nullptr;
		std::vector<uint8_t> ft_buffer;
		// Freetype uses the baseline as 0 and the built-in fonts the top
		// baseline_offset is subtracted from the baseline to get a proper rendering position
		int baseline_offset = 0;
		/** Workaround for bad kerning in RM2000 and RMG2000 fonts */
		bool rm2000_workaround = false;
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

	FontRef default_gothic;
	FontRef default_mincho;

	struct ExFont final : public Font {
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

BitmapFont::BitmapFont(StringView name, function_type func)
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
	return {0, 0, static_cast<int>(units * HALF_WIDTH), HEIGHT};
}

Font::GlyphRet BitmapFont::Glyph(char32_t code) {
	if (EP_UNLIKELY(!glyph_bm)) {
		glyph_bm = Bitmap::Create(nullptr, FULL_WIDTH, HEIGHT, 0, DynamicFormat(8,8,0,8,0,8,0,8,0,PF::Alpha));
	}
	if (EP_UNLIKELY(Utils::IsControlCharacter(code))) {
		return { glyph_bm, {0, 0}, {0, 0} };
	}
	auto glyph = func(code);
	auto width = glyph->is_full? FULL_WIDTH : HALF_WIDTH;

	glyph_bm->Clear();
	uint8_t* data = reinterpret_cast<uint8_t*>(glyph_bm->pixels());
	int pitch = glyph_bm->pitch();
	for(size_t y_ = 0; y_ < HEIGHT; ++y_)
		for(size_t x_ = 0; x_ < width; ++x_)
			data[y_*pitch+x_] = (glyph->data[y_] & (0x1 << x_)) ? 255 : 0;

	return { glyph_bm, {width, 0}, {0, 0} };
}

#ifdef HAVE_FREETYPE
FTFont::FTFont(Filesystem_Stream::InputStream is, int size, bool bold, bool italic)
	: Font(is.GetName(), size, bold, italic) {

	if (!library) {
		if (FT_Init_FreeType(&library) != FT_Err_Ok) {
			Output::Error("Couldn't initialize FreeType");
			return;
		}
	}

	assert(is);

	ft_buffer = Utils::ReadStream(is);

	FT_New_Memory_Face(library, ft_buffer.data(), ft_buffer.size(), 0, &face);

	if (face->num_charmaps > 0) {
		// Force unicode charmap
		if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0) {
			// If this fails, force the first for legacy fonts (FON, BDF, etc.)
			FT_Set_Charmap(face, face->charmaps[0]);
		}
	}

	if (FT_HAS_COLOR(face)) {
		FT_Select_Size(face, 0);
	} else {
		FT_Set_Pixel_Sizes(face, 0, size);
	}

	baseline_offset = static_cast<int>(size * (10.0 / 12.0));

	if (!strcmp(face->family_name, "RM2000") || !strcmp(face->family_name, "RMG2000")) {
		// Workaround for bad kerning in RM2000 and RMG2000 fonts
		rm2000_workaround = true;
	}
}

FTFont::~FTFont() {
	if (!library) {
		// Freetype already shut down because of Player cleanup
		return;
	}

	if (face != nullptr) {
		FT_Done_Face(face);
	}
}

Rect FTFont::GetSize(StringView txt) const {
	Rect rect = {0, 0, 0, static_cast<int>(size)};

	std::u32string txt32 = Utils::DecodeUTF32(txt);
	for (char32_t i: txt32) {
		Rect grect = GetSize(i);
		rect.width += grect.width;
	}

	return rect;
}

Rect FTFont::GetSize(char32_t code) const {
	auto glyph_index = FT_Get_Char_Index(face, code);

	if (glyph_index == 0 && code != 0 && fallback_font) {
		return fallback_font->GetSize(code);
	}

	auto load_glyph = [&](auto flags) {
		if (FT_Load_Glyph(face, glyph_index, flags) != FT_Err_Ok) {
			Output::Error("Couldn't load FreeType character {:#x}", uint32_t(code));
		}
	};

	if (FT_HAS_COLOR(face)) {
		load_glyph(FT_LOAD_COLOR);

		// When it is a color font check if the glyph is a color glyph
		// If it is not then reload the glyph monochrome
		if (face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA) {
			load_glyph(FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO);
		}
	} else {
		load_glyph(FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO);
	}

	FT_GlyphSlot slot = face->glyph;

	Point advance;
	advance.x = slot->advance.x / 64;
	advance.y = slot->advance.y / 64;

	if (EP_UNLIKELY(rm2000_workaround)) {
		advance.x = 6;
	}

	return {0, 0, advance.x, advance.y};
}

Font::GlyphRet FTFont::Glyph(char32_t code) {
    auto glyph_index = FT_Get_Char_Index(face, code);

	if (glyph_index == 0 && code != 0 && fallback_font) {
		return fallback_font->Glyph(code);
	}

	auto render_glyph = [&](auto flags, auto mode) {
		if (FT_Load_Glyph(face, glyph_index, flags) != FT_Err_Ok) {
			Output::Error("Couldn't load FreeType character {:#x}", uint32_t(code));
		}

		if (FT_Render_Glyph(face->glyph, mode) != FT_Err_Ok) {
			Output::Error("Couldn't render FreeType character {:#x}", uint32_t(code));
		}
	};

	if (FT_HAS_COLOR(face)) {
		render_glyph(FT_LOAD_COLOR, FT_RENDER_MODE_NORMAL);

		// When it is a color font check if the glyph is a color glyph
		// If it is not then rerender the glyph monochrome
		// FIXME: This is inefficient
		if (face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA) {
			render_glyph(FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO, FT_RENDER_MODE_MONO);
		}
	} else {
		render_glyph(FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO, FT_RENDER_MODE_MONO);
	}

	FT_GlyphSlot slot = face->glyph;
	FT_Bitmap* ft_bitmap = &slot->bitmap;

	assert(ft_bitmap->pixel_mode == FT_PIXEL_MODE_MONO || ft_bitmap->pixel_mode == FT_PIXEL_MODE_BGRA);

	size_t const pitch = std::abs(ft_bitmap->pitch);
	const int width = ft_bitmap->width;
	const int height = ft_bitmap->rows;

	BitmapRef bm;
	bool has_color = false;

	if (ft_bitmap->pixel_mode == FT_PIXEL_MODE_BGRA) {
		bm = Bitmap::Create(ft_bitmap->buffer, width, height, 0, format_B8G8R8A8_a().format());
		has_color = true;
	} else {
		bm = Bitmap::Create(width, height);
		auto* data = reinterpret_cast<uint32_t*>(bm->pixels());

		for (int row = 0; row < height; ++row) {
			for (int col = 0; col < width; ++col) {
				unsigned c = ft_bitmap->buffer[pitch * row + (col / 8)];
				unsigned bit = 7 - (col % 8);
				c = c & (0x01 << bit) ? 255 : 0;
				data[row * width + col] = (c << 24) + (c << 16) + (c << 8) + c;
			}
		}
	}

	Point advance;
	Point offset;

	advance.x = slot->advance.x / 64;
	advance.y = slot->advance.y / 64;
	offset.x = slot->bitmap_left;
	offset.y = slot->bitmap_top - baseline_offset;

	if (EP_UNLIKELY(rm2000_workaround)) {
		advance.x = 6;
	}

	return { bm, advance, offset, has_color };
}
#endif

FontRef Font::Default() {
	const bool m = (Main_Data::game_system && Main_Data::game_system->GetFontId() == lcf::rpg::System::Font_mincho);
	return Default(m);
}

FontRef Font::Default(bool const use_mincho) {
	if (use_mincho && default_mincho) {
		return default_mincho;
	} else if (!use_mincho && default_gothic) {
		return default_gothic;
	}

	return DefaultBitmapFont(use_mincho);
}

FontRef Font::DefaultBitmapFont() {
	const bool m = (Main_Data::game_system && Main_Data::game_system->GetFontId() == lcf::rpg::System::Font_mincho);
	return DefaultBitmapFont(m);
}

FontRef Font::DefaultBitmapFont(bool use_mincho) {
	if (Player::IsCJK()) {
		return use_mincho ? mincho : gothic;
	}
	else {
		return use_mincho ? rmg2000 : ttyp0;
	}
}

void Font::SetDefault(FontRef new_default, bool use_mincho) {
	if (use_mincho) {
		default_mincho = new_default;
	} else {
		default_gothic = new_default;
	}

	if (new_default) {
		new_default->SetFallbackFont(DefaultBitmapFont(use_mincho));
	}
}

FontRef Font::CreateFtFont(Filesystem_Stream::InputStream is, int size, bool bold, bool italic) {
#ifdef HAVE_FREETYPE
	return std::make_shared<FTFont>(std::move(is), size, bold, italic);
#else
	return nullptr;
#endif
}

void Font::ResetDefault() {
	SetDefault(nullptr, true);
	SetDefault(nullptr, false);
}

void Font::Dispose() {
	ResetDefault();

#ifdef HAVE_FREETYPE
	if (library) {
		FT_Done_Library(library);
		library = nullptr;
	}
#endif
}

// Constructor.
Font::Font(StringView name, int size, bool bold, bool italic)
	: name(ToString(name))
	, size(size)
	, bold(bold)
	, italic(italic)
{
}

Point Font::Render(Bitmap& dest, int const x, int const y, const Bitmap& sys, int color, char32_t code) {
	auto gret = Glyph(code);

	auto rect = Rect(x, y, gret.bitmap->width(), gret.bitmap->height());
	if (EP_UNLIKELY(rect.width == 0)) {
		return {};
	}

	rect.x += gret.offset.x;
	rect.y -= gret.offset.y;

	unsigned src_x;
	unsigned src_y;

	if (color != ColorShadow) {
		if (!gret.has_color) {
			auto shadow_rect = Rect(rect.x + 1, rect.y + 1, rect.width, rect.height);
			dest.MaskedBlit(shadow_rect, *gret.bitmap, 0, 0, sys, 16, 32);
		}

		src_x = color % 10 * 16 + 2;
		src_y = color / 10 * 16 + 48 + 16 - 12 - gret.offset.y;

		// When the glyph is large the system graphic color mask will be outside the rectangle
		// Move the mask slightly up to avoid this
		int offset = gret.bitmap->height() - gret.offset.y;
		if (offset > 12) {
			src_y -= offset - 12;
		}
	} else {
		src_x = 16;
		src_y = 32;
	}

	if (!gret.has_color) {
		dest.MaskedBlit(rect, *gret.bitmap, 0, 0, sys, src_x, src_y);
	} else {
		dest.Blit(rect.x, rect.y, *gret.bitmap, gret.bitmap->GetRect(), Opacity::Opaque());
	}

	return gret.advance;
}

Point Font::Render(Bitmap& dest, int x, int y, Color const& color, char32_t code) {
	auto gret = Glyph(code);

	auto rect = Rect(x, y, gret.bitmap->width(), gret.bitmap->height());
	dest.MaskedBlit(rect, *gret.bitmap, 0, 0, color);

	return gret.advance;
}

void Font::SetFallbackFont(FontRef fallback_font) {
	this->fallback_font = fallback_font;
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

	// EasyRPG Extension: Support for colored ExFont
	bool has_color = false;
	const auto* pixels = reinterpret_cast<uint8_t*>(bm->pixels());
	// For performance reasons only check the red channel of every 4th pixel (16 = 4 * 4 RGBA pixel) for color
	for (int i = 0; i < bm->pitch() * bm->height(); i += 16) {
		auto pixel = pixels[i];
		if (pixel != 0 && pixel != 255) {
			has_color = true;
			break;
		}
	}

	return { bm, {WIDTH, 0}, {0, 0}, has_color };
}

Rect ExFont::GetSize(StringView) const {
	return Rect(0, 0, 12, 12);
}

Rect ExFont::GetSize(char32_t) const {
	return Rect(0, 0, 12, 12);
}
