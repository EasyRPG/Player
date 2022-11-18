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

#ifndef EP_FONT_H
#define EP_FONT_H

// Headers
#include "filesystem_stream.h"
#include "point.h"
#include "system.h"
#include "memory_management.h"
#include "rect.h"
#include "string_view.h"
#include <string>

class Color;
class Rect;

/**
 * Font class.
 */
class Font {
 public:
	virtual ~Font() = default;

	/**
	 * Returns the size of the rendered string, not including shadows.
	 *
	 * @param txt the string to measure
	 * @return Rect describing the rendered string boundary
	 */
	virtual Rect GetSize(StringView txt) const = 0;
	/**
	 * Returns the size of the rendered utf32 character, not including shadows.
	 *
	 * @param ch the character to measure
	 * @return Rect describing the rendered character boundary
	 */
	virtual Rect GetSize(char32_t ch) const = 0;

	struct GlyphRet {
		/** bitmap which the glyph pixels are located within */
		BitmapRef bitmap;
		/**
		 * How far to advance the x/y offset after drawing for the next glyph.
		 * y value is only relevant for vertical layouts.
		 */
		Point advance;
		/** x/y position in the buffer where the glyph is rendered at */
		Point offset;
		/** When enabled the glyph is colored and not masked with the system graphic */
		bool has_color = false;
	};

	/* Returns a bitmap and rect containing the pixels of the glyph.
	 * The bitmap may be larger than the size of the glyph, and so the
	 * rect must be used to get the pixels out of the bitmap
	 *
	 * @param code which utf32 glyph to return.
	 * @return @refer GlyphRet
	 */
	virtual GlyphRet Glyph(char32_t code) = 0;

	/**
	 * Renders the glyph onto bitmap at the given position with system graphic and color
	 *
	 * @param dest the bitmap to render to
	 * @param x X offset to render glyph
	 * @param y Y offset to render glyph
	 * @param sys system graphic to use
	 * @param color which color in the system graphic
	 * @param glyph which utf32 glyph to render
	 *
	 * @return Point containing how far to advance in x/y direction.
	 */
	Point Render(Bitmap& dest, int x, int y, const Bitmap& sys, int color, char32_t glyph);

	/**
	 * Renders the glyph onto bitmap at the given position with system graphic and color
	 *
	 * @param dest the bitmap to render to
	 * @param x X offset to render glyph
	 * @param y Y offset to render glyph
	 * @param color which color in the system graphic
	 * @param glyph which utf32 glyph to render
	 *
	 * @return Point containing how far to advance in x/y direction.
	 */
	Point Render(Bitmap& dest, int x, int y, Color const& color, char32_t glyph);

	/**
	 * Defines a fallback font that shall be used when a glyph is not found in the current font.
	 * Currently only used by FreeType Fonts.
	 *
	 * @param fallback_font Font to fallback to
	 */
	void SetFallbackFont(FontRef fallback_font);

	static FontRef CreateFtFont(Filesystem_Stream::InputStream is, int size, bool bold, bool italic);
	static FontRef Default();
	static FontRef Default(bool use_mincho);
	static FontRef DefaultBitmapFont();
	static FontRef DefaultBitmapFont(bool use_mincho);
	static void SetDefault(FontRef new_default, bool use_mincho);
	static void ResetDefault();
	static void Dispose();

	static FontRef exfont;

	enum SystemColor {
		ColorShadow = -1,
		ColorDefault = 0,
		ColorDisabled = 3,
		ColorCritical = 4,
		ColorKnockout = 5,
		ColorHeal = 9
	};

	std::string name;
	unsigned size;
	bool bold;
	bool italic;

	size_t pixel_size() const { return size * 96 / 72; }
 protected:
	Font(StringView name, int size, bool bold, bool italic);

	FontRef fallback_font;
};

#endif
