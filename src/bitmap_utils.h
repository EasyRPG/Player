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
#include <map>
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
	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param src_pixel : pointer to source pixel
	/// @param r (out) : red
	/// @param g (out) : green
	/// @param b (out) : blue
	/// @param a (out) : alpha
	////////////////////////////////////////////////////////
	virtual void GetPixel(const uint8* src_pixels, uint8& r, uint8& g, uint8& b, uint8& a) = 0;

	////////////////////////////////////////////////////////
	/// Check the opacity of a pixel row
	/// @param src_pixels : pointer to source pixel row
	/// @param n : number of pixels
	/// @param all (out) : true if all pixels are opaque
	/// @param any (out) : true if any pixels are opaque
	////////////////////////////////////////////////////////
	virtual void CheckOpacity(const uint8* src_pixels, int n, bool& all, bool& any) = 0;

	////////////////////////////////////////////////////////
	/// Set a pixel to a given color.
	/// @param dst_pixel : pointer to destination pixel
	/// @param r : red
	/// @param g : green
	/// @param b : blue
	/// @param a : alpha
	////////////////////////////////////////////////////////
	virtual void SetPixel(uint8* dst_pixels, const uint8& r, const uint8& g, const uint8& b, const uint8& a) = 0;

	////////////////////////////////////////////////////////
	/// Set multiple pixels to a given value.
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void SetPixels(uint8* dst_pixels, const uint8* src_pixels, int n) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap to destination with opacity scaling
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixels
	/// @param n : number of pixels
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void Blit(uint8* dst_pixels, const uint8* src_pixels, int n, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap to destination with opacity scaling
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixels
	/// @param n : number of pixels
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void OpacityBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap over destination (transparency allows source through)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void OverlayBlit(uint8* dst_pixels, const uint8* src_pixels, int n) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap into destination (transparency is copied)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void CopyBlit(uint8* dst_pixels, const uint8* src_pixels, int n) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap to destination with horizontal flip
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void FlipHBlit(uint8* dst_pixels, const uint8* src_pixels, int n) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap to destination with scaling and opacity scaling
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param x : fixed point source x position
	/// @param step : fixed point source x step (inverse scale factor)
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void ScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap to destination with scaling and opacity scaling
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param x : fixed point source x position
	/// @param step : fixed point source x step (inverse scale factor)
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void OpacityScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap over destination with scaling
	///  (transparency allows source through)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param x : fixed point source x position
	/// @param step : fixed point source x step (inverse scale factor)
	////////////////////////////////////////////////////////
	virtual void OverlayScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap into destination with scaling
	///  (transparency is copied)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param x : fixed point source x position
	/// @param step : fixed point source x step (inverse scale factor)
	////////////////////////////////////////////////////////
	virtual void CopyScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap over destination with transformation and opacity scaling
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to top-left corner of source pixels
	/// @param src_pitch : pitch of source data
	/// @param x0 : destination x coordinate of row left
	/// @param x1 : destination x coordinate of row right
	/// @param y : destination y coordinat of row
	/// @param src_rect : source clip rectangle
	/// @param inv : inverse (dst->src) transformation matrix
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void TransformBlit(uint8* dst_pixels, const uint8* src_pixels, int src_pitch,
							   int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
							   int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap over destination with transformation and opacity scaling
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to top-left corner of source pixels
	/// @param src_pitch : pitch of source data
	/// @param x0 : destination x coordinate of row left
	/// @param x1 : destination x coordinate of row right
	/// @param y : destination y coordinat of row
	/// @param src_rect : source clip rectangle
	/// @param inv : inverse (dst->src) transformation matrix
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void OpacityTransformBlit(uint8* dst_pixels, const uint8* src_pixels, int src_pitch,
									  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
									  int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap over destination with transformation
	///  (transparency allows source through)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to top-left corner of source pixels
	/// @param src_pitch : pitch of source data
	/// @param x0 : destination x coordinate of row left
	/// @param x1 : destination x coordinate of row right
	/// @param y : destination y coordinat of row
	/// @param src_rect : source clip rectangle
	/// @param inv : inverse (dst->src) transformation matrix
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void OverlayTransformBlit(uint8* dst_pixels, const uint8* src_pixels, int src_pitch,
									  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap over destination with transformation
	///  (transparency is copied)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to top-left corner of source pixels
	/// @param src_pitch : pitch of source data
	/// @param x0 : destination x coordinate of row left
	/// @param x1 : destination x coordinate of row right
	/// @param y : destination y coordinat of row
	/// @param src_rect : source clip rectangle
	/// @param inv : inverse (dst->src) transformation matrix
	/// @param opacity : opacity scale (255 == unity)
	////////////////////////////////////////////////////////
	virtual void CopyTransformBlit(uint8* dst_pixels, const uint8* src_pixels, int src_pitch,
								   int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) = 0;

	////////////////////////////////////////////////////////
	/// Replace destination alpha with source alpha
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void MaskBlit(uint8* dst_pixels, const uint8* src_pixels, int n) = 0;

	////////////////////////////////////////////////////////
	/// Adjust bitmap HSL colors.
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param hue : hue change, degrees
	/// @param sat : saturation scale
	/// @param lum : luminance scale
	/// @param loff: luminance offset
	////////////////////////////////////////////////////////
	virtual void HSLBlit(uint8* dst_pixels, const uint8* src_pixels, int n,
						 int hue, int sat, int lum, int loff) = 0;

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone, without saturation change
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param tone : tone to apply (gray is ignored)
	////////////////////////////////////////////////////////
	virtual void ToneBlit(uint8* dst_pixels, const uint8* src_pixels, int n, const Tone& tone) = 0;

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone, with saturation change
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param tone : tone to apply
	/// @param factor : must be (255 - tone.gray) / 255;
	////////////////////////////////////////////////////////
	virtual void ToneBlit(uint8* dst_pixels, const uint8* src_pixels, int n, const Tone& tone, double factor) = 0;

	////////////////////////////////////////////////////////
	/// Blend bitmap with color
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param color : color to apply
	////////////////////////////////////////////////////////
	virtual void BlendBlit(uint8* dst_pixels, const uint8* src_pixels, int n, const Color& color) = 0;

	////////////////////////////////////////////////////////
	/// Adjust bitmap opacity
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of pixels
	/// @param tone : opacity scale (255 = unity)
	////////////////////////////////////////////////////////
	virtual void OpacityChangeBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int opacity) = 0;

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels horizontally and vertically.
	/// @param pixels_first (in, out) : pointer to the top-left pixel
	/// @param pixels_last (in, out) : pointer to the bottom-left pixel
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void FlipHV(uint8*& pixels_first, uint8*& pixels_last, int n) = 0;

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels horizontally
	/// @param pixels_left (in, out) : pointer to the left-most pixel
	/// @param pixels_last (in, out) : pointer to the right-most pixel
	/// @param n : number of pixels
	////////////////////////////////////////////////////////
	virtual void FlipH(uint8*& pixels_left, uint8*& pixels_right, int n) = 0;

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels vertically.
	/// @param pixels_up (in, out) : pointer to the upper pixel row
	/// @param pixels_down (in, out) : pointer to the lower pixel row
	/// @param n : number of pixels
	/// @param tmp_buffer : temporary buffer (size: n pixels)
	////////////////////////////////////////////////////////
	virtual void FlipV(uint8*& pixels_first, uint8*& pixels_last, int n, uint8* tmp_buffer) = 0;

	////////////////////////////////////////////////////////
	/// Blit source bitmap into destination scaled 2:1
	///  (transparency is copied)
	/// @param dst_pixels : pointer to destination pixel row
	/// @param src_pixel : pointer to source pixel row
	/// @param n : number of source pixels
	////////////////////////////////////////////////////////
	virtual void Blit2x(uint8* dst_pixels, const uint8* src_pixels, int n) = 0;

	////////////////////////////////////////////////////////
	/// Set the color key of the target
	/// @param colorkey : the color key
	////////////////////////////////////////////////////////
	virtual void SetDstColorKey(uint32 colorkey) = 0;

	////////////////////////////////////////////////////////
	/// Set the color key of the source
	/// @param colorkey : the color key
	////////////////////////////////////////////////////////
	virtual void SetSrcColorKey(uint32 colorkey) = 0;

	////////////////////////////////////////////////////////
	/// Set the destination dynamic format
	/// @param format : the new format
	////////////////////////////////////////////////////////
	virtual void SetDstFormat(const DynamicFormat& format) = 0;

	////////////////////////////////////////////////////////
	/// Set the source dynamic format
	/// @param format : the new format
	////////////////////////////////////////////////////////
	virtual void SetSrcFormat(const DynamicFormat& format) = 0;

	////////////////////////////////////////////////////////
	/// Get the destination pixel format
	/// @return : the destination format
	////////////////////////////////////////////////////////
	virtual const DynamicFormat& GetDstFormat() const = 0;

	////////////////////////////////////////////////////////
	/// Get the source pixel format
	/// @return : the source format
	////////////////////////////////////////////////////////
	virtual const DynamicFormat& GetSrcFormat() const = 0;

	////////////////////////////////////////////////////////
	/// BitmapUtils factory function
	/// @param bpp : bits per pixel
	/// @param has_alpha : has alpha
	/// @param has_colorkey : has color-key
	/// @param format : DynamicFormat
	/// @return : a BitmapUtils instance for the specified pixel format
	////////////////////////////////////////////////////////
	static BitmapUtils* Create(const DynamicFormat& dst_format,
							   const DynamicFormat& src_format,
							   bool need_source);

	/// Number of fraction bits for fixed point values (*ScaleBlit)
	static const int FRAC_BITS = 16;

protected:
	typedef std::pair<int, int> int_pair;
	static std::map<int, BitmapUtils*> unary_map;
	static std::map<int_pair, BitmapUtils*> binary_map;
	static bool maps_initialized;
};

#endif
