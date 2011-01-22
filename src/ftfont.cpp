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

#include "system.h"
#if defined(USE_SOFT_BITMAP) || defined(USE_PIXMAN_BITMAP)

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include "output.h"
#include "filefinder.h"
#include "font.h"
#include "bitmap.h"
#include "ftfont.h"

static FT_Library library;
static FT_Face face;
static bool ft_initialized = false;

void FreeType::Init(const Font* font) {
	if (ft_initialized)
		return;

    FT_Error ans = FT_Init_FreeType(&library);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't initialize FreeType\n");
		return;
	}

	std::string path = FileFinder::FindFont(font->name);
    ans = FT_New_Face(library, path.c_str(), 0, &face);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't initialize FreeType face\n");
		FT_Done_FreeType(library);
		return;
	}

    ans = FT_Set_Char_Size(face, font->size * 64, font->size * 64, 72, 72);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't set FreeType face size\n");
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		return;
    }

	ft_initialized = true;
}

void FreeType::Done() {
	if (!ft_initialized)
		return;

    FT_Done_Face(face);

    FT_Done_FreeType(library);

	ft_initialized = false;
}

Bitmap* FreeType::RenderChar(const Font* font, int c) {
	FT_Error ans = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't load FreeType character %d\n", c);
		return NULL;
	}

	ans = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't render FreeType character %d\n", c);
		return NULL;
	}

	FT_Bitmap ft_bitmap;
	if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
		ft_bitmap = face->glyph->bitmap;
	else {
		FT_Bitmap_New(&ft_bitmap);
		FT_Bitmap_Convert(library, &face->glyph->bitmap, &ft_bitmap, 4);
	}

	if (ft_bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
		Output::Error("FreeType character has wrong format\n", c);
		return NULL;
	}

	const uint8* src = (const uint8*) ft_bitmap.buffer;
	if (ft_bitmap.pitch < 0)
		src -= ft_bitmap.rows * ft_bitmap.pitch;

	Bitmap* bitmap = Bitmap::CreateBitmap(ft_bitmap.width, font->size + 2, true);
	uint8* dst = (uint8*) bitmap->pixels();

	const int base_line = bitmap->height() / 4;
	int offset = bitmap->height() - face->glyph->bitmap_top - base_line;

	uint32 fg = bitmap->GetUint32Color(0, 0, 0, 255);
	uint32 bg = bitmap->GetUint32Color(0, 0, 0, 0);
	for (int yd = 0; yd < bitmap->height(); yd++) {
		int ys = yd - offset;
		if (ys < 0 || ys >= ft_bitmap.rows)
			continue;
		const uint8* p = src + ys * ft_bitmap.pitch;
		uint32* q = (uint32*) (dst + yd * bitmap->pitch());
		for (int x = 0; x < ft_bitmap.width; x++)
			*q++ = (*p++ != 0) ? fg : bg;
	}

	return bitmap;
}

#endif

