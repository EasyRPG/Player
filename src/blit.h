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

#ifndef _EASYRPG_BLIT_H_
#define _EASYRPG_BLIT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <cstdlib>
#include "pixel_format.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
/// Blit1X traits
////////////////////////////////////////////////////////////

template <class PFsrc, class PFdst, bool same>
struct blit1x_traits {
};

template <class PFsrc, class PFdst>
struct blit1x_traits<PFsrc, PFdst, false> {
	static void copy_pixels(const DynamicFormat& dst_format, uint8*& dst_pixels,
							const DynamicFormat& src_format, const uint8*& src_pixels,
							int count) {
		for (int x = 0; x < count; x++) {
			uint8 r, g, b, a;
			PFsrc::get_rgba(src_format, src_pixels, r, g, b, a);
			src_pixels += PFsrc::bytes;
			PFdst::set_rgba(dst_format, dst_pixels, r, g, b, a);
			dst_pixels += PFdst::bytes;
		}
	}
};

template <class PFsrc, class PFdst>
struct blit1x_traits<PFsrc, PFdst, true> {
	static void copy_pixels(const DynamicFormat& dst_format, uint8*& dst_pixels,
							const DynamicFormat& src_format, const uint8*& src_pixels,
							int count) {
		memcpy(dst_pixels, src_pixels, count * PFsrc::bytes);
	}
};

////////////////////////////////////////////////////////////
/// Blit2X traits
////////////////////////////////////////////////////////////

template <class PFsrc, class PFdst, bool same>
struct blit2x_traits {
};

template <class PFsrc, class PFdst>
struct blit2x_traits<PFsrc, PFdst, false> {
	static void copy_pixel_2x(const DynamicFormat& dst_format, uint8*& dst_pixels,
							  const DynamicFormat& src_format, const uint8*& src_pixels) {
		uint8 r, g, b, a;
		PFsrc::get_rgba(src_format, src_pixels, r, g, b, a);
		src_pixels += PFsrc::bytes;
		PFdst::set_rgba(dst_format, dst_pixels, r, g, b, a);
		dst_pixels += PFdst::bytes;
		PFdst::set_rgba(dst_format, dst_pixels, r, g, b, a);
		dst_pixels += PFdst::bytes;
	}
};

template <class PFsrc, class PFdst>
struct blit2x_traits<PFsrc, PFdst, true> {
	static void copy_pixel_2x(const DynamicFormat& dst_format, uint8*& dst_pixels,
							  const DynamicFormat& src_format, const uint8*& src_pixels) {
		PFsrc::copy_pixel(dst_pixels, src_pixels);
		dst_pixels += PFdst::bytes;
		PFsrc::copy_pixel(dst_pixels, src_pixels);
		dst_pixels += PFdst::bytes;
		src_pixels += PFsrc::bytes;
	}
};

////////////////////////////////////////////////////////////
/// Blit abstract parent class
////////////////////////////////////////////////////////////

class Blit {
public:
	Blit(const DynamicFormat &dst_format, const DynamicFormat &src_format) :
		dst_format(dst_format),
		src_format(src_format) {}

	virtual void Blit1X(uint8* dst_pixels, int dst_pitch, Bitmap* src) = 0;
	virtual void Blit2X(uint8* dst_pixels, int dst_pitch, Bitmap* src) = 0;
	virtual bool Match() = 0;

	static Blit* Create(int dst_bpp, const DynamicFormat& dst_format,
						int src_bpp, const DynamicFormat& src_format);
protected:
	DynamicFormat dst_format;
	DynamicFormat src_format;
};

////////////////////////////////////////////////////////////
/// Blit template class
////////////////////////////////////////////////////////////

template <class PFsrc, class PFdst>
class BlitT : public Blit {
	static const bool same =
		PFsrc::bits == PFdst::bits   &&
		PFsrc::alpha == PFdst::alpha &&
		PFsrc::colorkey == PFdst::colorkey &&
		PFsrc::mask_r_traits_type::mask == PFdst::mask_r_traits_type::mask  &&
		PFsrc::mask_g_traits_type::mask == PFdst::mask_g_traits_type::mask  &&
		PFsrc::mask_b_traits_type::mask == PFdst::mask_b_traits_type::mask  &&
		PFsrc::mask_a_traits_type::mask == PFdst::mask_a_traits_type::mask  &&
		!PFsrc::dynamic_masks &&
		!PFdst::dynamic_masks;
	typedef blit1x_traits<PFsrc,PFdst,same> blit1x_traits_type;
	typedef blit2x_traits<PFsrc,PFdst,same> blit2x_traits_type;

public:
	BlitT(const DynamicFormat &dst_format, const DynamicFormat &src_format) :
		Blit(dst_format, src_format) {}

	void Blit1X(uint8* dst_pixels, int dst_pitch, Bitmap* src);
	void Blit2X(uint8* dst_pixels, int dst_pitch, Bitmap* src);
	bool Match();
};

////////////////////////////////////////////////////////////

#endif

