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

#ifndef _FONT_H_
#define _FONT_H_

// Headers
#include "system.h"
#include <string>

class Color;
class Rect;

/**
 * Font class.
 */
class Font {
 public:
	virtual ~Font() {}

	Rect GetSize(std::string const& txt) const;
	virtual Rect GetSize(std::u32string const& txt) const = 0;

	virtual BitmapRef Glyph(char32_t code) = 0;

	void Render(Bitmap& bmp, int x, int y, Bitmap const& sys, int color, char32_t glyph);
	void Render(Bitmap& bmp, int x, int y, Color const& color, char32_t glyph);

	static FontRef Create(const std::string& name, int size, bool bold, bool italic);
	static FontRef Default();
	static FontRef Default(bool mincho);
	static void Dispose();

	static FontRef exfont;

	static const int default_size = 9;
	static const bool default_bold = false;
	static const bool default_italic = false;

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
	Font(const std::string& name, int size, bool bold, bool italic);
};

#endif
