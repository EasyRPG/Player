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
#ifndef USE_SDL_TTF

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
#include "surface.h"
#include "ftfont.h"

FT_Library FTFont::library;
int FTFont::ft_lib_refcount = 0;

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
FTFont::FTFont(std::string name, int size, bool bold, bool italic)
	: Font(name, size, bold, italic), ft_face_initialized(false) {}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
FTFont::~FTFont() {
	if (ft_face_initialized) {
		FT_Done_Face(face);
		ft_face_initialized = false;

		if (ft_lib_refcount > 0)
			ft_lib_refcount--;
		if (ft_lib_refcount == 0)
			FT_Done_FreeType(library);
	}
}

////////////////////////////////////////////////////////////
/// Initialization
////////////////////////////////////////////////////////////
void FTFont::Init() {
	if (ft_face_initialized)
		return;

	if (ft_lib_refcount == 0) {
		FT_Error ans = FT_Init_FreeType(&library);
		if (ans != FT_Err_Ok) {
			Output::Error("Couldn't initialize FreeType\n");
			return;
		}
	}

	std::string path = FileFinder::FindFont(name);
    FT_Error ans = FT_New_Face(library, path.c_str(), 0, &face);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't initialize FreeType face\n");
		FT_Done_FreeType(library);
		return;
	}

	for (int i = 0; i < face->num_fixed_sizes; i++) {
		FT_Bitmap_Size* size = &face->available_sizes[i];
		Output::Debug("Font Size %d: %d %d %f %f %f", i,
					  size->width, size->height, size->size / 64.0,
					  size->x_ppem / 64.0, size->y_ppem / 64.0);
	}

	// RM2000.FON hack
	int sz, dpi;
	if (face->num_fixed_sizes == 1) {
		sz = face->available_sizes[0].size;
		dpi = 96;
	}
	else {
		sz = size * 64;
		dpi = 72;
	}

    ans = FT_Set_Char_Size(face, sz, sz, dpi, dpi);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't set FreeType face size\n");
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		return;
    }

	ft_lib_refcount++;
	ft_face_initialized = true;
}

////////////////////////////////////////////////////////////
/// Public methods
////////////////////////////////////////////////////////////
int FTFont::GetHeight() {
	return size + 2;
}

Bitmap* FTFont::Render(int c) {
	Init();

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

	Surface* bitmap = Surface::CreateSurface(ft_bitmap.width, size + 2, true);
	uint8* dst = (uint8*) bitmap->pixels();

	const int base_line = (face->descender != 0)
		? bitmap->height() * -face->descender / face->height
		: 0;
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

