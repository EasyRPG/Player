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

#ifndef _FONT_H_
#define _FONT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "system.h"

#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

////////////////////////////////////////////////////////////
/// Font class
////////////////////////////////////////////////////////////
class Font {
public:
	Font(const std::string& name, int size, bool bold, bool italic);
	~Font();

	int GetHeight();
	BitmapRef Render(int glyph);

	static FontRef CreateFont(const std::string& name = "", int size = 0, bool bold = false, bool italic = false);
	static bool Exists(const std::string& name);
	static void Dispose();

	static const int default_size = 9;
	static const bool default_bold = false;
	static const bool default_italic = false;

	enum SystemColor {
		ColorDefault = 0,
		ColorDisabled = 3,
		ColorCritical = 4,
		ColorKnockout = 5
	};

	const std::string name;
	const int size;
	const bool bold;
	const bool italic;
 private:
	static FT_Library library;
	static int ft_lib_refcount;
	FT_Face face;
	bool ft_face_initialized;

	void Init();
};

#endif
