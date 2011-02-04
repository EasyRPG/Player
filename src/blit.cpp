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
void BlitT<PFsrc, PFdst>::Blit1X(uint8* dst_pixels, int dst_pitch, Bitmap* src) {
	src->Lock();

	const uint8* src_pixels = (uint8*)src->pixels();
	int src_width = src->width();
	int src_height = src->height();
	int src_pad = src->pitch() - src_width * PFsrc::bytes;
	int dst_pad = dst_pitch - src_width * PFdst::bytes;

	for (int y = 0; y < src_height; y++) {
		blit1x_traits_type::copy_pixels(dst_format, dst_pixels, src_format, src_pixels, src_width);
		src_pixels += src_pad;
		dst_pixels += dst_pad;
	}

	src->Unlock();
}

template <class PFsrc, class PFdst>
void BlitT<PFsrc, PFdst>::Blit2X(uint8* dst_pixels, int dst_pitch, Bitmap* src) {
	src->Lock();

	const uint8* src_pixels = (uint8*)src->pixels();
	int src_width = src->width();
	int src_height = src->height();
	int src_pad = src->pitch() - src_width * PFsrc::bytes;
	int dst_pad = dst_pitch - 2 * src_width * PFdst::bytes;

	for (int y = 0; y < src_height; y++) {
		for (int x = 0; x < src_width; x++) {
			blit2x_traits_type::copy_pixel_2x(dst_format, dst_pixels, src_format, src_pixels);

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

template <class PFsrc, class PFdst>
bool BlitT<PFsrc, PFdst>::Match() {
	if (PFsrc::Format(src_format) != src_format)
		return false;
	if (PFdst::Format(dst_format) != dst_format)
		return false;
	return true;
}

template class BlitT<format_B8G8R8A8, format_dynamic_32>;
template class BlitT<format_B8G8R8A8, format_dynamic_24>;
template class BlitT<format_B8G8R8A8, format_dynamic_16>;

////////////////////////////////////////////////////////////
Blit* Blit::Create(int dst_bpp, const DynamicFormat& dst_format,
				   int src_bpp, const DynamicFormat& src_format) {
	if (src_bpp != 32)
		return NULL;
	Blit* blit = NULL;
	switch (dst_bpp) {
		case 32:
			blit = new BlitT<format_B8G8R8A8, format_dynamic_32>(dst_format, src_format);
			break;
		case 24:
			blit = new BlitT<format_B8G8R8A8, format_dynamic_24>(dst_format, src_format);
			break;
		case 16:
			blit = new BlitT<format_B8G8R8A8, format_dynamic_16>(dst_format, src_format);
			break;
	}
	if (blit->Match())
		return blit;

	delete blit;
	return NULL;
}

