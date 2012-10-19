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
#include "bitmap_utils.h"

////////////////////////////////////////////////////////////
/// PixmanBitmap class.
////////////////////////////////////////////////////////////
class PixmanBitmap : public Surface {
public:
	PixmanBitmap(int width, int height, bool transparent);
	PixmanBitmap(const std::string& filename, bool transparent, uint32 flags);
	PixmanBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags);
	PixmanBitmap(Bitmap* source, Rect src_rect, bool transparent);
	PixmanBitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& format);
	~PixmanBitmap();

	static DynamicFormat ChooseFormat(const DynamicFormat& format);
	static void SetFormat(const DynamicFormat& format);

	Bitmap* Resample(int scale_w, int scale_h, const Rect& src_rect);

	void Blit(int x, int y, Bitmap* _src, Rect src_rect, int opacity);
	void TiledBlit(Rect src_rect, Bitmap* _src, Rect dst_rect, int opacity);
	void TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);
	void StretchBlit(Bitmap* src, Rect src_rect, int opacity);
	void StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);
	void TransformBlit(Rect dst_rect, Bitmap* src, Rect src_rect, const Matrix& inv, int opacity);
	void MaskBlit(int x, int y, Bitmap* src, Rect src_rect);
	void WaverBlit(int x, int y, Bitmap* src, Rect src_rect, int depth, double phase, int opacity);
	void Fill(const Color &color);
	void FillRect(Rect dst_rect, const Color &color);
	void Clear();
	void ClearRect(Rect dst_rect);
	void OpacityBlit(int x, int y, Bitmap* src, Rect src_rect, int opacity);
	void ToneBlit(int x, int y, Bitmap* src, Rect src_rect, const Tone &tone);
	void BlendBlit(int x, int y, Bitmap* _src, Rect src_rect, const Color& color);
	void FlipBlit(int x, int y, Bitmap* src, Rect src_rect, bool horizontal, bool vertical);
	void Flip(const Rect& dst_rect, bool horizontal, bool vertical);
	void Blit2x(Rect dst_rect, Bitmap* _src, Rect src_rect);

	static DynamicFormat pixel_format;
	static DynamicFormat opaque_pixel_format;
	static DynamicFormat image_format;
	static DynamicFormat opaque_image_format;

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
	pixman_format_code_t pixman_format;

	void Init(int width, int height, void* data, int pitch = 0, bool destroy = true);

	Color GetColor(uint32 color) const;
	uint32 GetUint32Color(const Color &color) const;
	uint32 GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const;
	void GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const;

	void Lock();
	void Unlock();

	void ReadPNG(FILE* stream, const void *data);
	void ReadXYZ(const uint8 *data, uint len);
	void ReadXYZ(FILE *stream);
	void ConvertImage(int& width, int& height, void*& pixels, bool transparent);

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

	typedef std::pair<int, pixman_format_code_t> format_pair;
	static std::map<int, pixman_format_code_t> formats_map;
	static bool formats_initialized;

	static void initialize_formats();
	static void add_pair(pixman_format_code_t pcode, const DynamicFormat& format);
	static pixman_format_code_t find_format(const DynamicFormat& format);
};

#endif

#endif
