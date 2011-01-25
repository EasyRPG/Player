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

#ifndef _PIXMAN_BITMAP_H_
#define _PIXMAN_BITMAP_H_

#include "system.h"
#ifdef USE_PIXMAN_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <cstdio>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <pixman.h>
// for SDL_BYTEORDER
#include <SDL.h>

#include "bitmap.h"
#include "surface.h"

////////////////////////////////////////////////////////////
/// PixmanBitmap class.
////////////////////////////////////////////////////////////
class PixmanBitmap : public Surface {
public:
	PixmanBitmap(int width, int height, bool transparent);
	PixmanBitmap(const std::string filename, bool transparent, uint32 flags);
	PixmanBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags);
	PixmanBitmap(Bitmap* source, Rect src_rect, bool transparent);
	~PixmanBitmap();

	void Blit(int x, int y, Bitmap* _src, Rect src_rect, int opacity);
	void TiledBlit(Rect src_rect, Bitmap* _src, Rect dst_rect, int opacity);
	void TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);
	void StretchBlit(Bitmap* src, Rect src_rect, int opacity);
	void StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);
	void Mask(int x, int y, Bitmap* _src, Rect src_rect);
	void Fill(const Color &color);
	void FillRect(Rect dst_rect, const Color &color);
	void Clear();
	void ClearRect(Rect dst_rect);

	void ToneChange(const Tone &tone);
	void Flip(bool horizontal, bool vertical);
	Bitmap* Resample(int scale_w, int scale_h, const Rect& src_rect);
	Bitmap* RotateScale(double angle, int scale_w, int scale_h);
	void OpacityChange(int opacity, const Rect& dst_rect);
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
	friend class PixmanBitmapScreen;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	static const unsigned int AMASK = 0xFF000000;
	static const unsigned int RMASK = 0x00FF0000;
	static const unsigned int GMASK = 0x0000FF00;
	static const unsigned int BMASK = 0x000000FF;
	static const pixman_format_code_t format = PIXMAN_a8r8g8b8;
#else
	static const unsigned int BMASK = 0xFF000000;
	static const unsigned int GMASK = 0x00FF0000;
	static const unsigned int RMASK = 0x0000FF00;
	static const unsigned int AMASK = 0x000000FF;
	static const pixman_format_code_t format = PIXMAN_b8g8r8a8;
#endif

	/// Bitmap data.
	pixman_image_t *bitmap;

	void Init(int width, int height, void* data);

	Color GetColor(uint32 color) const;
	uint32 GetUint32Color(const Color &color) const;
	uint32 GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const;
	void GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const;

	void Lock();
	void Unlock();

	void ReadPNG(FILE* stream, const void *data);
	void ReadXYZ(const uint8 *data, uint len);
	void ReadXYZ(FILE *stream);

	static pixman_image_t* GetSubimage(Bitmap* _src, const Rect& src_rect);
};

#endif

#endif
