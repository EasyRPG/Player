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

#ifndef _EASYRPG_BITMAP_UTILS_H_
#define _EASYRPG_BITMAP_UTILS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include "pixel_format.h"
#include "color.h"
#include "rect.h"
#include "tone.h"

class Bitmap;

////////////////////////////////////////////////////////////
/// Bitmap utils abstract parent class
////////////////////////////////////////////////////////////

class BitmapUtils {
public:
	BitmapUtils(const DynamicFormat& format) :
		format(format) {}

	virtual Color GetPixel(Bitmap* src, int x, int y) = 0;
	virtual void SetPixel(Surface* dst, int x, int y, const Color &color) = 0;
	virtual Bitmap* Resample(Bitmap* src, int scale_w, int scale_h, const Rect& src_rect) = 0;
	virtual Bitmap* RotateScale(Bitmap* src, double angle, int scale_w, int scale_h) = 0;
	virtual Bitmap* Waver(Bitmap* src, int depth, double phase) = 0;
	virtual void Blit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int opacity) = 0;
	virtual void TiledBlit(Surface* dst, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) = 0;
	virtual void TiledBlit(Surface* dst, int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) = 0;
	virtual void StretchBlit(Surface* dst, Bitmap* src, Rect src_rect, int opacity) = 0;
	virtual void StretchBlit(Surface* dst, Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) = 0;
	virtual void Mask(Surface* dst, int x, int y, Bitmap* src, Rect src_rect) = 0;
	virtual void Fill(Surface* dst, const Color &color) = 0;
	virtual void FillRect(Surface* dst, Rect dst_rect, const Color &color) = 0;
	virtual void Clear(Surface* dst) = 0;
	virtual void ClearRect(Surface* dst, Rect dst_rect) = 0;
	virtual void HueChange(Surface* dst, double hue) = 0;
	virtual void HSLChange(Surface* dst, double hue, double sat, double lum, double loff, Rect dst_rect) = 0;
	virtual void ToneChange(Surface* dst, const Tone &tone) = 0;
	virtual void Flip(Surface* dst, bool horizontal, bool vertical) = 0;
	virtual void OpacityChange(Surface* dst, int opacity, const Rect &src_rect) = 0;

	virtual void SetColorKey(int colorkey);

	static BitmapUtils* Create(int bpp, bool has_alpha, bool has_colorkey,
							   const DynamicFormat& format);
	DynamicFormat format;
};

////////////////////////////////////////////////////////////
/// Bitmap utils class template
////////////////////////////////////////////////////////////
template <class PF>
class BitmapUtilsT : public BitmapUtils {
public:
	BitmapUtilsT(const DynamicFormat& format) :
		BitmapUtils(format) {}

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @return pixel color
	////////////////////////////////////////////////////////
	Color GetPixel(Bitmap* src, int x, int y);

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @param color : pixel color
	////////////////////////////////////////////////////////
	void SetPixel(Surface* dst, int x, int y, const Color &color);

	////////////////////////////////////////////////////////
	/// Create a resampled bitmap.
	/// @param scale_w : resampled width
	/// @param scale_h : resampled height
	/// @param src_rect : source rect to resample
	////////////////////////////////////////////////////////
	Bitmap* Resample(Bitmap* src, int scale_w, int scale_h, const Rect& src_rect);

	////////////////////////////////////////////////////////
	/// Create a rotated and scaled bitmap.
	/// @param angle : rotation angle
	/// @param scale_w : resampled width
	/// @param scale_h : resampled height
	////////////////////////////////////////////////////////
	Bitmap* RotateScale(Bitmap* src, double angle, int scale_w, int scale_h);

	////////////////////////////////////////////////////////
	/// Create a wavy bitmap.
	/// @param depth : wave magnitude
	/// @param phase : wave phase
	////////////////////////////////////////////////////////
	Bitmap* Waver(Bitmap* src, int depth, double phase);

	////////////////////////////////////////////////////////
	/// Blit source bitmap to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	void Blit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	void TiledBlit(Surface* dst, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param ox : tile start x offset
	/// @param ox : tile start y offset
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	void TiledBlit(Surface* dst, int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	void StretchBlit(Surface* dst, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	void StretchBlit(Surface* dst, Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);

	/// Blit source bitmap transparency to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	////////////////////////////////////////////////////////
	void Mask(Surface* dst, int x, int y, Bitmap* src, Rect src_rect);

	////////////////////////////////////////////////////////
	/// Fill entire bitmap with color.
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	void Fill(Surface* dst, const Color &color);

	////////////////////////////////////////////////////////
	/// Fill bitmap rect with color.
	/// @param dst_rect : destination rect
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	void FillRect(Surface* dst, Rect dst_rect, const Color &color);

	////////////////////////////////////////////////////////
	/// Clears the bitmap with transparent pixels.
	////////////////////////////////////////////////////////
	void Clear(Surface* dst);

	////////////////////////////////////////////////////////
	/// Clears the bitmap rect with transparent pixels.
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	void ClearRect(Surface* dst, Rect dst_rect);
	
	////////////////////////////////////////////////////////
	/// Rotate bitmap hue.
	/// @param hue : hue change, degrees
	////////////////////////////////////////////////////////
	void HueChange(Surface* dst, double hue);

	////////////////////////////////////////////////////////
	/// Adjust bitmap HSL colors.
	/// @param hue : hue change, degrees
	/// @param sat : saturation scale
	/// @param lum : luminance scale
	/// @param loff: luminance offset
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	void HSLChange(Surface* dst, double hue, double sat, double lum, double loff, Rect dst_rect);

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone.
	/// @param tone : tone to apply
	////////////////////////////////////////////////////////
	void ToneChange(Surface* dst, const Tone &tone);

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels.
	/// @param horizontal : flip horizontally (mirror)
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	void Flip(Surface* dst, bool horizontal, bool vertical);

	////////////////////////////////////////////////////////
	/// Change the opacity of a bitmap.
	/// @param opacity : the maximum opacity
	/// @param src_rect: the rectangle to modify
	////////////////////////////////////////////////////////
	void OpacityChange(Surface* dst, int opacity, const Rect &src_rect);
};

#endif
