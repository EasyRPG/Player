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
#include <cstdint>
#include <map>
#include <type_traits>
#include <vector>
#include <iterator>

#include "filesystem_stream.h"
#include "system.h"
#include "game_system.h"
#include "main_data.h"

#ifdef HAVE_FREETYPE
#  include <ft2build.h>
#  include FT_FREETYPE_H
#  include FT_BITMAP_H
#  include FT_MODULE_H
#  include FT_TRUETYPE_TABLES_H
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

		Rect vGetSize(char32_t glyph) const override;
		GlyphRet vRender(char32_t glyph) const override;

	private:
		function_type func;
		mutable BitmapRef glyph_bm;
	}; // class BitmapFont

#ifdef HAVE_FREETYPE
	FT_Library library = nullptr;

	struct FTFont final : public Font  {
		FTFont(Filesystem_Stream::InputStream is, int size, bool bold, bool italic);
		~FTFont() override;

		Rect vGetSize(char32_t glyph) const override;
		GlyphRet vRender(char32_t glyph) const override;
		GlyphRet vRenderShaped(char32_t glyph) const override;
		bool vCanShape() const override;
#ifdef HAVE_HARFBUZZ
		std::vector<ShapeRet> vShape(U32StringView txt) const override;
#endif
		void vApplyStyle(const Style& style) override;

	private:
		void SetSize(int height, bool create);

		FT_Face face = nullptr;
		std::vector<uint8_t> ft_buffer;
		// Freetype uses the baseline as 0 and the built-in fonts the top
		// baseline_offset is subtracted from the baseline to get a proper rendering position
		int baseline_offset = 0;
		/** Workaround for bad kerning in RM2000 and RMG2000 fonts */
		bool rm2000_workaround = false;

#ifdef HAVE_HARFBUZZ
		hb_buffer_t* hb_buffer = nullptr;
		hb_font_t* hb_font = nullptr;
#endif
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
			Rect vGetSize(char32_t glyph) const override;
			GlyphRet vRender(char32_t glyph) const override;
		private:
			mutable BitmapRef bm;
	};

	/** FreeType Font Cache */
	struct CacheItem {
		FontRef font;
		Game_Clock::time_point last_access;
	};

	using key_type = std::string;
	std::unordered_map<key_type, CacheItem> ft_cache;

	// Hard to track the size of a font
	// Instead limit the cache to the last 3 referenced fonts
	constexpr int cache_limit = 3;
	size_t cache_size = 0;

	using namespace std::chrono_literals;

	void FreeFontMemory() {
		auto cur_ticks = Game_Clock::GetFrameTime();

		for (auto it = ft_cache.begin(); it != ft_cache.end();) {
			if (it->second.font.use_count() != 1) {
				// Font is referenced
				++it;
				continue;
			}

			auto last_access = cur_ticks - it->second.last_access;
			bool cache_exhausted = cache_size > cache_limit;
			if (cache_exhausted) {
				if (last_access <= 50ms) {
					// Used during the last 3 frames, must be important, keep it.
					++it;
					continue;
				}
			} else if (last_access <= 3s) {
				++it;
				continue;
			}
			cache_size -= 1;

			it = ft_cache.erase(it);
		}
	}
} // anonymous namespace

BitmapFont::BitmapFont(StringView name, function_type func)
	: Font(name, HEIGHT, false, false), func(func)
{}

Rect BitmapFont::vGetSize(char32_t glyph) const {
	auto bm_glyph = func(glyph);
	size_t units = bm_glyph->is_full ? 2 : 1;
	return {0, 0, static_cast<int>(units * HALF_WIDTH), HEIGHT};
}

Font::GlyphRet BitmapFont::vRender(char32_t glyph) const {
	std::vector<Font::GlyphRet> glyphs;

	if (EP_UNLIKELY(!glyph_bm)) {
		glyph_bm = Bitmap::Create(nullptr, FULL_WIDTH, HEIGHT, 0, DynamicFormat(8, 8, 0, 8, 0, 8, 0, 8, 0, PF::Alpha));
	}
	auto bm_glyph = func(glyph);
	auto width = bm_glyph->is_full ? FULL_WIDTH : HALF_WIDTH;

	glyph_bm->Clear();
	uint8_t* data = reinterpret_cast<uint8_t*>(glyph_bm->pixels());
	int pitch = glyph_bm->pitch();
	for (size_t y_ = 0; y_ < HEIGHT; ++y_)
		for (size_t x_ = 0; x_ < width; ++x_)
			data[y_ * pitch + x_] = (bm_glyph->data[y_] & (0x1 << x_)) ? 255 : 0;

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

	SetSize(size, true);

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

#ifdef HAVE_HARFBUZZ
	if (hb_buffer) {
		hb_buffer_destroy(hb_buffer);
	}

	if (hb_font) {
		hb_font_destroy(hb_font);
	}
#endif

	if (face) {
		FT_Done_Face(face);
	}
}

Rect FTFont::vGetSize(char32_t glyph) const {
	auto glyph_index = FT_Get_Char_Index(face, glyph);

	if (glyph_index == 0 && fallback_font) {
		return fallback_font->vGetSize(glyph);
	}

	auto load_glyph = [&](auto flags) {
		if (FT_Load_Glyph(face, glyph_index, flags) != FT_Err_Ok) {
			Output::Error("Couldn't load FreeType character {:#x}", uint32_t(glyph));
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
	advance.x = Utils::RoundTo<int>(slot->advance.x / 64.0);
	advance.y = Utils::RoundTo<int>(slot->advance.y / 64.0);

	if (EP_UNLIKELY(rm2000_workaround)) {
		advance.x = 6;
	}

	return {0, 0, advance.x, advance.y};
}

Font::GlyphRet FTFont::vRender(char32_t glyph) const {
	auto glyph_index = FT_Get_Char_Index(face, glyph);

	if (glyph_index == 0 && fallback_font) {
		return fallback_font->vRender(glyph);
	}

	return vRenderShaped(glyph_index);
}

Font::GlyphRet FTFont::vRenderShaped(char32_t glyph) const {
	if (glyph == 0 && fallback_font) {
		return fallback_font->vRender(glyph);
	}

	auto render_glyph = [&](auto flags, auto mode) {
		if (FT_Load_Glyph(face, glyph, flags) != FT_Err_Ok) {
			Output::Error("Couldn't load FreeType character {:#x}", uint32_t(glyph));
		}

		if (FT_Render_Glyph(face->glyph, mode) != FT_Err_Ok) {
			Output::Error("Couldn't render FreeType character {:#x}", uint32_t(glyph));
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

	advance.x = Utils::RoundTo<int>(slot->advance.x / 64.0);
	advance.y = Utils::RoundTo<int>(slot->advance.y / 64.0);
	offset.x = slot->bitmap_left;
	offset.y = slot->bitmap_top - baseline_offset;

	if (EP_UNLIKELY(rm2000_workaround)) {
		advance.x = 6;
	}

	return { bm, advance, offset, has_color };
}

bool FTFont::vCanShape() const {
#ifdef HAVE_HARFBUZZ
	return FT_IS_SFNT(face);
#else
	return false;
#endif
}

#ifdef HAVE_HARFBUZZ
std::vector<Font::ShapeRet> FTFont::vShape(U32StringView txt) const {
	hb_buffer_clear_contents(hb_buffer);

	hb_buffer_add_utf32(hb_buffer, reinterpret_cast<const uint32_t*>(txt.data()), txt.size(), 0, txt.size());
	hb_buffer_guess_segment_properties(hb_buffer);

	hb_shape(hb_font, hb_buffer, nullptr, 0);

	unsigned int glyph_count;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

	std::vector<Font::ShapeRet> ret;
	Point advance;
	Point offset;

	for (unsigned int i = 0; i < glyph_count; ++i) {
		auto& info = glyph_info[i];
		auto& pos = glyph_pos[i];

		if (info.codepoint == 0) {
			auto s = vGetSize(txt[info.cluster]);
			advance.x = s.width;
			advance.y = s.height;
			ret.push_back({txt[info.cluster], advance, offset, true});
		} else {
			advance.x = Utils::RoundTo<int>(pos.x_advance / 64.0);
			advance.y = Utils::RoundTo<int>(pos.y_advance / 64.0);
			offset.x = Utils::RoundTo<int>(pos.x_offset / 64.0);
			offset.y = Utils::RoundTo<int>(pos.y_offset / 64.0);
			ret.push_back({static_cast<char32_t>(info.codepoint), advance, offset, false});
		}
	}

	return ret;
}
#endif

void FTFont::vApplyStyle(const Style& style) {
	if (current_style.size == style.size) {
		return;
	}

	SetSize(style.size, false);
}

void FTFont::SetSize(int height, bool create) {
	if (FT_HAS_COLOR(face)) {
		// FIXME: Find the best size
		FT_Select_Size(face, 0);
	} else if (FT_IS_SCALABLE(face)) {
		// Calculate the pt size from px
		auto table_os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2));
		auto table_hori = static_cast<TT_HoriHeader*>(FT_Get_Sfnt_Table(face, ft_sfnt_hhea));

		if (table_os2 && table_hori) {
			int units;
			if (table_os2->usWinAscent + table_os2->usWinDescent == 0) {
				units = table_hori->Ascender - table_hori->Descender;
			} else {
				units = table_os2->usWinAscent + table_os2->usWinDescent;
			}

			int pt = FT_MulDiv(face->units_per_EM, height, units);
			if (FT_MulDiv(units, pt, face->units_per_EM) > height) {
				--pt;
			}

			height = std::max<int>(1, pt);
		}

		FT_Set_Pixel_Sizes(face, 0, height);
	} else {
		FT_Set_Pixel_Sizes(face, 0, face->available_sizes->height);
	}

#ifdef HAVE_HARFBUZZ
	if (create) {
		hb_buffer = hb_buffer_create();
	} else {
		// Without this the sizes become desynchronized
		hb_font_destroy(hb_font);
	}
	hb_font = hb_ft_font_create_referenced(face);
	hb_ft_font_set_funcs(hb_font);
#endif

	baseline_offset = FT_MulFix(face->ascender, face->size->metrics.y_scale) / 64;
	if (baseline_offset == 0) {
		// FIXME: Becomes 0 for FON files. How is the baseline calculated for them?
		baseline_offset = static_cast<int>(height * (10.0 / 12.0));
	}
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
	if (!is) {
		return nullptr;
	}

	FreeFontMemory();

	std::string key = ToString(is.GetName()) + ":" + std::to_string(size) + ":" + (bold ? "T" : "F") + (italic ? "T" : "F");

	auto it = ft_cache.find(key);

	if (it == ft_cache.end()) {
		auto ft_font = std::make_shared<FTFont>(std::move(is), size, bold, italic);
		if (!ft_font) {
			return nullptr;
		}

		++cache_size;

		return (ft_cache[key] = {ft_font, Game_Clock::GetFrameTime()}).font;
	} else {
		it->second.last_access = Game_Clock::GetFrameTime();
		return it->second.font;
	}

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
{
	original_style.size = size;
	original_style.bold = bold;
	original_style.italic = italic;
	current_style = original_style;
}

Rect Font::GetSize(char32_t glyph) const {
	if (EP_UNLIKELY(Utils::IsControlCharacter(glyph))) {
		if (glyph == '\n') {
			return {0, 0, 0, static_cast<int>(current_style.size)};
		}

		return {};
	}

	Rect size = vGetSize(glyph);
	size.width += current_style.letter_spacing;
	size.height = current_style.size;

	return size;
}

Rect Font::GetSize(const ShapeRet& shape_ret) const {
	int width = shape_ret.advance.x + current_style.letter_spacing;
	int height = current_style.size;
	return {0, 0, width, height};
}

Point Font::Render(Bitmap& dest, int const x, int const y, const Bitmap& sys, int color, char32_t glyph) const {
	if (EP_UNLIKELY(Utils::IsControlCharacter(glyph))) {
		return {};
	}

	auto gret = vRender(glyph);

	auto rect = Rect(x, y, gret.bitmap->width(), gret.bitmap->height());
	if (EP_UNLIKELY(rect.width == 0)) {
		return {};
	}

	rect.x += gret.offset.x;
	rect.y -= gret.offset.y;

	unsigned src_x;
	unsigned src_y;

	if (color != ColorShadow) {
		if (!gret.has_color && current_style.draw_shadow) {
			auto shadow_rect = Rect(rect.x + 1, rect.y + 1, rect.width, rect.height);
			dest.MaskedBlit(shadow_rect, *gret.bitmap, 0, 0, sys, 16, 32);
		}

		src_x = color % 10 * 16 + 2;
		src_y = color / 10 * 16 + 48 + 16 - 12 - gret.offset.y;
	} else {
		src_x = 16;
		src_y = 32;
	}

	if (!gret.has_color) {
		if (current_style.draw_gradient) {
			// When the glyph is large the system graphic color mask will be outside the rectangle
			// Move the mask slightly up to avoid this
			int offset = gret.bitmap->height() - gret.offset.y;
			if (offset > 12) {
				src_y -= offset - 12;
			}

			dest.MaskedBlit(rect, *gret.bitmap, 0, 0, sys, src_x, src_y);
		} else {
			auto col = sys.GetColorAt(current_style.color_offset.x + src_x, current_style.color_offset.y + src_y);
			auto col_bm = Bitmap::Create(gret.bitmap->width(), gret.bitmap->height(), col);
			dest.MaskedBlit(rect, *gret.bitmap, 0, 0, *col_bm, 0, 0);
		}
	} else {
		dest.Blit(rect.x, rect.y, *gret.bitmap, gret.bitmap->GetRect(), Opacity::Opaque());
	}

	gret.advance.x += current_style.letter_spacing;

	return gret.advance;
}

Point Font::Render(Bitmap& dest, int const x, int const y, const Bitmap& sys, int color, const Font::ShapeRet& shape) const {
	if (shape.not_found) {
		return Render(dest, x, y, sys, color, shape.code);
	}

	auto gret = vRenderShaped(shape.code);

	auto rect = Rect(x, y, gret.bitmap->width(), gret.bitmap->height());
	if (EP_UNLIKELY(rect.width == 0)) {
		return {};
	}

	rect.x += shape.offset.x + gret.offset.x;
	rect.y -= shape.offset.y + gret.offset.y;

	unsigned src_x;
	unsigned src_y;

	if (color != ColorShadow) {
		if (!gret.has_color && current_style.draw_shadow) {
			auto shadow_rect = Rect(rect.x + 1, rect.y + 1, rect.width, rect.height);
			dest.MaskedBlit(shadow_rect, *gret.bitmap, 0, 0, sys, 16, 32);
		}

		src_x = color % 10 * 16 + 2;
		src_y = color / 10 * 16 + 48 + 16 - 12 - shape.offset.y - gret.offset.y;

		// When the glyph is large the system graphic color mask will be outside the rectangle
		// Move the mask slightly up to avoid this
		int offset = gret.bitmap->height() - shape.offset.y - gret.offset.y;
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

	Point advance = { shape.advance.x + current_style.letter_spacing, shape.advance.y };
	return advance;
}

Point Font::Render(Bitmap& dest, int x, int y, Color const& color, char32_t glyph) const {
	if (EP_UNLIKELY(Utils::IsControlCharacter(glyph))) {
		return {};
	}

	auto gret = vRender(glyph);

	auto rect = Rect(x, y, gret.bitmap->width(), gret.bitmap->height());
	dest.MaskedBlit(rect, *gret.bitmap, 0, 0, color);

	gret.advance.x += current_style.letter_spacing;

	return gret.advance;
}

bool Font::CanShape() const {
	return vCanShape();
}

std::vector<Font::ShapeRet> Font::Shape(U32StringView text) const {
	assert(vCanShape());

	return vShape(text);
}

void Font::SetFallbackFont(FontRef fallback_font) {
	this->fallback_font = fallback_font;
}

bool Font::IsStyleApplied() const {
	return style_applied;
}

Font::Style Font::GetCurrentStyle() const {
	return current_style;
}

Font::StyleScopeGuard Font::ApplyStyle(Style new_style) {
	vApplyStyle(new_style);
	current_style = new_style;
	style_applied = true;

	return lcf::ScopeGuard<std::function<void()>>([&]() {
		vApplyStyle(original_style);
		current_style = original_style;
		style_applied = false;
	});
}

ExFont::ExFont() : Font("exfont", HEIGHT, false, false) {
}

FontRef Font::exfont = std::make_shared<ExFont>();

Font::GlyphRet ExFont::vRender(char32_t glyph) const {
	if (EP_UNLIKELY(!bm)) { bm = Bitmap::Create(WIDTH, HEIGHT, true); }
	auto exfont = Cache::Exfont();

	bool is_lower = (glyph >= 'a' && glyph <= 'z');
	bool is_upper = (glyph >= 'A' && glyph <= 'Z');

	if (!is_lower && !is_upper) {
		// Invalid ExFont
		return { bm, {WIDTH, 0}, {0, 0}, false };
	}

	glyph = is_lower ? (glyph - 'a' + 26) : (glyph - 'A');

	Rect const rect((glyph % 13) * WIDTH, (glyph / 13) * HEIGHT, WIDTH, HEIGHT);
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

Rect ExFont::vGetSize(char32_t) const {
	return Rect(0, 0, WIDTH, HEIGHT);
}
