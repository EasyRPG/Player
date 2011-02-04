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

#include <algorithm>
#include <cmath>
#include <cstring>
#include "system.h"
#include "bitmap.h"
#include "surface.h"
#include "pixel_format.h"
#include "bitmap_utils.h"

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::GetPixel(const uint8* src_pixel, uint8& r, uint8& g, uint8& b, uint8& a) {
	PFsrc::get_rgba(format, src_pixel, r, g, b, a);
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CheckOpacity(const uint8* src_pixels, int n, bool& all, bool& any) {
	for (int x = 0; x < x; x++) {
		if (PFsrc::get_alpha(format, src_pixels) > 0)
			any = true;
		else
			all = false;
		if (any && !all)
			return;
		src_pixels += PFsrc::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetPixel(uint8* dst_pixel, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
	PFdst::set_rgba(format, dst_pixel, r, g, b, a);
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetPixels(uint8* dst_pixels, const uint8* src_pixel, int n) {
	uint8 tmp_pixel[4];
	copy_pixel(tmp_pixel, src_pixel);
	PFdst::set_pixels(dst_pixels, tmp_pixel, n);
}

////////////////////////////////////////////////////////////
/// Not Virtual
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::blend_pixel(uint8* dst_pixel, const uint8* src_pixel, int opacity) {
	uint8 sr, sg, sb, sa;
	PFsrc::get_rgba(src_format, src_pixel, sr, sg, sb, sa);
	uint8 dr, dg, db, da;
	PFdst::get_rgba(format, dst_pixel, dr, dg, db, da);
	int srca = (int) sa * opacity / PFsrc::ONE;
	uint8 rr, rg, rb, ra;

	if (sa == 0)
		;	// skip
	else if (sa >= PFsrc::opaque(src_format))
		copy_pixel(dst_pixel, src_pixel);
	else if (da == 0)
		copy_pixel(dst_pixel, src_pixel);
	else if (da >= PFdst::opaque(format)) {
		rr = (uint8) ((dr * (255 - srca) + sr * srca) / PFsrc::ONE);
		rg = (uint8) ((dg * (255 - srca) + sg * srca) / PFsrc::ONE);
		rb = (uint8) ((db * (255 - srca) + sb * srca) / PFsrc::ONE);
		ra = (uint8) ((da * (255 - srca)) / PFsrc::ONE + srca);
		PFdst::set_rgba(format, dst_pixel, rr, rg, rb, ra);
	}
	else {
		int dsta = (int) da;
		int resa = 255 - (255 - dsta) * (255 - srca) / PFsrc::ONE;
		rr = (uint8) ((dr * dsta * (255 - srca) + sr * srca) / resa);
		rg = (uint8) ((dg * dsta * (255 - srca) + sg * srca) / resa);
		rb = (uint8) ((db * dsta * (255 - srca) + sb * srca) / resa);
		ra = (uint8) resa;
		PFdst::set_rgba(format, dst_pixel, rr, rg, rb, ra);
	}
}

/// Not Virtual
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::overlay_pixel(uint8* dst_pixel, const uint8* src_pixel) {
	blend_pixel(dst_pixel, src_pixel, PFsrc::ONE);
}

/// Not Virtual
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::copy_pixel(uint8* dst_pixel, const uint8* src_pixel) {
	uint8 r, g, b, a;
	PFsrc::get_rgba(src_format, src_pixel, r, g, b, a);
	PFdst::set_rgba(format, dst_pixel, r, g, b, a);
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int opacity) {
	for (int i = 0; i < n; i++) {
		blend_pixel(dst_pixels, src_pixels, opacity);
		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OverlayBlit(uint8* dst_pixels, const uint8* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		overlay_pixel(dst_pixels, src_pixels);
		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CopyBlit(uint8* dst_pixels, const uint8* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		copy_pixel(dst_pixels, src_pixels);
		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipHBlit(uint8* dst_pixels, const uint8* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		copy_pixel(dst_pixels, src_pixels);
		src_pixels -= PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step, int opacity) {
	for (int i = 0; i < n; i++) {
		const uint8* p = src_pixels + (x >> FRAC_BITS) * PFsrc::bytes;
		blend_pixel(dst_pixels, p, opacity);
		dst_pixels += PFdst::bytes;
		x += step;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OverlayScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step) {
	for (int i = 0; i < n; i++) {
		const uint8* p = src_pixels + (x >> FRAC_BITS) * PFsrc::bytes;
		overlay_pixel(dst_pixels, p);
		dst_pixels += PFdst::bytes;
		x += step;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CopyScaleBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int x, int step) {
	for (int i = 0; i < n; i++) {
		const uint8* p = src_pixels + (x >> FRAC_BITS) * PFsrc::bytes;
		copy_pixel(dst_pixels, p);
		dst_pixels += PFdst::bytes;
		x += step;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::TransformBlit(uint8* dst_pixels, const uint8* src_pixels, int src_pitch,
											  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) {
	int sx0 = src_rect.x;
	int sy0 = src_rect.y;
	int sx1 = src_rect.x + src_rect.width;
	int sy1 = src_rect.y + src_rect.height;

	for (int x = x0; x < x1; x++) {
		double fx, fy;
		inv.Transform(x + 0.5, y + 0.5, fx, fy);
		int xi = (int) floor(fx);
		int yi = (int) floor(fy);
		if (xi < sx0 || xi >= sx1 || yi < sy0 || yi >= sy1)
			;
		else
			overlay_pixel(dst_pixels, &src_pixels[yi * src_pitch + xi * PFsrc::bytes]);
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::MaskBlit(uint8* dst_pixels, const uint8* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		uint8 sa = PFsrc::get_alpha(src_format, src_pixels);
		uint8 da = PFdst::get_alpha(format, dst_pixels);
		uint8 ra = std::min(sa, da);
		PFdst::set_alpha(format, dst_pixels, ra);
		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
static inline void RGB_to_HSL(const uint8& r, const uint8& g, const uint8& b,
							  int &h, int &s, int &l)
{
	enum RGBOrder {
		O_RGB,
		O_RBG,
		O_GRB,
		O_GBR,
		O_BRG,
		O_BGR
	} order = (r > g)
		  ? ((r > b) ? ((g < b) ? O_RBG : O_RGB) : O_BRG)
		  : ((r < b) ? ((g > b) ? O_GBR : O_BGR) : O_GRB);

	int c = 0;
	int l2 = 0;
	switch (order) {
		case O_RGB: c = (r - b); h = (c == 0) ? 0 : 0x100*(g - b)/c + 0x000; l2 = (r + b); break;
		case O_RBG: c = (r - g); h = (c == 0) ? 0 : 0x100*(g - b)/c + 0x600; l2 = (r + g); break;
		case O_GRB: c = (g - b); h = (c == 0) ? 0 : 0x100*(b - r)/c + 0x200; l2 = (g + b); break;
		case O_GBR: c = (g - r); h = (c == 0) ? 0 : 0x100*(b - r)/c + 0x200; l2 = (g + r); break;
		case O_BRG: c = (b - g); h = (c == 0) ? 0 : 0x100*(r - g)/c + 0x400; l2 = (b + g); break;
		case O_BGR: c = (b - r); h = (c == 0) ? 0 : 0x100*(r - g)/c + 0x400; l2 = (b + r); break;
	}

	if (l2 == 0) {
		s = 0;
		l = 0;
	}
	else {
		s = 0x100 * c / ((l2 > 0xFF) ? 0x1FF - l2 : l2);
		l = l2 / 2;
	}
}

static inline void HSL_to_RGB(const int& h, const int& s, const int& l,
							  uint8 &r, uint8 &g, uint8 &b)
{

	int l2 = 2 * l;
	int c = s * ((l2 > 0xFF) ? 0x1FF - l2 : l2) / 0x100;
	int m = (l2 - c) / 2;
	int h0 = h & 0xFF;
	int h1 = 0xFF - h0;

	switch (h >> 8) {
		case 0: r = m + c; g = m + h0*c/0x100; b = m; break;
		case 1: r = m + h1*c/0x100; g = m + c; b = m; break;
		case 2: r = m; g = m + c; b = m + h0*c/0x100; break;
		case 3: r = m; g = m + h1*c/0x100; b = m + c; break;
		case 4: r = m + h0*c/0x100; g = m; b = m + c; break;
		case 5: r = m + c; g = m; b = m + h1*c/0x100; break;
	}
}

static inline void HSL_adjust(int& h, int& s, int& l,
							  int hue, int sat, int lmul, int loff) {
	h += hue;
	if (h > 0x600) h -= 0x600;
	s = s * sat / 0x100;
	if (s > 0xFF) s = 0xFF;
	l = l * lmul / 0x100 + loff;
	l = (l > 0xFF) ? 0xFF : (l < 0) ? 0 : l;
}

static inline void RGB_adjust_HSL(uint8& r, uint8& g, uint8& b,
								  int hue, int sat, int lmul, int loff) {
	int h, s, l;
	RGB_to_HSL(r, g, b, h, s, l);
	HSL_adjust(h, s, l, hue, sat, lmul, loff);
	HSL_to_RGB(h, s, l, r, g, b);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::HSLBlit(uint8* dst_pixels, const uint8* src_pixels, int n,
										double hue, double sat, double lum, double loff) {
	for (int i = 0; i < n; i++) {
		uint8 r, g, b, a;
		PFsrc::get_rgba(src_format, src_pixels, r, g, b, a);
		if (a != 0) {
			RGB_adjust_HSL(r, g, b, hue, sat, lum, loff);
			PFdst::set_rgba(format, dst_pixels, r, g, b, a);
		}
		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::ToneBlit(uint8* dst_pixels, const uint8* src_pixels, int n, const Tone& tone) {
	for (int i = 0; i < n; i++) {
		uint8 r, g, b, a;
		PFsrc::get_rgba(src_format, src_pixels, r, g, b, a);
		if (a != 0) {
			r = (uint8)std::max(std::min(r + tone.red,   255), 0);
			g = (uint8)std::max(std::min(g + tone.green, 255), 0);
			b = (uint8)std::max(std::min(b + tone.blue,  255), 0);
			PFdst::set_rgba(format, dst_pixels, r, g, b, a);
		}
		dst_pixels += PFdst::bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::ToneBlit(uint8* dst_pixels, const uint8* src_pixels, int n,
										 const Tone& tone, double factor) {
	for (int i = 0; i < n; i++) {
		uint8 r, g, b, a;
		PFsrc::get_rgba(src_format, src_pixels, r, g, b, a);
		if (a != 0) {
			double gray = r * 0.299 + g * 0.587 + b * 0.114;

			r = (uint8)std::max(std::min((r - gray) * factor + gray + tone.red   + 0.5, 255.0), 0.0);
			g = (uint8)std::max(std::min((g - gray) * factor + gray + tone.green + 0.5, 255.0), 0.0);
			b = (uint8)std::max(std::min((b - gray) * factor + gray + tone.blue  + 0.5, 255.0), 0.0);

			PFdst::set_rgba(format, dst_pixels, r, g, b, a);
		}

		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityChangeBlit(uint8* dst_pixels, const uint8* src_pixels, int n, int opacity) {
	for (int i = 0; i < n; i++) {
		uint8 a = PFsrc::get_alpha(src_format, src_pixels);
		a = (uint8) ((a * opacity) / PFsrc::ONE);
		PFdst::set_alpha(format, dst_pixels, a);
		src_pixels += PFsrc::bytes;
		dst_pixels += PFdst::bytes;
	}
}

////////////////////////////////////////////////////////////
template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipHV(uint8*& pixels_first, uint8*& pixels_last, int n) {
	for (int i = 0; i < n; i++) {
		if (pixels_first == pixels_last)
			break;

		uint8 tmp_buffer[4];
		PFdst::copy_pixel(tmp_buffer, pixels_first);
		PFdst::copy_pixel(pixels_first, pixels_last);
		PFdst::copy_pixel(pixels_last, tmp_buffer);

		pixels_first += PFdst::bytes;
		pixels_last -= PFdst::bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipH(uint8*& pixels_left, uint8*& pixels_right, int n) {
	for (int i = 0; i < n; i++) {
		if (pixels_left == pixels_right)
			continue;

		uint8 tmp_buffer[4];
		PFdst::copy_pixel(tmp_buffer, pixels_left);
		PFdst::copy_pixel(pixels_left, pixels_right);
		PFdst::copy_pixel(pixels_right, tmp_buffer);

		pixels_left += PFdst::bytes;
		pixels_right -= PFdst::bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipV(uint8*& pixels_up, uint8*& pixels_down, int n, uint8* tmp_buffer) {
	PFdst::copy_pixels(tmp_buffer, pixels_down, n);
	PFdst::copy_pixels(pixels_down, pixels_up, n);
	PFdst::copy_pixels(pixels_up, tmp_buffer, n);
}

////////////////////////////////////////////////////////////
void BitmapUtils::SetColorKey(uint32 colorkey) {
	format.SetColorKey(colorkey);
}

void BitmapUtils::SetSrcColorKey(uint32 colorkey) {
	src_format.SetColorKey(colorkey);
}

void BitmapUtils::SetSrcFormat(const DynamicFormat& format) {
	src_format = format;
}

////////////////////////////////////////////////////////////
template class BitmapUtilsT<format_B8G8R8A8,format_B8G8R8A8>;
template class BitmapUtilsT<format_R8G8B8A8,format_R8G8B8A8>;
template class BitmapUtilsT<format_A8B8G8R8,format_A8B8G8R8>;
template class BitmapUtilsT<format_A8R8G8B8,format_A8R8G8B8>;

template class BitmapUtilsT<format_dynamic_32_a,format_dynamic_32_a>;
template class BitmapUtilsT<format_dynamic_32_ck,format_dynamic_32_ck>;

template class BitmapUtilsT<format_dynamic_24_ck,format_dynamic_24_ck>;

template class BitmapUtilsT<format_dynamic_16_a,format_dynamic_16_a>;
template class BitmapUtilsT<format_dynamic_16_ck,format_dynamic_16_ck>;

////////////////////////////////////////////////////////////
BitmapUtils* BitmapUtils::Create(int bpp, bool dynamic_alpha,
								 const DynamicFormat& format,
								 const DynamicFormat& src_format) {
	if (bpp == 32 && !dynamic_alpha && format.has_alpha && !format.has_colorkey) {
		if (format.a.mask == format_B8G8R8A8::a_mask(format)) {
			if (format.r.mask == format_B8G8R8A8::r_mask(format) &&
				format.g.mask == format_B8G8R8A8::g_mask(format) &&
				format.b.mask == format_B8G8R8A8::b_mask(format))
				return new BitmapUtilsT<format_B8G8R8A8,format_B8G8R8A8>(format, src_format);
			if (format.r.mask == format_R8G8B8A8::r_mask(format) &&
				format.g.mask == format_R8G8B8A8::g_mask(format) &&
				format.b.mask == format_R8G8B8A8::b_mask(format))
				return new BitmapUtilsT<format_R8G8B8A8,format_R8G8B8A8>(format, src_format);
		}
		if (format.a.mask == format_A8B8G8R8::a_mask(format)) {
			if (format.r.mask == format_A8B8G8R8::r_mask(format) &&
				format.g.mask == format_A8B8G8R8::g_mask(format) &&
				format.b.mask == format_A8B8G8R8::b_mask(format))
				return new BitmapUtilsT<format_A8B8G8R8,format_A8B8G8R8>(format, src_format);
			if (format.r.mask == format_A8R8G8B8::r_mask(format) &&
				format.g.mask == format_A8R8G8B8::g_mask(format) &&
				format.b.mask == format_A8R8G8B8::b_mask(format))
				return new BitmapUtilsT<format_A8R8G8B8,format_A8R8G8B8>(format, src_format);
		}
	}

	switch (bpp) {
		case 32:
			if (format.has_alpha)
				return new BitmapUtilsT<format_dynamic_32_a,format_dynamic_32_a>(format, src_format);
			else
				return new BitmapUtilsT<format_dynamic_32_ck,format_dynamic_32_ck>(format, src_format);
			break;
		case 24:
			return new BitmapUtilsT<format_dynamic_24_ck,format_dynamic_24_ck>(format, src_format);
			break;
		case 16:
			if (format.has_alpha)
				return new BitmapUtilsT<format_dynamic_16_a,format_dynamic_16_a>(format, src_format);
			else
				return new BitmapUtilsT<format_dynamic_16_ck,format_dynamic_16_ck>(format, src_format);
			break;
		default:
			return NULL;
	}
}

