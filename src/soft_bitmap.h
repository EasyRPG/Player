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

#include "surface.h"
#include "pixel_format.h"

////////////////////////////////////////////////////////////
/// SoftBitmap class.
////////////////////////////////////////////////////////////
class SoftBitmap : public Surface {
public:
	SoftBitmap(int width, int height, bool transparent);
	SoftBitmap(const std::string& filename, bool transparent, uint32 flags);
	SoftBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags);
	SoftBitmap(Bitmap* source, Rect src_rect, bool transparent);
	~SoftBitmap();

	Color GetPixel(int x, int y);
	void SetPixel(int x, int y, const Color &color);
	void Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity);
	void TiledBlit(Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);
	void TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);
	void StretchBlit(Bitmap* src, Rect src_rect, int opacity);
	void StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);
	void Mask(int x, int y, Bitmap* src, Rect src_rect);
	void Fill(const Color &color);
	void FillRect(Rect dst_rect, const Color &color);
	void Clear();
	void ClearRect(Rect dst_rect);
	void HueChange(double hue);
	void HSLChange(double hue, double sat, double lum, double loff, Rect dst_rect);
	void ToneChange(const Tone &tone);
	void Flip(bool horizontal, bool vertical);
	void OpacityChange(int opacity, const Rect &src_rect);
	Bitmap* Resample(int scale_w, int scale_h, const Rect& src_rect);
	Bitmap* RotateScale(double angle, int scale_w, int scale_h);
	Bitmap* Waver(int depth, double phase);

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

#ifndef USE_BIG_ENDIAN
	typedef PixelFormat<32,false,true,false,true,8,16,8,8,8,0,8,24> pixel_format;
	typedef PixelFormat<32,false,true,false,true,8,0,8,8,8,16,8,24> image_format;
#else
	typedef PixelFormat<32,false,true,false,true,8,8,8,16,8,24,8,0> pixel_format;
	typedef PixelFormat<32,false,true,false,true,8,24,8,16,8,8,8,0> image_format;
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

	void ConvertImage(int& width, int& height, void*& pixels);
};

#endif

#endif
