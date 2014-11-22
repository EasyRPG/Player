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
