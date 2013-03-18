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

	virtual Rect GetSize(std::string const& txt) const = 0;

	virtual void Render(Bitmap& bmp, int x, int y, Bitmap const& sys, int color, unsigned glyph) = 0;
	virtual void Render(Bitmap& bmp, int x, int y, Color const& color, unsigned glyph) = 0;

	static FontRef Create(const std::string& name, int size, bool bold, bool italic);
	static FontRef Default(bool mincho = false);
	static void Dispose();

	static const int default_size = 9;
	static const bool default_bold = false;
	static const bool default_italic = false;

	enum SystemColor {
		ColorShadow = -1,
		ColorDefault = 0,
		ColorDisabled = 3,
		ColorCritical = 4,
		ColorKnockout = 5
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
