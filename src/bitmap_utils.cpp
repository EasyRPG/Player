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

// Headers
#include <algorithm>
#include <map>
#include <cmath>
#include <cstring>
#include "system.h"
#include "output.h"
#include "bitmap.h"
#include "pixel_format.h"
#include "bitmap_utils.h"

//Traits types.
template <class BMU,
		  PF::OpacityType SrcAlpha,
		  PF::OpacityType DstAlpha>
struct blend_traits {
	static inline void blend_pixel(BMU* bmu, uint8_t* dst_pixel, const uint8_t* src_pixel, int opacity) {
		uint8_t sr, sg, sb, sa;
		bmu->pf_src.get_rgba(src_pixel, sr, sg, sb, sa);
		uint8_t dr, dg, db, da;
		bmu->pf_dst.get_rgba(dst_pixel, dr, dg, db, da);
		int srca = (int) sa * opacity / PF::ONE;

		if (sa == 0)
			;	// skip
		else if (srca >= bmu->pf_src.opaque())
			bmu->copy_pixel(dst_pixel, src_pixel);
		else if (da == 0)
			bmu->copy_pixel(dst_pixel, src_pixel);
		else if (da >= bmu->pf_dst.opaque()) {
			uint8_t rr = (uint8_t) ((dr * (255 - srca) + sr * srca) / PF::ONE);
			uint8_t rg = (uint8_t) ((dg * (255 - srca) + sg * srca) / PF::ONE);
			uint8_t rb = (uint8_t) ((db * (255 - srca) + sb * srca) / PF::ONE);
			uint8_t ra = (uint8_t) ((da * (255 - srca)) / PF::ONE + srca);
			bmu->pf_dst.set_rgba(dst_pixel, rr, rg, rb, ra);
		}
		else {
			int dsta = (int) da;
			int resa = 255 - (255 - dsta) * (255 - srca) / PF::ONE;
			uint8_t rr = (uint8_t) ((dr * dsta * (255 - srca) + sr * srca) / resa);
			uint8_t rg = (uint8_t) ((dg * dsta * (255 - srca) + sg * srca) / resa);
			uint8_t rb = (uint8_t) ((db * dsta * (255 - srca) + sb * srca) / resa);
			uint8_t ra = (uint8_t) resa;
			bmu->pf_dst.set_rgba(dst_pixel, rr, rg, rb, ra);
		}
	}
};

template <class BMU,
		  PF::OpacityType SrcAlpha,
		  PF::OpacityType DstAlpha>
struct overlay_traits {
	static inline void overlay_pixel(BMU* bmu, uint8_t* dst_pixel, const uint8_t* src_pixel) {
		bmu->blend_pixel(dst_pixel, src_pixel, PF::ONE);
	}
};

template <class BMU, PF::OpacityType DstAlpha>
struct overlay_traits<BMU, PF::Binary, DstAlpha> {
	static inline void overlay_pixel(BMU* bmu, uint8_t* dst_pixel, const uint8_t* src_pixel) {
		if (bmu->pf_src.get_alpha(src_pixel) != 0)
			bmu->copy_pixel(dst_pixel, src_pixel);
	}
};

template <class BMU, PF::OpacityType DstAlpha>
struct overlay_traits<BMU, PF::Opaque, DstAlpha> {
	static inline void overlay_pixel(BMU* bmu, uint8_t* dst_pixel, const uint8_t* src_pixel) {
		bmu->copy_pixel(dst_pixel, src_pixel);
	}
};

template <class BMU, bool SameFormat>
struct copy_traits {};

template <class BMU>
struct copy_traits<BMU, false> {
	static inline void copy_pixel(BMU* bmu, uint8_t* dst_pixel, const uint8_t* src_pixel) {
		uint8_t r, g, b, a;
		bmu->pf_src.get_rgba(src_pixel, r, g, b, a);
		bmu->pf_dst.set_rgba(dst_pixel, r, g, b, a);
	}

	static inline void copy_pixels(BMU* bmu, uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
		for (int i = 0; i < n; i++) {
			copy_pixel(bmu, dst_pixels, src_pixels);
			src_pixels += bmu->pf_src.bytes;
			dst_pixels += bmu->pf_dst.bytes;
		}
	}
};

template <class BMU>
struct copy_traits<BMU, true> {
	static inline void copy_pixel(BMU* bmu, uint8_t* dst_pixel, const uint8_t* src_pixel) {
		bmu->pf_dst.copy_pixel(dst_pixel, src_pixel);
	}
	static inline void copy_pixels(BMU* bmu, uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
		bmu->pf_dst.copy_pixels(dst_pixels, src_pixels, n);
	}
};

// BitmapUtils class template.
template <class PFsrc, class PFdst>
class BitmapUtilsT : public BitmapUtils {
public:
	// Implementations of inherited pure virtual methods.
	// For documentation, see parent BitmapUtils class.
	void GetPixel(const uint8_t* src_pixels, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
	void CheckOpacity(const uint8_t* src_pixels, int n, bool& all, bool& any);
	void SetPixel(uint8_t* dst_pixels, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a);
	void SetPixels(uint8_t* dst_pixels, const uint8_t* src_pixels, int n);
	void OpacityBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity);
	void OverlayBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n);
	void CopyBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n);
	void FlipHBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n);
	void OpacityScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step, int opacity);
	void OverlayScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step);
	void CopyScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step);
	void OpacityTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
							  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
							  int opacity);
	void OverlayTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
							  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv);
	void CopyTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
						   int x0, int x1, int y, const Rect& src_rect, const Matrix& inv);
	void MaskBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n);
	void HSLBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n,
					int hue, int sat, int lum, int loff);
	void ToneBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Tone& tone);
	void ToneBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Tone& tone, double factor);
	void BlendBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Color& color);
	void OpacityChangeBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity);
	void FlipHV(uint8_t*& pixels_first, uint8_t*& pixels_last, int n);
	void FlipH(uint8_t*& pixels_left, uint8_t*& pixels_right, int n);
	void FlipV(uint8_t*& pixels_first, uint8_t*& pixels_last, int n, uint8_t* tmp_buffer);
	void Blit2x(uint8_t* dst_pixels, const uint8_t* src_pixels, int n);

	void SetDstColorKey(uint32_t colorkey);
	void SetSrcColorKey(uint32_t colorkey);
	void SetDstFormat(const DynamicFormat& format);
	void SetSrcFormat(const DynamicFormat& format);
	const DynamicFormat& GetDstFormat() const;
	const DynamicFormat& GetSrcFormat() const;

	typedef BitmapUtilsT<PFsrc,PFdst> my_type;
	static const PF::OpacityType src_opacity = PFsrc::opacity;
	static const PF::OpacityType dst_opacity = PFdst::opacity;
	static const bool same_format =
		!PFsrc::dynamic_masks &&
		!PFdst::dynamic_masks &&
		PFsrc::mask_r_traits_type::_mask == PFdst::mask_r_traits_type::_mask &&
		PFsrc::mask_g_traits_type::_mask == PFdst::mask_g_traits_type::_mask &&
		PFsrc::mask_b_traits_type::_mask == PFdst::mask_b_traits_type::_mask;

	typedef blend_traits<my_type, src_opacity, dst_opacity> blend_traits_type;
	typedef overlay_traits<my_type, src_opacity, dst_opacity> overlay_traits_type;
	typedef copy_traits<my_type, same_format> copy_traits_type;

	inline void blend_pixel(uint8_t* dst_pixel, const uint8_t* src_pixel, int opacity) {
		blend_traits_type::blend_pixel(this, dst_pixel, src_pixel, opacity);
	}
	inline void overlay_pixel(uint8_t* dst_pixel, const uint8_t* src_pixel) {
		overlay_traits_type::overlay_pixel(this, dst_pixel, src_pixel);
	}
	inline void copy_pixel(uint8_t* dst_pixel, const uint8_t* src_pixel) {
		copy_traits_type::copy_pixel(this, dst_pixel, src_pixel);
	}
	inline void copy_pixels(uint8_t* dst_pixel, const uint8_t* src_pixel, int n) {
		copy_traits_type::copy_pixels(this, dst_pixel, src_pixel, n);
	}

//protected:
	PFsrc pf_src;
	PFdst pf_dst;
};

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::GetPixel(const uint8_t* src_pixel, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
	pf_src.get_rgba(src_pixel, r, g, b, a);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CheckOpacity(const uint8_t* src_pixels, int n, bool& all, bool& any) {
	for (int x = 0; x < n; x++) {
		if (pf_src.get_alpha(src_pixels) > 0)
			any = true;
		else
			all = false;
		if (any && !all)
			return;
		src_pixels += pf_src.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetPixel(uint8_t* dst_pixel, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
	pf_dst.set_rgba(dst_pixel, r, g, b, a);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetPixels(uint8_t* dst_pixels, const uint8_t* src_pixel, int n) {
	uint8_t tmp_pixel[4];
	copy_pixel(tmp_pixel, src_pixel);
	pf_dst.set_pixels(dst_pixels, tmp_pixel, n);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity) {
	for (int i = 0; i < n; i++) {
		blend_pixel(dst_pixels, src_pixels, opacity);
		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OverlayBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		overlay_pixel(dst_pixels, src_pixels);
		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CopyBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
	copy_pixels(dst_pixels, src_pixels, n);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipHBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		copy_pixel(dst_pixels, src_pixels);
		src_pixels -= pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step, int opacity) {
	for (int i = 0; i < n; i++) {
		const uint8_t* p = src_pixels + (x >> FRAC_BITS) * pf_src.bytes;
		blend_pixel(dst_pixels, p, opacity);
		dst_pixels += pf_dst.bytes;
		x += step;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OverlayScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step) {
	for (int i = 0; i < n; i++) {
		const uint8_t* p = src_pixels + (x >> FRAC_BITS) * pf_src.bytes;
		overlay_pixel(dst_pixels, p);
		dst_pixels += pf_dst.bytes;
		x += step;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CopyScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step) {
	for (int i = 0; i < n; i++) {
		const uint8_t* p = src_pixels + (x >> FRAC_BITS) * pf_src.bytes;
		copy_pixel(dst_pixels, p);
		dst_pixels += pf_dst.bytes;
		x += step;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
													 int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
													 int opacity) {
	const int sx0 = src_rect.x;
	const int sy0 = src_rect.y;
	const int sx1 = src_rect.x + src_rect.width;
	const int sy1 = src_rect.y + src_rect.height;

	for (int x = x0; x < x1; x++) {
		double fx, fy;
		inv.Transform(x + 0.5, y + 0.5, fx, fy);
		int xi = (int) floor(fx);
		int yi = (int) floor(fy);
		if (xi < sx0 || xi >= sx1 || yi < sy0 || yi >= sy1)
			;
		else
			blend_pixel(dst_pixels, &src_pixels[yi * src_pitch + xi * pf_src.bytes], opacity);
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OverlayTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
													 int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) {
	const int sx0 = src_rect.x;
	const int sy0 = src_rect.y;
	const int sx1 = src_rect.x + src_rect.width;
	const int sy1 = src_rect.y + src_rect.height;

	for (int x = x0; x < x1; x++) {
		double fx, fy;
		inv.Transform(x + 0.5, y + 0.5, fx, fy);
		int xi = (int) floor(fx);
		int yi = (int) floor(fy);
		if (xi < sx0 || xi >= sx1 || yi < sy0 || yi >= sy1)
			;
		else
			overlay_pixel(dst_pixels, &src_pixels[yi * src_pitch + xi * pf_src.bytes]);
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::CopyTransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
												  int x0, int x1, int y, const Rect& src_rect, const Matrix& inv) {
	const int sx0 = src_rect.x;
	const int sy0 = src_rect.y;
	const int sx1 = src_rect.x + src_rect.width;
	const int sy1 = src_rect.y + src_rect.height;

	for (int x = x0; x < x1; x++) {
		double fx, fy;
		inv.Transform(x + 0.5, y + 0.5, fx, fy);
		int xi = (int) floor(fx);
		int yi = (int) floor(fy);
		if (xi < sx0 || xi >= sx1 || yi < sy0 || yi >= sy1)
			;
		else
			copy_pixel(dst_pixels, &src_pixels[yi * src_pitch + xi * pf_src.bytes]);
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::MaskBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		uint8_t sa = pf_src.get_alpha(src_pixels);
		uint8_t da = pf_dst.get_alpha(dst_pixels);
		uint8_t ra = std::min(sa, da);
		pf_dst.set_alpha(dst_pixels, ra);
		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

static inline void RGB_to_HSL(const uint8_t& r, const uint8_t& g, const uint8_t& b,
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
							  uint8_t &r, uint8_t &g, uint8_t &b)
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

static inline void RGB_adjust_HSL(uint8_t& r, uint8_t& g, uint8_t& b,
								  int hue, int sat, int lmul, int loff) {
	int h, s, l;
	RGB_to_HSL(r, g, b, h, s, l);
	HSL_adjust(h, s, l, hue, sat, lmul, loff);
	HSL_to_RGB(h, s, l, r, g, b);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::HSLBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n,
										int hue, int sat, int lum, int loff) {
	for (int i = 0; i < n; i++) {
		uint8_t r, g, b, a;
		pf_src.get_rgba(src_pixels, r, g, b, a);
		if (a != 0) {
			RGB_adjust_HSL(r, g, b, hue, sat, lum, loff);
			pf_dst.set_rgba(dst_pixels, r, g, b, a);
		}
		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::ToneBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Tone& tone) {
	for (int i = 0; i < n; i++) {
		uint8_t r, g, b, a;
		pf_src.get_rgba(src_pixels, r, g, b, a);
		if (a != 0) {
			r = (uint8_t)std::max(std::min(r + tone.red,   255), 0);
			g = (uint8_t)std::max(std::min(g + tone.green, 255), 0);
			b = (uint8_t)std::max(std::min(b + tone.blue,  255), 0);
		}
		pf_dst.set_rgba(dst_pixels, r, g, b, a);
		dst_pixels += pf_dst.bytes;
		src_pixels += pf_src.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::ToneBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n,
										 const Tone& tone, double factor) {
	for (int i = 0; i < n; i++) {
		uint8_t r, g, b, a;
		pf_src.get_rgba(src_pixels, r, g, b, a);
		if (a != 0) {
			double gray = r * 0.299 + g * 0.587 + b * 0.114;

			r = (uint8_t)std::max(std::min((r - gray) * factor + gray + tone.red   + 0.5, 255.0), 0.0);
			g = (uint8_t)std::max(std::min((g - gray) * factor + gray + tone.green + 0.5, 255.0), 0.0);
			b = (uint8_t)std::max(std::min((b - gray) * factor + gray + tone.blue  + 0.5, 255.0), 0.0);

			pf_dst.set_rgba(dst_pixels, r, g, b, a);
		}

		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::BlendBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, const Color& color) {
	for (int i = 0; i < n; i++) {
		uint8_t r, g, b, a;
		pf_src.get_rgba(src_pixels, r, g, b, a);
		if (a != 0) {
			uint8_t srca = color.alpha;
			uint8_t rr = (uint8_t) ((r * (255 - srca) + color.red   * srca) / PF::ONE);
			uint8_t rg = (uint8_t) ((g * (255 - srca) + color.green * srca) / PF::ONE);
			uint8_t rb = (uint8_t) ((b * (255 - srca) + color.blue  * srca) / PF::ONE);
			uint8_t ra = a;
			pf_dst.set_rgba(dst_pixels, rr, rg, rb, ra);
		}
		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::OpacityChangeBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity) {
	for (int i = 0; i < n; i++) {
		uint8_t a = pf_src.get_alpha(src_pixels);
		a = (uint8_t) ((a * opacity) / pf_src.ONE);
		pf_dst.set_alpha(dst_pixels, a);
		src_pixels += pf_src.bytes;
		dst_pixels += pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipHV(uint8_t*& pixels_first, uint8_t*& pixels_last, int n) {
	for (int i = 0; i < n; i++) {
		if (pixels_first == pixels_last)
			break;

		uint8_t tmp_buffer[4];
		copy_pixel(tmp_buffer, pixels_first);
		copy_pixel(pixels_first, pixels_last);
		copy_pixel(pixels_last, tmp_buffer);

		pixels_first += pf_dst.bytes;
		pixels_last -= pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipH(uint8_t*& pixels_left, uint8_t*& pixels_right, int n) {
	for (int i = 0; i < n; i++) {
		if (pixels_left == pixels_right)
			continue;

		uint8_t tmp_buffer[4];
		copy_pixel(tmp_buffer, pixels_left);
		copy_pixel(pixels_left, pixels_right);
		copy_pixel(pixels_right, tmp_buffer);

		pixels_left += pf_dst.bytes;
		pixels_right -= pf_dst.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::FlipV(uint8_t*& pixels_up, uint8_t*& pixels_down, int n, uint8_t* tmp_buffer) {
	copy_pixels(tmp_buffer, pixels_down, n);
	copy_pixels(pixels_down, pixels_up, n);
	copy_pixels(pixels_up, tmp_buffer, n);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::Blit2x(uint8_t* dst_pixels, const uint8_t* src_pixels, int n) {
	for (int i = 0; i < n; i++) {
		copy_pixel(dst_pixels, src_pixels);
		dst_pixels += pf_dst.bytes;
		copy_pixel(dst_pixels, src_pixels);
		dst_pixels += pf_dst.bytes;
		src_pixels += pf_src.bytes;
	}
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetDstColorKey(uint32_t colorkey) {
	pf_dst.SetColorKey(colorkey);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetSrcColorKey(uint32_t colorkey) {
	pf_src.SetColorKey(colorkey);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetDstFormat(const DynamicFormat& format) {
	pf_dst.SetFormat(format);
}

template <class PFsrc, class PFdst>
void BitmapUtilsT<PFsrc,PFdst>::SetSrcFormat(const DynamicFormat& format) {
	pf_src.SetFormat(format);
}

template <class PFsrc, class PFdst>
const DynamicFormat& BitmapUtilsT<PFsrc,PFdst>::GetDstFormat() const {
	return pf_dst.Format();
}

template <class PFsrc, class PFdst>
const DynamicFormat& BitmapUtilsT<PFsrc,PFdst>::GetSrcFormat() const {
	return pf_src.Format();
}

void BitmapUtils::Blit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int opacity) {
	bool opacity_blit = opacity < 255;
	bool overlay_blit = GetSrcFormat().alpha_type != PF::NoAlpha;

	if (opacity_blit)
		OpacityBlit(dst_pixels, src_pixels, n, opacity);
	else if (overlay_blit)
		OverlayBlit(dst_pixels, src_pixels, n);
	else
		CopyBlit(dst_pixels, src_pixels, n);
}

void BitmapUtils::ScaleBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int n, int x, int step, int opacity) {
	bool opacity_blit = opacity < 255;
	bool overlay_blit = GetSrcFormat().alpha_type != PF::NoAlpha;

	if (opacity_blit)
		OpacityScaleBlit(dst_pixels, src_pixels, n, x, step, opacity);
	else if (overlay_blit)
		OverlayScaleBlit(dst_pixels, src_pixels, n, x, step);
	else
		CopyScaleBlit(dst_pixels, src_pixels, n, x, step);
}

void BitmapUtils::TransformBlit(uint8_t* dst_pixels, const uint8_t* src_pixels, int src_pitch,
								int x0, int x1, int y, const Rect& src_rect, const Matrix& inv,
								int opacity) {
	bool opacity_blit = opacity < 255;
	bool overlay_blit = GetSrcFormat().alpha_type != PF::NoAlpha;

	if (opacity_blit)
		OpacityTransformBlit(dst_pixels, src_pixels, src_pitch, x0, x1, y, src_rect, inv, opacity);
	else if (overlay_blit)
		OverlayTransformBlit(dst_pixels, src_pixels, src_pitch, x0, x1, y, src_rect, inv);
	else
		CopyTransformBlit(dst_pixels, src_pixels, src_pitch, x0, x1, y, src_rect, inv);
}

typedef BitmapUtils* BitmapUtilsP;

static const BitmapUtilsP bitmap_utils[] = {
	// 32-bpp with alpha => similar
	new BitmapUtilsT<format_B8G8R8A8_a,format_B8G8R8A8_a>(),
	new BitmapUtilsT<format_B8G8R8A8_a,format_B8G8R8A8_n>(),
	new BitmapUtilsT<format_B8G8R8A8_n,format_B8G8R8A8_a>(),
	new BitmapUtilsT<format_B8G8R8A8_n,format_B8G8R8A8_n>(),

	new BitmapUtilsT<format_R8G8B8A8_a,format_R8G8B8A8_a>(),
	new BitmapUtilsT<format_R8G8B8A8_a,format_R8G8B8A8_n>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_R8G8B8A8_a>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_R8G8B8A8_n>(),

	new BitmapUtilsT<format_A8B8G8R8_a,format_A8B8G8R8_a>(),
	new BitmapUtilsT<format_A8B8G8R8_a,format_A8B8G8R8_n>(),
	new BitmapUtilsT<format_A8B8G8R8_n,format_A8B8G8R8_a>(),
	new BitmapUtilsT<format_A8B8G8R8_n,format_A8B8G8R8_n>(),

	new BitmapUtilsT<format_A8R8G8B8_a,format_A8R8G8B8_a>(),
	new BitmapUtilsT<format_A8R8G8B8_a,format_A8R8G8B8_n>(),
	new BitmapUtilsT<format_A8R8G8B8_n,format_A8R8G8B8_a>(),
	new BitmapUtilsT<format_A8R8G8B8_n,format_A8R8G8B8_n>(),

	// 32-bpp with color-key => similar

	new BitmapUtilsT<format_B8G8R8A8_k,format_B8G8R8A8_k>(),
	new BitmapUtilsT<format_B8G8R8A8_k,format_B8G8R8A8_n>(),
	new BitmapUtilsT<format_B8G8R8A8_n,format_B8G8R8A8_k>(),

	new BitmapUtilsT<format_R8G8B8A8_k,format_R8G8B8A8_k>(),
	new BitmapUtilsT<format_R8G8B8A8_k,format_R8G8B8A8_n>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_R8G8B8A8_k>(),

	new BitmapUtilsT<format_A8B8G8R8_k,format_A8B8G8R8_k>(),
	new BitmapUtilsT<format_A8B8G8R8_k,format_A8B8G8R8_n>(),
	new BitmapUtilsT<format_A8B8G8R8_n,format_A8B8G8R8_k>(),

	new BitmapUtilsT<format_A8R8G8B8_k,format_A8R8G8B8_k>(),
	new BitmapUtilsT<format_A8R8G8B8_k,format_A8R8G8B8_n>(),
	new BitmapUtilsT<format_A8R8G8B8_n,format_A8R8G8B8_k>(),

	// 32-bpp RGBA (external image format) => dynamic

	new BitmapUtilsT<format_R8G8B8A8_a,format_dynamic_32_a>(),
	new BitmapUtilsT<format_R8G8B8A8_a,format_dynamic_32_k>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_dynamic_32_n>(),

	new BitmapUtilsT<format_R8G8B8A8_a,format_dynamic_24_k>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_dynamic_24_n>(),

	new BitmapUtilsT<format_R8G8B8A8_a,format_dynamic_16_a>(),
	new BitmapUtilsT<format_R8G8B8A8_a,format_dynamic_16_k>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_dynamic_16_n>(),

	// 32-bpp => dynamic no-alpha
	new BitmapUtilsT<format_B8G8R8A8_n,format_dynamic_32_n>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_dynamic_32_n>(),
	new BitmapUtilsT<format_A8B8G8R8_n,format_dynamic_32_n>(),
	new BitmapUtilsT<format_A8R8G8B8_n,format_dynamic_32_n>(),

	new BitmapUtilsT<format_B8G8R8A8_n,format_dynamic_24_n>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_dynamic_24_n>(),
	new BitmapUtilsT<format_A8B8G8R8_n,format_dynamic_24_n>(),
	new BitmapUtilsT<format_A8R8G8B8_n,format_dynamic_24_n>(),

	new BitmapUtilsT<format_B8G8R8A8_n,format_dynamic_16_n>(),
	new BitmapUtilsT<format_R8G8B8A8_n,format_dynamic_16_n>(),
	new BitmapUtilsT<format_A8B8G8R8_n,format_dynamic_16_n>(),
	new BitmapUtilsT<format_A8R8G8B8_n,format_dynamic_16_n>(),

	// 8-bpp L (freetype) => dynamic

	new BitmapUtilsT<format_L8_k,format_dynamic_32_a>(),
	new BitmapUtilsT<format_L8_k,format_dynamic_32_k>(),

	new BitmapUtilsT<format_L8_k,format_dynamic_24_k>(),

	new BitmapUtilsT<format_L8_k,format_dynamic_16_a>(),
	new BitmapUtilsT<format_L8_k,format_dynamic_16_k>(),

	// Dynamic => Dynamic

	new BitmapUtilsT<format_dynamic_32_a,format_dynamic_32_a>(),
	new BitmapUtilsT<format_dynamic_32_a,format_dynamic_32_n>(),
	new BitmapUtilsT<format_dynamic_32_n,format_dynamic_32_a>(),
	new BitmapUtilsT<format_dynamic_32_n,format_dynamic_32_n>(),

	new BitmapUtilsT<format_dynamic_32_k,format_dynamic_32_k>(),
	new BitmapUtilsT<format_dynamic_32_k,format_dynamic_32_n>(),
	new BitmapUtilsT<format_dynamic_32_n,format_dynamic_32_k>(),
	new BitmapUtilsT<format_dynamic_32_n,format_dynamic_32_n>(),

	new BitmapUtilsT<format_dynamic_24_k,format_dynamic_24_k>(),
	new BitmapUtilsT<format_dynamic_24_k,format_dynamic_24_n>(),
	new BitmapUtilsT<format_dynamic_24_n,format_dynamic_24_k>(),
	new BitmapUtilsT<format_dynamic_24_n,format_dynamic_24_n>(),

	new BitmapUtilsT<format_dynamic_16_a,format_dynamic_16_a>(),
	new BitmapUtilsT<format_dynamic_16_a,format_dynamic_16_n>(),
	new BitmapUtilsT<format_dynamic_16_n,format_dynamic_16_a>(),
	new BitmapUtilsT<format_dynamic_16_n,format_dynamic_16_n>(),

	new BitmapUtilsT<format_dynamic_16_k,format_dynamic_16_k>(),
	new BitmapUtilsT<format_dynamic_16_k,format_dynamic_16_n>(),
	new BitmapUtilsT<format_dynamic_16_n,format_dynamic_16_k>(),
	new BitmapUtilsT<format_dynamic_16_n,format_dynamic_16_n>(),

	(BitmapUtilsP) NULL
};

std::map<int, BitmapUtils*> BitmapUtils::unary_map;
typedef std::pair<int, int> int_pair;
std::map<int_pair, BitmapUtils*> BitmapUtils::binary_map;
bool BitmapUtils::maps_initialized = false;

BitmapUtils* BitmapUtils::Create(const DynamicFormat& dst_format,
								 const DynamicFormat& src_format,
								 bool need_source) {
	if (!maps_initialized) {
		for (const BitmapUtilsP* pp = bitmap_utils; *pp != NULL; pp++) {
			BitmapUtilsP p = *pp;
			const DynamicFormat& dfmt = p->GetDstFormat();
			const DynamicFormat& sfmt = p->GetSrcFormat();
			int dkey = dfmt.code(true);
			int skey = sfmt.code(true);
			unary_map[dkey] = p;
			binary_map[int_pair(dkey, skey)] = p;
		}

		maps_initialized = true;
	}

	if (need_source) {
		std::map<int_pair, BitmapUtils*>::const_iterator it;

		// static dst, static src
		int_pair key1(dst_format.code(true), src_format.code(true));
		it = binary_map.find(key1);
		if (it != binary_map.end())
			return it->second;

		// dynamic dst, static src
		int_pair key2(dst_format.code(false), src_format.code(true));
		it = binary_map.find(key2);
		if (it != binary_map.end()) {
			it->second->SetDstFormat(dst_format);
			return it->second;
		}

		// static dst, dynamic src
		int_pair key3(dst_format.code(true), src_format.code(false));
		it = binary_map.find(key3);
		if (it != binary_map.end()) {
			it->second->SetSrcFormat(src_format);
			return it->second;
		}

		// dynamic dst, dynamic src
		int_pair key4(dst_format.code(false), src_format.code(false));
		it = binary_map.find(key4);
		if (it != binary_map.end()) {
			it->second->SetDstFormat(dst_format);
			it->second->SetSrcFormat(src_format);
			return it->second;
		}
	}
	else {
		std::map<int, BitmapUtils*>::const_iterator it;

		int key1 = dst_format.code(true);
		it = unary_map.find(key1);
		if (it != unary_map.end())
			return it->second;

		int key2 = dst_format.code(false);
		it = unary_map.find(key2);
		if (it != unary_map.end()) {
			it->second->SetDstFormat(dst_format);
			return it->second;
		}
	}

	Output::Error("Couldn't find a renderer");

	return NULL;
}
