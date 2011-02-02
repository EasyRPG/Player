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
#include "matrix.h"

class Bitmap;

////////////////////////////////////////////////////////////
/// Bitmap utils abstract parent class
////////////////////////////////////////////////////////////

class BitmapUtils {
public:
	BitmapUtils(const DynamicFormat& format) :
		format(format) {}

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param src : the source bitmap
	/// @param x : pixel x
	/// @param y : pixel y
	/// @return pixel color
	////////////////////////////////////////////////////////
	virtual Color GetPixel(Bitmap* src, int x, int y) = 0;

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param src : the source bitmap
	/// @param x : pixel x
	/// @param y : pixel y
	/// @param color : pixel color
	////////////////////////////////////////////////////////
	virtual void SetPixel(Surface* dst, int x, int y, const Color &color) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap
	/// @param dst : the destination surface
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void Blit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles
	/// @param dst : the destination surface
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void TiledBlit(Surface* dst, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles
	/// @param dst : the destination surface
	/// @param ox : tile start x offset
	/// @param ox : tile start y offset
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void TiledBlit(Surface* dst, int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched
	/// @param dst : the destination surface
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void StretchBlit(Surface* dst, Bitmap* src, Rect src_rect, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched
	/// @param dst : the destination surface
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void StretchBlit(Surface* dst, Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap flipped
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param horizontal : flip horizontally
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	virtual void FlipBlit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, bool horizontal, bool vertical) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap scaled
	/// @param dst : the destination surface
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	////////////////////////////////////////////////////////
	virtual void ScaleBlit(Surface* dst, const Rect& dst_rect, Bitmap* src, const Rect& src_rect) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap scaled, rotated and translated
	/// @param dst : the destination surface
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param inv : transformation matrix
	///  - from destination coordinates to source coordinates
	////////////////////////////////////////////////////////
	virtual void TransformBlit(Surface *dst, Rect dst_rect, Bitmap* src, Rect src_rect, const Matrix& inv) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap scaled, rotated and translated
	/// @param dst : the destination surface
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param angle : rotation angle (positive is clockwise)
	/// @param dst_w : scaled width
	/// @param dst_h : scaled height
	/// @param src_pos_x : source origin x
	/// @param src_pos_y : source origin y
	/// @param dst_pos_x : destination origin x
	/// @param dst_pos_y : destination origin y
	////////////////////////////////////////////////////////
	virtual void TransformBlit(Surface *dst, Rect dst_rect,
							   Bitmap* src, Rect src_rect,
							   double angle,
							   double scale_x, double scale_y,
							   int src_pos_x, int src_pos_y,
							   int dst_pos_x, int dst_pos_y) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap transparency
	/// @param dst : the destination surface
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	////////////////////////////////////////////////////////
	virtual void Mask(Surface* dst, int x, int y, Bitmap* src, Rect src_rect) = 0;

	////////////////////////////////////////////////////////
	/// Blit source with waver effect.
	/// @param dst : the destination surface
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param depth : wave magnitude
	/// @param phase : wave phase
	////////////////////////////////////////////////////////
	virtual void WaverBlit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int depth, double phase) = 0;

	////////////////////////////////////////////////////////
	/// Fill entire bitmap with color.
	/// @param dst : the destination surface
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	virtual void Fill(Surface* dst, const Color &color) = 0;

	////////////////////////////////////////////////////////
	/// Fill bitmap rect with color.
	/// @param dst : the destination surface
	/// @param dst_rect : destination rect
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	virtual void FillRect(Surface* dst, Rect dst_rect, const Color &color) = 0;

	////////////////////////////////////////////////////////
	/// Clears the bitmap with transparent pixels.
	/// @param dst : the destination surface
	////////////////////////////////////////////////////////
	virtual void Clear(Surface* dst) = 0;

	////////////////////////////////////////////////////////
	/// Clears the bitmap rect with transparent pixels.
	/// @param dst : the destination surface
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	virtual void ClearRect(Surface* dst, Rect dst_rect) = 0;

	////////////////////////////////////////////////////////
	/// Rotate bitmap hue.
	/// @param dst : the destination surface
	/// @param hue : hue change, degrees
	////////////////////////////////////////////////////////
	virtual void HueChange(Surface* dst, double hue) = 0;

	////////////////////////////////////////////////////////
	/// Adjust bitmap HSL colors.
	/// @param dst : the destination surface
	/// @param hue : hue change, degrees
	/// @param sat : saturation scale
	/// @param lum : luminance scale
	/// @param loff: luminance offset
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	virtual void HSLChange(Surface* dst, double hue, double sat, double lum, double loff, Rect dst_rect) = 0;

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone.
	/// @param dst : the destination surface
	/// @param tone : tone to apply
	////////////////////////////////////////////////////////
	virtual void ToneChange(Surface* dst, const Rect& dst_rect, const Tone &tone) = 0;

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels.
	/// @param dst : the destination surface
	/// @param horizontal : flip horizontally (mirror)
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	virtual void Flip(Surface* dst, const Rect& dst_rect, bool horizontal, bool vertical) = 0;

	////////////////////////////////////////////////////////
	/// Change the opacity of a bitmap.
	/// @param dst : the destination surface
	/// @param opacity : the maximum opacity
	/// @param src_rect: the rectangle to modify
	////////////////////////////////////////////////////////
	virtual void OpacityChange(Surface* dst, int opacity, const Rect &src_rect) = 0;

	////////////////////////////////////////////////////////
	/// Set the color key of the associated DynamicFormat
	/// @param colorkey : the color-key
	////////////////////////////////////////////////////////
	virtual void SetColorKey(uint32 colorkey);

	////////////////////////////////////////////////////////
	/// Calculate the bounding rectangle of a transformed rectangle
	/// @param m    : transformation matrix
	/// @param rect : source rectangle
	/// @return : the bounding rectangle
	////////////////////////////////////////////////////////
	static Rect TransformRectangle(const Matrix& m, const Rect& rect);

	////////////////////////////////////////////////////////
	/// BitmapUtils factory function
	/// @param bpp : bits per pixel
	/// @param has_alpha : has alpha
	/// @param has_colorkey : has color-key
	/// @param format : DynamicFormat
	/// @return : a BitmapUtils instance for the specified pixel format
	////////////////////////////////////////////////////////
	static BitmapUtils* Create(int bpp, bool dynamic_alpha,
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

	Color GetPixel(Bitmap* src, int x, int y);
	void SetPixel(Surface* dst, int x, int y, const Color &color);
	Bitmap* Resample(Bitmap* src, int scale_w, int scale_h, const Rect& src_rect);
	Bitmap* RotateScale(Bitmap* src, double angle, int scale_w, int scale_h);
	Bitmap* Waver(Bitmap* src, int depth, double phase);
	void Blit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int opacity);
	void TiledBlit(Surface* dst, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);
	void TiledBlit(Surface* dst, int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);
	void StretchBlit(Surface* dst, Bitmap* src, Rect src_rect, int opacity);
	void StretchBlit(Surface* dst, Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);
	void FlipBlit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, bool horizontal, bool vertical);
	void ScaleBlit(Surface* dst, const Rect& dst_rect, Bitmap* src, const Rect& src_rect);
	void TransformBlit(Surface *dst, Rect dst_rect, Bitmap* src, Rect src_rect, const Matrix& inv);
	void TransformBlit(Surface *dst, Rect dst_rect,
					   Bitmap* src, Rect src_rect,
					   double angle,
					   double scale_x, double scale_y,
					   int src_pos_x, int src_pos_y,
					   int dst_pos_x, int dst_pos_y);
	void Mask(Surface* dst, int x, int y, Bitmap* src, Rect src_rect);
	void WaverBlit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int depth, double phase);
	void Fill(Surface* dst, const Color &color);
	void FillRect(Surface* dst, Rect dst_rect, const Color &color);
	void Clear(Surface* dst);
	void ClearRect(Surface* dst, Rect dst_rect);
	void HueChange(Surface* dst, double hue);
	void HSLChange(Surface* dst, double hue, double sat, double lum, double loff, Rect dst_rect);
	void ToneChange(Surface* dst, const Rect& dst_rect, const Tone &tone);
	void Flip(Surface* dst, const Rect& dst_rect, bool horizontal, bool vertical);
	void OpacityChange(Surface* dst, int opacity, const Rect &src_rect);
};

#endif
