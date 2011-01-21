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

#ifndef _SOFT_BITMAP_H_
#define _SOFT_BITMAP_H_

#include "system.h"
#ifdef USE_SOFT_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <cstdio>
#include <ft2build.h>
#include FT_FREETYPE_H
// for SDL_BYTEORDER
#include "SDL.h"

#include "bitmap.h"

////////////////////////////////////////////////////////////
/// SoftBitmap class.
////////////////////////////////////////////////////////////
class SoftBitmap : public Bitmap {
public:
	SoftBitmap(int width, int height, bool transparent);
	SoftBitmap(const std::string filename, bool transparent);
	SoftBitmap(const uint8* data, uint bytes, bool transparent);
	SoftBitmap(Bitmap* source, Rect src_rect, bool transparent);
	~SoftBitmap();

	void Blit(int x, int y, Bitmap* _src, Rect src_rect, int opacity);
	void Mask(int x, int y, Bitmap* _src, Rect src_rect);
	void TextDraw(int x, int y, std::string text, TextAlignment align = Bitmap::TextAlignLeft);
	void SetTransparentColor(Color color);

	void* pixels();
	int width() const;
	int height() const;
	uint8 bpp() const;
	uint16 pitch() const;
	uint32 rmask() const;
	uint32 gmask() const;
	uint32 bmask() const;
	uint32 amask() const;
	uint32 colorkey() const;

protected:
	friend class SoftBitmapScreen;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	static const unsigned int RMASK = 0x0000FF00;
	static const unsigned int GMASK = 0x00FF0000;
	static const unsigned int BMASK = 0xFF000000;
	static const unsigned int AMASK = 0x000000FF;
#else
	static const unsigned int RMASK = 0x00FF0000;
	static const unsigned int GMASK = 0x0000FF00;
	static const unsigned int BMASK = 0x000000FF;
	static const unsigned int AMASK = 0xFF000000;
#endif

	/// Bitmap data.
	int w, h;
	void* bitmap;

	void Init(int width, int height);

	Color GetColor(uint32 color) const;
	uint32 GetUint32Color(const Color &color) const;
	uint32 GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const;
	void GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const;

	void Lock();
	void Unlock();

	static FT_Library library;
	static FT_Face face;
	static bool ft_initialized;

	void InitFreeType();
	SoftBitmap* RenderFreeTypeChar(int c);
	void DoneFreeType();

	void ReadPNG(FILE* stream, const void *data);
};

#endif

#endif
