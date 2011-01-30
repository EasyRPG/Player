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
	static void copy_pixels(const DynamicFormat& format, uint8*& dst_pixels, const uint8*& src_pixels, int count) {
		for (int x = 0; x < count; x++) {
			uint8 r, g, b, a;
			PFsrc::get_rgba(format, src_pixels, r, g, b, a);
			src_pixels += PFsrc::bytes;
			PFdst::set_rgba(format, dst_pixels, r, g, b, a);
			dst_pixels += PFdst::bytes;
		}
	}
};

template <class PFsrc, class PFdst>
struct blit1x_traits<PFsrc, PFdst, true> {
	static void copy_pixels(const DynamicFormat& format, uint8*& dst_pixels, const uint8*& src_pixels, int count) {
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
	static void copy_pixel_2x(const DynamicFormat& format, uint8*& dst_pixels, const uint8*& src_pixels) {
		uint8 r, g, b, a;
		PFsrc::get_rgba(format, src_pixels, r, g, b, a);
		src_pixels += PFsrc::bytes;
		PFdst::set_rgba(format, dst_pixels, r, g, b, a);
		dst_pixels += PFdst::bytes;
		PFdst::set_rgba(format, dst_pixels, r, g, b, a);
		dst_pixels += PFdst::bytes;
	}
};

template <class PFsrc, class PFdst>
struct blit2x_traits<PFsrc, PFdst, true> {
	static void copy_pixel_2x(const DynamicFormat& format, uint8*& dst_pixels, const uint8*& src_pixels) {
		PFsrc::copy_pixel(dst_pixels, src_pixels);
		dst_pixels += PFdst::bytes;
		PFsrc::copy_pixel(dst_pixels, src_pixels);
		dst_pixels += PFdst::bytes;
		src_pixels += PFsrc::bytes;
	}
};

////////////////////////////////////////////////////////////
/// Blit class
////////////////////////////////////////////////////////////

template <class PFsrc, class PFdst>
class Blit {
	static const bool same =
		PFsrc::bits == PFdst::bits   &&
		PFsrc::has_alpha == PFdst::has_alpha &&
		PFsrc::has_colorkey == PFdst::has_colorkey &&
		PFsrc::mask_r_traits_type::_mask == PFdst::mask_r_traits_type::_mask  &&
		PFsrc::mask_g_traits_type::_mask == PFdst::mask_g_traits_type::_mask  &&
		PFsrc::mask_b_traits_type::_mask == PFdst::mask_b_traits_type::_mask  &&
		PFsrc::mask_a_traits_type::_mask == PFdst::mask_a_traits_type::_mask  &&
		!PFsrc::dynamic &&
		!PFdst::dynamic;
	typedef blit1x_traits<PFsrc,PFdst,same> blit1x_traits_type;
	typedef blit2x_traits<PFsrc,PFdst,same> blit2x_traits_type;

public:
	static void Blit1X(const DynamicFormat& format, uint8* dst_pixels, int dst_pitch, Bitmap* src);
	static void Blit2X(const DynamicFormat& format, uint8* dst_pixels, int dst_pitch, Bitmap* src);
};

////////////////////////////////////////////////////////////

#endif

