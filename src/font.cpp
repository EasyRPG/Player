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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "system.h"
#include "filefinder.h"
#include "output.h"
#include "font.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
/// Static Variables
////////////////////////////////////////////////////////////
static std::vector<EASYRPG_WEAK_PTR<Font> > fonts;

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Font::Font(const std::string& name, int size, bool bold, bool italic):
	name(name),
	size(size),
	bold(bold),
	italic(italic),
	ft_face_initialized(false) {
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Font::~Font() {
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
/// Class Exists
////////////////////////////////////////////////////////////
bool Font::Exists(const std::string& name) {
	return FileFinder::FindFont(name) != "";
}

////////////////////////////////////////////////////////////
/// Factory
////////////////////////////////////////////////////////////
FontRef Font::CreateFont(const std::string& _name, int size, bool bold, bool italic) {
	const std::string& name = _name.empty() ? FileFinder::DefaultFont() : _name;
	if (size == 0)
		size = default_size;

	std::vector<EASYRPG_WEAK_PTR<Font> >::const_iterator it;
	for (it = fonts.begin(); it != fonts.end(); it++) {
		if(it->expired()) { continue; }

		FontRef font = it->lock();
		if (font->name == name && font->size == size &&
			font->bold == bold && font->italic == italic)
			return font;
	}

	FontRef font(new Font(name, size, bold, italic));
	fonts.push_back(font);
	return font;
}

////////////////////////////////////////////////////////////
/// Cleanup
////////////////////////////////////////////////////////////
void Font::Dispose() {
	fonts.clear();
}

FT_Library Font::library;
int Font::ft_lib_refcount = 0;

////////////////////////////////////////////////////////////
/// Initialization
////////////////////////////////////////////////////////////
void Font::Init() {
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
		Output::Error("Couldn't initialize FreeType face: %s(%s)\n", name.c_str(), path.c_str());
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
int Font::GetHeight() {
	return size + 2;
}

BitmapRef Font::Render(int c) {
	Init();

	FT_Error ans = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't load FreeType character %d\n", c);
		return BitmapRef();
	}

	ans = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't render FreeType character %d\n", c);
		return BitmapRef();
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
		return BitmapRef();
	}

	int pitch = ft_bitmap.pitch;
	if (pitch < 0)
		pitch = -pitch;

    BitmapRef source = Bitmap::Create(
		ft_bitmap.buffer, ft_bitmap.width, ft_bitmap.rows, pitch, format_L8_k().format());
	source->SetTransparentColor(Color(0,0,0,0));

    BitmapRef bitmap = Bitmap::Create(ft_bitmap.width, size + 2, true);
	bitmap->SetTransparentColor(Color(0,0,0,0));

	const int base_line = (face->descender != 0)
		? bitmap->height() * -face->descender / face->height
		: 0;
	int offset = bitmap->height() - face->glyph->bitmap_top - base_line;

	Rect rect = source->GetRect();
	rect.y += offset;
	bitmap->FillRect(rect, Color(255,255,255,255));

	bitmap->MaskBlit(0, offset, *source, source->GetRect());
	if (ft_bitmap.pitch < 0)
		bitmap->Flip(bitmap->GetRect(), false, true);

	return bitmap;
}
