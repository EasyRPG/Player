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
/// Bitmap utils class template
////////////////////////////////////////////////////////////
template <class PF>
class BitmapUtils {
public:
	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @return pixel color
	////////////////////////////////////////////////////////
	static Color GetPixel(Bitmap* src, int x, int y);

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @param color : pixel color
	////////////////////////////////////////////////////////
	static void SetPixel(Surface* dst, int x, int y, const Color &color);

	////////////////////////////////////////////////////////
	/// Create a resampled bitmap.
	/// @param scale_w : resampled width
	/// @param scale_h : resampled height
	/// @param src_rect : source rect to resample
	////////////////////////////////////////////////////////
	static Bitmap* Resample(Bitmap* src, int scale_w, int scale_h, const Rect& src_rect);

	////////////////////////////////////////////////////////
	/// Create a rotated and scaled bitmap.
	/// @param angle : rotation angle
	/// @param scale_w : resampled width
	/// @param scale_h : resampled height
	////////////////////////////////////////////////////////
	static Bitmap* RotateScale(Bitmap* src, double angle, int scale_w, int scale_h);

	////////////////////////////////////////////////////////
	/// Create a wavy bitmap.
	/// @param depth : wave magnitude
	/// @param phase : wave phase
	////////////////////////////////////////////////////////
	static Bitmap* Waver(Bitmap* src, int depth, double phase);

	////////////////////////////////////////////////////////
	/// Blit source bitmap to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	static void Blit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	static void TiledBlit(Surface* dst, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param ox : tile start x offset
	/// @param ox : tile start y offset
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	static void TiledBlit(Surface* dst, int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	static void StretchBlit(Surface* dst, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	static void StretchBlit(Surface* dst, Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);

	/// Blit source bitmap transparency to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	////////////////////////////////////////////////////////
	static void Mask(Surface* dst, int x, int y, Bitmap* src, Rect src_rect);

	////////////////////////////////////////////////////////
	/// Fill entire bitmap with color.
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	static void Fill(Surface* dst, const Color &color);

	////////////////////////////////////////////////////////
	/// Fill bitmap rect with color.
	/// @param dst_rect : destination rect
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	static void FillRect(Surface* dst, Rect dst_rect, const Color &color);

	////////////////////////////////////////////////////////
	/// Clears the bitmap with transparent pixels.
	////////////////////////////////////////////////////////
	static void Clear(Surface* dst);

	////////////////////////////////////////////////////////
	/// Clears the bitmap rect with transparent pixels.
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	static void ClearRect(Surface* dst, Rect dst_rect);
	
	////////////////////////////////////////////////////////
	/// Rotate bitmap hue.
	/// @param hue : hue change, degrees
	////////////////////////////////////////////////////////
	static void HueChange(Surface* dst, double hue);

	////////////////////////////////////////////////////////
	/// Adjust bitmap HSL colors.
	/// @param hue : hue change, degrees
	/// @param sat : saturation scale
	/// @param lum : luminance scale
	/// @param loff: luminance offset
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	static void HSLChange(Surface* dst, double hue, double sat, double lum, double loff, Rect dst_rect);

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone.
	/// @param tone : tone to apply
	////////////////////////////////////////////////////////
	static void ToneChange(Surface* dst, const Tone &tone);

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels.
	/// @param horizontal : flip horizontally (mirror)
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	static void Flip(Surface* dst, bool horizontal, bool vertical);

	////////////////////////////////////////////////////////
	/// Change the opacity of a bitmap.
	/// @param opacity : the maximum opacity
	/// @param src_rect: the rectangle to modify
	////////////////////////////////////////////////////////
	static void OpacityChange(Surface* dst, int opacity, const Rect &src_rect);

};

#endif
