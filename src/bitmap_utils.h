/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EASYRPG_BITMAP_UTILS_H_
#define _EASYRPG_BITMAP_UTILS_H_

// Headers
#include <string>
#include <list>
#include <map>
#include "system.h"
#include "pixel_format.h"
#include "color.h"
#include "rect.h"
#include "tone.h"
#include "matrix.h"

/**
 * Bitmap utils abstract parent class.
 */
class BitmapUtils {
public:
	/**
	 * Gets a pixel color.
	 *
	 * @param src_pixels pointer to source pixel.
	 * @param r (out) red.
	 * @param g (out) green.
	 * @param b (out) blue.
	 * @param a (out) alpha.
	 */
	virtual void GetPixel(const uint8_t* src_pixels, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) = 0;

	/**
	 * Checks the opacity of a pixel row.
	 *
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param all (out) true if all pixels are opaque.
	 * @param any (out) true if any pixels are opaque.
	 */
	virtual void CheckOpacity(const uint8_t* src_pixels, int n, bool& all, bool& any) = 0;

	/**
	 * Sets a pixel to a given color.
	 *
	 * @param dst_pixels pointer to destination pixel.
	 * @param r red.
	 * @param g green.
	 * @param b blue.
	 * @param a alpha.
	 */
	virtual void SetPixel(uint8_t* dst_pixels, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) = 0;

	/**
	 * Sets multiple pixels to a given value.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel.
	 * @param n number of pixels.
	 */
	virtual void SetPixels(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) = 0;

	/**
	 * Blits source bitmap to destination with opacity scaling.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixels.
	 * @param n number of pixels.
	 * @param opacity opacity scale (255 == unity).
	 */
	virtual void Blit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity);

	/**
	 * Blits source bitmap to destination with opacity scaling.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixels.
	 * @param n number of pixels.
	 * @param opacity opacity scale (255 == unity).
	 */
	virtual void OpacityBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity) = 0;

	/**
	 * Blits source bitmap over destination (transparency allows source through).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 */
	virtual void OverlayBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) = 0;

	/**
	 * Blits source bitmap into destination (transparency is copied).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 */
	virtual void CopyBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) = 0;

	/**
	 * Blits source bitmap to destination with horizontal flip.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 */
	virtual void FlipHBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) = 0;

	/**
	 * Blits source bitmap to destination with scaling and opacity scaling.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param x fixed point source x position.
	 * @param step fixed point source x step (inverse scale factor).
	 * @param opacity opacity scale (255 == unity).
	 */
	virtual void ScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step, int opacity);

	/**
	 * Blits source bitmap to destination with scaling and opacity scaling.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param x fixed point source x position.
	 * @param step fixed point source x step (inverse scale factor).
	 * @param opacity opacity scale (255 == unity).
	 */
	virtual void OpacityScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step, int opacity) = 0;

	/**
	 * Blits source bitmap over destination with scaling
	 * (transparency allows source through).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param x fixed point source x position.
	 * @param step fixed point source x step (inverse scale factor).
	 */
	virtual void OverlayScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step) = 0;

	/**
	 * Blits source bitmap into destination with scaling
	 * (transparency is copied).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param x fixed point source x position.
	 * @param step fixed point source x step (inverse scale factor).
	 */
	virtual void CopyScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step) = 0;

	/**
	 * Blits source bitmap over destination with transformation and opacity scaling.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels  pointer to top-left corner of source pixels.
	 * @param src_pitch pitch of source data.
	 * @param x0 destination x coordinate of row left.
	 * @param x1 destination x coordinate of row right.
	 * @param y destination y coordinat of row.
	 * @param src_rect source clip rectangle.
	 * @param inv inverse (dst->src) transformation matrix.
	 * @param opacity opacity scale (255 == unity).
	 */
	virtual void TransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
							   int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
							   int opacity);

	/**
	 * Blits source bitmap over destination with transformation and opacity scaling.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to top-left corner of source pixels.
	 * @param src_pitch pitch of source data.
	 * @param x0 destination x coordinate of row left.
	 * @param x1 destination x coordinate of row right.
	 * @param y destination y coordinat of row.
	 * @param src_rect source clip rectangle.
	 * @param inv inverse (dst->src) transformation matrix.
	 * @param opacity opacity scale (255 == unity).
	 */
	virtual void OpacityTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
									  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
									  int opacity) = 0;

	/**
	 * Blits source bitmap over destination with transformation
	 * (transparency allows source through).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to top-left corner of source pixels.
	 * @param src_pitch pitch of source data.
	 * @param x0 destination x coordinate of row left.
	 * @param x1 destination x coordinate of row right.
	 * @param y destination y coordinat of row.
	 * @param src_rect source clip rectangle.
	 * @param inv inverse (dst->src) transformation matrix.
	 */
	virtual void OverlayTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
									  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) = 0;

	/**
	 * Blits source bitmap over destination with transformation
	 * (transparency is copied).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to top-left corner of source pixels.
	 * @param src_pitch pitch of source data.
	 * @param x0 destination x coordinate of row left.
	 * @param x1 destination x coordinate of row right.
	 * @param y destination y coordinat of row.
	 * @param src_rect source clip rectangle.
	 * @param inv inverse (dst->src) transformation matrix.
	 */
	virtual void CopyTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
								   int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) = 0;

	/**
	 * Replaces destination alpha with source alpha.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 */
	virtual void MaskBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) = 0;

	/**
	 * Adjusts bitmap HSL colors.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param hue hue change, degrees.
	 * @param sat saturation scale.
	 * @param lum luminance scale.
	 * @param loff luminance offset.
	 */
	virtual void HSLBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n,
						 int hue, int sat, int lum, int loff) = 0;

	/**
	 * Adjust bitmap tone, without saturation change
	 * @param dst_pixels : pointer to destination pixel row
	 * @param src_pixels : pointer to source pixel row
	 * @param n : number of pixels
	 * @param tone : tone to apply (gray is ignored)
	 */
	virtual void ToneBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Tone& tone) = 0;

	/**
	 * Adjusts bitmap tone, with saturation change.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param tone tone to apply.
	 * @param factor must be (255 - tone.gray) / 255.
	 */
	virtual void ToneBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Tone& tone, double factor) = 0;

	/**
	 * Blends bitmap with color.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param color color to apply.
	 */
	virtual void BlendBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Color& color) = 0;

	/**
	 * Adjusts bitmap opacity.
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of pixels.
	 * @param opacity opacity scale (255 = unity).
	 */
	virtual void OpacityChangeBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity) = 0;

	/**
	 * Flips the bitmap pixels horizontally and vertically.
	 *
	 * @param pixels_first (in, out) pointer to the top-left pixel.
	 * @param pixels_last (in, out) pointer to the bottom-left pixel.
	 * @param n number of pixels.
	 */
	virtual void FlipHV(uint8_t*& pixels_first, uint8_t*& pixels_last, int n) = 0;

	/**
	 * Flips the bitmap pixels horizontally.
	 *
	 * @param pixels_left (in, out) pointer to the left-most pixel.
	 * @param pixels_right (in, out) pointer to the right-most pixel.
	 * @param n number of pixels.
	 */
	virtual void FlipH(uint8_t*& pixels_left, uint8_t*& pixels_right, int n) = 0;

	/**
	 * Flips the bitmap pixels vertically.
	 *
	 * @param pixels_first (in, out) pointer to the top-left pixel.
	 * @param pixels_last (in, out) pointer to the bottom-left pixel.
	 * @param n number of pixels.
	 * @param tmp_buffer temporary buffer (size: n pixels).
	 */
	virtual void FlipV(uint8_t*& pixels_first, uint8_t*& pixels_last, int n, uint8_t* tmp_buffer) = 0;

	/**
	 * Blits source bitmap into destination scaled 2:1
	 * (transparency is copied).
	 *
	 * @param dst_pixels pointer to destination pixel row.
	 * @param src_pixels pointer to source pixel row.
	 * @param n number of source pixels.
	 */
	virtual void Blit2x(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) = 0;

	/**
	 * Sets the color key of the target.
	 *
	 * @param colorkey the color key.
	 */
	virtual void SetDstColorKey(uint32_t colorkey) = 0;

	/**
	 * Sets the color key of the source.
	 *
	 * @param colorkey the color key.
	 */
	virtual void SetSrcColorKey(uint32_t colorkey) = 0;

	/**
	 * Set the destination dynamic format.
	 *
	 * @param format the new format.
	 */
	virtual void SetDstFormat(const DynamicFormat& format) = 0;

	/**
	 * Set the source dynamic format.
	 *
	 * @param format the new format.
	 */
	virtual void SetSrcFormat(const DynamicFormat& format) = 0;

	/**
	 * Gets the destination pixel format.
	 *
	 * @return the destination format.
	 */
	virtual const DynamicFormat& GetDstFormat() const = 0;

	/**
	 * Gets the source pixel format.
	 *
	 * @return the source format.
	 */
	virtual const DynamicFormat& GetSrcFormat() const = 0;

	/**
	 * BitmapUtils factory function.
	 *
	 * @param dst_format destination pixel format.
	 * @param src_format source pixel format.
	 * @param need_source true if needs source pixel format.
	 * @return a BitmapUtils instance for the specified pixel format.
	 */
	static BitmapUtils* Create(const DynamicFormat& dst_format,
							   const DynamicFormat& src_format,
							   bool need_source);

	/** Number of fraction bits for fixed point values (*ScaleBlit). */
	static const int FRAC_BITS = 16;

protected:
	typedef std::pair<int, int> int_pair;
	static std::map<int, BitmapUtils*> unary_map;
	static std::map<int_pair, BitmapUtils*> binary_map;
	static bool maps_initialized;
};

#endif
