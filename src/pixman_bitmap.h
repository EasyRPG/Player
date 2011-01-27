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

#include "bitmap.h"
#include "surface.h"
#include "pixel_format.h"

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
	Bitmap* Waver(int depth, double phase);
	void OpacityChange(int opacity, const Rect& dst_rect);
	void SetTransparentColor(Color color);

#ifndef USE_BIG_ENDIAN
	typedef PixelFormat<32,false,true,false,true,8,16,8,8,8,0,8,24> pixel_format;
	typedef PixelFormat<32,false,true,false,true,8,0,8,8,8,16,8,24> image_format;
	static const pixman_format_code_t pixman_format = PIXMAN_a8r8g8b8;
#else
	typedef PixelFormat<32,false,true,false,true,8,8,8,16,8,24,8,0> pixel_format;
	typedef PixelFormat<32,false,true,false,true,8,24,8,16,8,8,8,0> image_format;
	static const pixman_format_code_t pixman_format = PIXMAN_b8g8r8a8;
#endif

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
	void ConvertImage(int& width, int& height, void*& pixels);

	static pixman_image_t* GetSubimage(Bitmap* _src, const Rect& src_rect);
	static inline void MultiplyAlpha(uint8 &r, uint8 &g, uint8 &b, const uint8 &a) {
		r = (uint8)((int)r * a / 0xFF);
		g = (uint8)((int)g * a / 0xFF);
		b = (uint8)((int)b * a / 0xFF);
	}
	static inline void DivideAlpha(uint8 &r, uint8 &g, uint8 &b, const uint8 &a) {
		if (a == 0)
			r = g = b = 0;
		else {
			r = (uint8)((int)r * 0xFF / a);
			g = (uint8)((int)g * 0xFF / a);
			b = (uint8)((int)b * 0xFF / a);
		}
	}
};

#endif

#endif
