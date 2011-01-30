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

#include "surface.h"
#include "pixel_format.h"
#include "blit.h"

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void Blit<PFsrc, PFdst>::Blit1X(const DynamicFormat& format, uint8* dst_pixels, int dst_pitch, Bitmap* src) {
	src->Lock();

	const uint8* src_pixels = (uint8*)src->pixels();
	int src_width = src->width();
	int src_height = src->height();
	int src_pad = src->pitch() - src_width * PFsrc::bytes;
	int dst_pad = dst_pitch - src_width * PFdst::bytes;

	for (int y = 0; y < src_height; y++) {
		blit1x_traits_type::copy_pixels(format, dst_pixels, src_pixels, src_width);
		src_pixels += src_pad;
		dst_pixels += dst_pad;
	}

	src->Unlock();
}

template <class PFsrc, class PFdst>
void Blit<PFsrc, PFdst>::Blit2X(const DynamicFormat& format, uint8* dst_pixels, int dst_pitch, Bitmap* src) {
	src->Lock();

	const uint8* src_pixels = (uint8*)src->pixels();
	int src_width = src->width();
	int src_height = src->height();
	int src_pad = src->pitch() - src_width * PFsrc::bytes;
	int dst_pad = dst_pitch - 2 * src_width * PFdst::bytes;

	for (int y = 0; y < src_height; y++) {
		for (int x = 0; x < src_width; x++) {
			blit2x_traits_type::copy_pixel_2x(format, dst_pixels, src_pixels);

			dst_pixels += PFdst::bytes;
			src_pixels += PFsrc::bytes;
		}

		src_pixels += src_pad;
		dst_pixels += dst_pad;

		memcpy(dst_pixels, dst_pixels - dst_pitch, dst_pitch);
		dst_pixels += dst_pitch;
	}

	src->Unlock();
}

template class Blit<format_B8G8R8A8, PixelFormat<32,false,true ,false, true, 8,16,8,8,8,0,8,24> >;
template class Blit<format_B8G8R8A8, PixelFormat<24,false,true ,false, true, 8,16,8,8,8,0,0,0> >;
template class Blit<format_B8G8R8A8, PixelFormat<16,false,true ,false, false,5,11,6,5,5,0,0,0> >;
