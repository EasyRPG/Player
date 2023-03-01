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

#ifndef EP_TEXT_H
#define EP_TEXT_H

#include "point.h"
#include "system.h"
#include "memory_management.h"
#include "rect.h"
#include "color.h"
#include "string_view.h"
#include <string>

class Font;
class Bitmap;

namespace Text {
	/** TextDraw alignment options. */
	enum Alignment {
		/** Align text to the left. */
		AlignLeft,
		/** Align text to the center. */
		AlignCenter,
		/** Align text to the right. */
		AlignRight
	};

	/**
	 * Draws the text onto dest bitmap with given parameters.
	 *
	 * @param dest the bitmap to render to.
	 * @param x X offset to render text.
	 * @param y Y offset to render text.
	 * @param font the font used to render.
	 * @param system the system graphic to use to render.
	 * @param color which color from the system graphic to use.
	 * @param text the utf8 / exfont text to render.
	 * @param align the text alignment to use
	 *
	 * @return Where to draw the next glyph when continuing drawing. See Font::GlyphRet.advance
	 */
	Point Draw(Bitmap& dest, int x, int y, const Font& font, const Bitmap& system, int color, StringView text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws the text onto dest bitmap with given parameters. Does not draw a shadow.
	 *
	 * @param dest the bitmap to render to.
	 * @param x X offset to render text.
	 * @param y Y offset to render text.
	 * @param font the font used to render.
	 * @param color which color to use.
	 * @param text the utf8 / exfont text to render.
	 *
	 * @return Where to draw the next glyph when continuing drawing. See Font::GlyphRet.advance
	 */
	Point Draw(Bitmap& dest, int x, int y, const Font& font, Color color, StringView text);

	/**
	 * Draws the character onto dest bitmap with given parameters.
	 *
	 * @param dest the bitmap to render to.
	 * @param x X offset to render text.
	 * @param y Y offset to render text.
	 * @param font the font used to render.
	 * @param system the system graphic to use to render.
	 * @param color which color from the system graphic to use.
	 * @param ch the character to render.
	 * @param is_exfont if true, treat ch as an exfont character. Otherwise, a utf32 character.
	 *
	 * @return Where to draw the next glyph when continuing drawing. See Font::GlyphRet.advance
	 */
	Point Draw(Bitmap& dest, int x, int y, const Font& font, const Bitmap& system, int color, char32_t glyph, bool is_exfont);


	/**
	 * Draws the character onto dest bitmap with given parameters. Does not draw a shadow.
	 *
	 * @param dest the bitmap to render to.
	 * @param x X offset to render text.
	 * @param y Y offset to render text.
	 * @param font the font used to render.
	 * @param color which color to use.
	 * @param ch the character to render.
	 * @param is_exfont if true, treat ch as an exfont character. Otherwise, a utf32 character.
	 *
	 * @return Where to draw the next glyph when continuing drawing. See Font::GlyphRet.advance
	 */
	Point Draw(Bitmap& dest, int x, int y, const Font& font, Color color, char32_t glyph, bool is_exfont);

	/**
	 * Determines the size of a bitmap required to render a string.
	 * The dimensions of the Rect describe a bounding box to fit the text.
	 * For continuous rendering use the "width" property.
	 *
	 * @param font the font used to render.
	 * @param text the string to measure.
	 *
	 * @return Rect describing the rendered string boundary
	 */
	Rect GetSize(const Font& font, StringView text);

	/**
	 * Determines the size of a bitmap required to render a single character.
	 * The dimensions of the Rect describe a bounding box to fit the character.
	 * For continuous rendering use the "width" property.
	 *
	 * @param font the font used to render.
	 * @param glyph the character to mesaure.
	 * @param is_exfont if true, treat ch as an exfont character. Otherwise, a utf32 character.
	 *
	 * @return Rect describing the rendered string boundary
	 */
	Rect GetSize(const Font& font, char32_t glyph, bool is_exfont);
}
#endif
