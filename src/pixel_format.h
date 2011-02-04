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

#ifndef _EASYRPG_PIXEL_FORMAT_H_
#define _EASYRPG_PIXEL_FORMAT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cstdlib>
#include <algorithm>
#include "system.h"

////////////////////////////////////////////////////////////
/// Component struct
////////////////////////////////////////////////////////////
struct Component {
	uint8 bits;
	uint8 shift;
	uint8 byte;
	uint32 mask;

	static inline int count_bits(uint32 mask) {
		int count = 0;
		if ((mask & 0xFFFF0000) != 0)
			count += 16, mask >>= 16;
		if ((mask & 0xFF00) != 0)
			count += 8, mask >>= 8;
		if ((mask & 0xF0) != 0)
			count += 4, mask >>= 4;
		if ((mask & 0xC) != 0)
			count += 2, mask >>= 2;
		if ((mask & 0x2) != 0)
			count += 1, mask >>= 1;
		if ((mask & 0x1) != 0)
			count++;
		return count;
	}

	inline void convert_mask() {
		int bit_count = count_bits(mask);
		uint32 mask_ex = (1U << bit_count) - 1;
		uint32 mask_lo = mask_ex - mask;
		shift = (uint8)count_bits(mask_lo);
		bits = (uint8)(bit_count - shift);
		byte = shift / 8;
	}

	inline bool operator==(const Component& c) {
		return mask == c.mask;
	}

	inline bool operator!=(const Component& c) {
		return mask != c.mask;
	}

	Component() {}

	Component(unsigned int bits, unsigned int shift) :
		bits((uint8)bits),
		shift((uint8)shift),
		byte((uint8)(shift / 8)),
		mask(((1 << bits)-1) << shift) {}

	Component(uint32 mask) :
		mask(mask) { convert_mask(); }
};

////////////////////////////////////////////////////////////
/// DynamicFormat struct
////////////////////////////////////////////////////////////
struct DynamicFormat {
	Component r, g, b, a;
	uint32 colorkey;
	bool has_alpha;
	bool has_colorkey;

	DynamicFormat() {}

	DynamicFormat(int rb, int rs,
				  int gb, int gs,
				  int bb, int bs,
				  int ab, int as,
				  uint32 colorkey,
				  bool has_colorkey) :
		r(rb, rs), g(gb, gs), b(bb, bs), a(ab, as),
		colorkey(colorkey),
		has_alpha(ab > 0),
		has_colorkey(has_colorkey) {}

	DynamicFormat(uint32 rmask,
				  uint32 gmask,
				  uint32 bmask,
				  uint32 amask,
				  uint32 colorkey,
				  bool has_colorkey) :
		r(rmask), g(gmask), b(bmask), a(amask),
		colorkey(colorkey),
		has_alpha(amask != 0),
		has_colorkey(has_colorkey) {}

	void Set(int rb, int rs,
			 int gb, int gs,
			 int bb, int bs,
			 int ab, int as,
			 uint32 colorkey,
			 bool _has_colorkey) {
		r = Component(rb, rs);
		g = Component(gb, gs);
		b = Component(bb, bs);
		a = Component(ab, as);
		colorkey = colorkey;
		has_alpha = ab > 0;
		has_colorkey = _has_colorkey;
	}

	void Set(uint32 rmask,
			 uint32 gmask,
			 uint32 bmask,
			 uint32 amask,
			 uint32 _colorkey,
			 bool _has_colorkey) {
		r = Component(rmask);
		g = Component(gmask);
		b = Component(bmask);
		a = Component(amask);
		colorkey = _colorkey;
		has_alpha = amask != 0;
		has_colorkey = _has_colorkey;
	}

	void SetColorKey(int _colorkey) {
		colorkey = _colorkey;
		has_colorkey = true;
	}

	inline bool operator==(const DynamicFormat& f) {
		return r ==  f.r && g == f.g && b == f.b && a == f.a;
	}

	inline bool operator!=(const DynamicFormat& f) {
		return r !=  f.r || g != f.g || b != f.b || a != f.a;
	}

	inline uint8 opaque(void) {
		return (a.bits > 0) ? (uint8)(((1 << a.bits) - 1) << (8 - a.bits)) : 255;
	}
};

////////////////////////////////////////////////////////////
/// PixelFormat class
////////////////////////////////////////////////////////////

#define NoAlpha false
#define HasAlpha true

#define NoColorkey false
#define HasColorkey true

#define StaticMasks false
#define DynamicMasks true

#define StaticAlpha false
#define DynamicAlpha true

#define NotAligned false
#define IsAligned true

////////////////////////////////////////////////////////////
/// Bits traits
////////////////////////////////////////////////////////////

template <class PF, int bits>
struct bits_traits {
};

template <class PF>
struct bits_traits<PF, 16> {
	static inline uint32 get_uint32(const uint8* p) {
		return (uint32)*(const uint16*)p;
	}
	static inline void set_uint32(uint8* p, uint32 pix) {
		*(uint16*)p = (uint16) pix;
	}
	static inline void copy_pixel(uint8* dst, const uint8* src) {
		*(uint16*)dst = *(const uint16*)src;
	}
	static inline void set_pixels(uint8* dst, const uint8* src, int n) {
		uint16 pixel = (uint16) get_uint32(src);
		uint16* dst_pix = (uint16*) dst;
		std::fill(dst_pix, dst_pix + n, pixel);
	}
};

template <class PF>
struct bits_traits<PF, 24> {
	static inline uint32 get_uint32(const uint8* p) {
		return
			((uint32)(p[PF::endian(2)]) << 16) |
			((uint32)(p[PF::endian(1)]) <<  8) |
			((uint32)(p[PF::endian(0)]) <<  0);
	}
	static inline void set_uint32(uint8* p, uint32 pix) {
		p[PF::endian(0)] = (pix >>  0) & 0xFF;
		p[PF::endian(1)] = (pix >>  8) & 0xFF;
		p[PF::endian(2)] = (pix >> 16) & 0xFF;
	}
	static inline void copy_pixel(uint8* dst, const uint8* src) {
		memcpy(dst, src, 3);
	}
	static inline void set_pixels(uint8* dst, const uint8* src, int n) {
		for (int i = 0; i < n; i++)
			copy_pixel(dst + i * 3, src);
	}
};

template <class PF>
struct bits_traits<PF, 32> {
	static inline uint32 get_uint32(const uint8* p) {
		return *(const uint32*) p;
	}
	static inline void set_uint32(uint8* p, uint32 pix) {
		*(uint32*)p = pix;
	}
	static inline void copy_pixel(uint8* dst, const uint8* src) {
		*(uint32*)dst = *(const uint32*)src;
	}
	static inline void set_pixels(uint8* dst, const uint8* src, int n) {
		uint32 pixel = get_uint32(src);
		uint32* dst_pix = (uint32*) dst;
		std::fill(dst_pix, dst_pix + n, pixel);
	}
};

////////////////////////////////////////////////////////////
/// Alpha traits
////////////////////////////////////////////////////////////

// general case
template<class PF,
		 bool aligned,
		 bool dynamic_alpha,
		 bool alpha_flag,
		 bool colorkey>
struct alpha_traits {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		uint8 r, g, b, a;
		PF::get_rgba(format, p, r, g, b, a);
		return a;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		uint8 r, g, b, a;
		PF::get_rgba(format, p, r, g, b, a);
		PF::set_rgba(format, p, r, g, b, alpha);
	}
	static inline void overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
									 const DynamicFormat& src_format, const uint8* src) {
		PF::default_overlay_pixel(dst_format, dst, src_format, src);
	}
};

// dynamic, colorkey
template <class PF, bool aligned>
struct alpha_traits<PF, aligned, DynamicAlpha, NoAlpha, HasColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		uint32 pix = PF::get_uint32(p);
		return (PF::has_colorkey(format) && pix == format.colorkey) ? 0 : 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		if (PF::has_colorkey(format) && alpha == 0)
			PF::set_uint32(p, format.colorkey);
	}
	static inline void overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
									 const DynamicFormat& src_format, const uint8* src) {
		if (PF::get_alpha(src_format, src) != 0)
			PF::copy_pixel(dst, src);
	}
};

// colorkey
template <class PF, bool aligned>
struct alpha_traits<PF, aligned, StaticAlpha, NoAlpha, HasColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		uint32 pix = PF::get_uint32(p);
		return (pix == format.colorkey) ? 0 : 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		if (alpha == 0)
			PF::set_uint32(p, format.colorkey);
	}
	static inline void overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
									 const DynamicFormat& src_format, const uint8* src) {
		if (PF::get_uint32(src) != src_format.colorkey)
			PF::copy_pixel(dst, src);
	}
};

// no alpha or colorkey
template<class PF, bool aligned>
struct alpha_traits<PF, aligned, StaticAlpha, NoAlpha, NoColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
	}
	static inline void overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
									 const DynamicFormat& src_format, const uint8* src) {
		PF::copy_pixel(dst, src);
	}
};

// aligned, with alpha
template<class PF, bool dynamic_alpha>
struct alpha_traits<PF, IsAligned, dynamic_alpha, HasAlpha, HasColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return PF::has_alpha(format) ? p[PF::a_byte(format)] : 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		if (PF::has_alpha(format))
			p[PF::a_byte(format)] = alpha;
	}
	static inline void overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
									 const DynamicFormat& src_format, const uint8* src) {
		PF::default_overlay_pixel(dst_format, dst, src_format, src);
	}
};

////////////////////////////////////////////////////////////
/// RGBA traits
////////////////////////////////////////////////////////////

template<class PF,
		 bool aligned,
		 bool dynamic_alpha,
		 bool alpha,
		 bool colorkey>
struct rgba_traits {
};

// aligned, has alpha
template<class PF, bool dynamic_alpha>
struct rgba_traits<PF, IsAligned, dynamic_alpha, HasAlpha, NoColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::r_byte(format)];
		g = p[PF::g_byte(format)];
		b = p[PF::b_byte(format)];
		a = PF::has_alpha(format) ? p[PF::a_byte(format)] : 255;
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		p[PF::r_byte(format)] = r;
		p[PF::g_byte(format)] = g;
		p[PF::b_byte(format)] = b;
		if (PF::has_alpha(format))
			p[PF::a_byte(format)] = a;
	}
};

// aligned, has colorkey
template<class PF, bool dynamic_alpha>
struct rgba_traits<PF, IsAligned, dynamic_alpha, NoAlpha, HasColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::r_byte(format)];
		g = p[PF::g_byte(format)];
		b = p[PF::b_byte(format)];
		a = (PF::has_colorkey(format) && PF::get_uint32(p) == format.colorkey) ? 0 : 255;
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		if (PF::has_colorkey(format) && a == 0)
			PF::set_uint32(p, format.colorkey);
		else {
			p[PF::r_byte(format)] = r;
			p[PF::g_byte(format)] = g;
			p[PF::b_byte(format)] = b;
		}
	}
};

// aligned, no alpha or colorkey
template<class PF>
struct rgba_traits<PF, IsAligned, StaticAlpha, NoAlpha, NoColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::r_byte(format)];
		g = p[PF::g_byte(format)];
		b = p[PF::b_byte(format)];
		a = 255;
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		p[PF::r_byte(format)] = r;
		p[PF::g_byte(format)] = g;
		p[PF::b_byte(format)] = b;
	}
};

// unaligned, has alpha (dynamic)
template<class PF, bool dynamic_alpha>
struct rgba_traits<PF, NotAligned, dynamic_alpha, HasAlpha, NoColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		PF::uint32_to_rgba(format, pix, r, g, b, a);
		if (!PF::has_alpha(format))
			a = 255;
	}
	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = PF::rgba_to_uint32(format, r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

// unaligned, has colorkey (dynamic)
template<class PF, bool dynamic_alpha>
struct rgba_traits<PF, NotAligned, dynamic_alpha, NoAlpha, HasColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		if (PF::has_colorkey(format) && pix == format.colorkey)
			a = 0;
		PF::uint32_to_rgba(format, pix, r, g, b, a);
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = (PF::has_colorkey(format) && a == 0)
			? format.colorkey
			: PF::rgba_to_uint32(format, r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

// unaligned, no alpha or colorkey
template<class PF>
struct rgba_traits<PF, NotAligned, StaticAlpha, NoAlpha, NoColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		PF::uint32_to_rgba(format, pix, r, g, b, a);
		a = 255;
	}
	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = PF::rgba_to_uint32(format, r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

////////////////////////////////////////////////////////////
/// Mask traits
////////////////////////////////////////////////////////////

template<class PF, bool dynamic, int _bits, int _shift>
struct mask_traits {
};

template<class PF, int _bits, int _shift>
struct mask_traits<PF, StaticMasks, _bits, _shift> {
	static const int _byte = _shift / 8;
	static const int _mask = ((1 << _bits)-1) << _shift;
	static inline int bits(const Component& c) { return _bits; }
	static inline int shift(const Component& c) { return _shift; }
	static inline int byte(const Component& c) { return _byte; }
	static inline int mask(const Component& c) { return _mask; }
};

template<class PF, int _bits, int _shift>
struct mask_traits<PF, DynamicMasks, _bits, _shift> {
	static inline int bits(const Component& c) { return c.bits; }
	static inline int shift(const Component& c) { return c.shift; }
	static inline int byte(const Component& c) { return c.byte; }
	static inline int mask(const Component& c) { return c.mask; }
};

////////////////////////////////////////////////////////////
/// PixelFormat class
////////////////////////////////////////////////////////////

template <int BITS,
		  bool DYNAMIC_MASKS, bool DYNAMIC_ALPHA, bool HAS_ALPHA, bool COLORKEY, bool ALIGNED,
		  int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS>
class PixelFormat {
public:
	typedef class PixelFormat<BITS,DYNAMIC_MASKS,DYNAMIC_ALPHA,HAS_ALPHA,COLORKEY,ALIGNED,
							  RB,RS,GB,GS,BB,BS,AB,AS> my_type;

	static const int bits = BITS;
	static const int bytes = (BITS + 7) / 8;

	static const bool dynamic_masks = DYNAMIC_MASKS;
	static const bool dynamic_alpha = DYNAMIC_ALPHA;
	static const bool alpha = HAS_ALPHA;
	static const bool colorkey = COLORKEY;
	static const bool aligned = ALIGNED;

	static const int ONE = 255;
	// static const int ONE = 256; // faster but less accurate

	typedef bits_traits<my_type, bits> bits_traits_type;
	typedef alpha_traits<my_type,aligned,dynamic_alpha,alpha,colorkey> alpha_traits_type;
	typedef rgba_traits<my_type,aligned,dynamic_alpha,alpha,colorkey> rgba_traits_type;
	typedef mask_traits<my_type,dynamic_masks,RB,RS> mask_r_traits_type;
	typedef mask_traits<my_type,dynamic_masks,GB,GS> mask_g_traits_type;
	typedef mask_traits<my_type,dynamic_masks,BB,BS> mask_b_traits_type;
	typedef mask_traits<my_type,dynamic_masks,AB,AS> mask_a_traits_type;

	static inline int endian(int byte) {
#ifndef USE_BIG_ENDIAN
		return byte;
#else
		return bytes - 1 - byte;
#endif
	}

	static inline void uint32_to_rgba(const DynamicFormat& format, uint32 pix, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = (uint8)(((pix >> r_shift(format)) & ((1 << r_bits(format)) - 1)) << (8 - r_bits(format)));
		g = (uint8)(((pix >> g_shift(format)) & ((1 << g_bits(format)) - 1)) << (8 - g_bits(format)));
		b = (uint8)(((pix >> b_shift(format)) & ((1 << b_bits(format)) - 1)) << (8 - b_bits(format)));
		a = (uint8)(((pix >> a_shift(format)) & ((1 << a_bits(format)) - 1)) << (8 - a_bits(format)));
	}

	static inline uint32 rgba_to_uint32(const DynamicFormat& format, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		return
			(((uint32)r >> (8 - r_bits(format))) << r_shift(format)) | 
			(((uint32)g >> (8 - g_bits(format))) << g_shift(format)) | 
			(((uint32)b >> (8 - b_bits(format))) << b_shift(format)) | 
			(((uint32)a >> (8 - a_bits(format))) << a_shift(format));
	}

	static inline int r_byte(const DynamicFormat& format) {		return endian(mask_r_traits_type::byte(format.r));	}
	static inline int g_byte(const DynamicFormat& format) {		return endian(mask_g_traits_type::byte(format.g));	}
	static inline int b_byte(const DynamicFormat& format) {		return endian(mask_b_traits_type::byte(format.b));	}
	static inline int a_byte(const DynamicFormat& format) {		return endian(mask_a_traits_type::byte(format.a));	}

	static inline uint32 r_mask(const DynamicFormat& format) {		return mask_r_traits_type::mask(format.r);	}
	static inline uint32 g_mask(const DynamicFormat& format) {		return mask_g_traits_type::mask(format.g);	}
	static inline uint32 b_mask(const DynamicFormat& format) {		return mask_b_traits_type::mask(format.b);	}
	static inline uint32 a_mask(const DynamicFormat& format) {		return mask_a_traits_type::mask(format.a);	}

	static inline int r_bits(const DynamicFormat& format) {		return mask_r_traits_type::bits(format.r);	}
	static inline int g_bits(const DynamicFormat& format) {		return mask_g_traits_type::bits(format.g);	}
	static inline int b_bits(const DynamicFormat& format) {		return mask_b_traits_type::bits(format.b);	}
	static inline int a_bits(const DynamicFormat& format) {		return mask_a_traits_type::bits(format.a);	}

	static inline int r_shift(const DynamicFormat& format) {	return mask_r_traits_type::shift(format.r);	}
	static inline int g_shift(const DynamicFormat& format) {	return mask_g_traits_type::shift(format.g);	}
	static inline int b_shift(const DynamicFormat& format) {	return mask_b_traits_type::shift(format.b);	}
	static inline int a_shift(const DynamicFormat& format) {	return mask_a_traits_type::shift(format.a);	}

	static inline bool has_alpha(const DynamicFormat& format) {
		return dynamic_alpha ? format.has_alpha : alpha;
	}

	static inline bool has_colorkey(const DynamicFormat& format) {
		return dynamic_alpha ? format.has_colorkey : colorkey;
	}

	static inline uint32 get_uint32(const uint8* p) {
		return bits_traits_type::get_uint32(p);
	}

	static inline void set_uint32(uint8* p, uint32 pix) {
		bits_traits_type::set_uint32(p, pix);
	}

	static inline void copy_pixel(uint8* dst, const uint8* src) {
		bits_traits_type::copy_pixel(dst, src);
	}

	static inline void copy_pixels(uint8* dst, const uint8* src, int n) {
		memcpy(dst, src, n * bytes);
	}

	static inline void set_pixels(uint8* dst, const uint8* src, int n) {
		bits_traits_type::set_pixels(dst, src, n);
	}

	static inline uint8 opaque(const DynamicFormat& format) {
		return (uint8) (0xFF << (8 - a_bits(format)));
	}

	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return alpha_traits_type::get_alpha(format, p);
	}

	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		alpha_traits_type::set_alpha(format, p, alpha);
	}

	static inline void overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
									 const DynamicFormat& src_format, const uint8* src) {
		alpha_traits_type::overlay_pixel(dst_format, dst, src_format, src);
	}

	static inline void default_overlay_pixel(const DynamicFormat& dst_format, uint8* dst,
											 const DynamicFormat& src_format, const uint8* src) {
		if (get_alpha(src_format, src) >= opaque(src_format))
			copy_pixel(dst, src);
		else {
			uint8 sr, sg, sb, sa;
			get_rgba(src_format, src, sr, sg, sb, sa);
			uint8 dr, dg, db, da;
			get_rgba(dst_format, dst, dr, dg, db, da);
			int srca = (int) sa;
			uint8 rr, rg, rb, ra;

			if (da >= opaque(dst_format)) {
				rr = (uint8) ((dr * (255 - srca) + sr * srca) / ONE);
				rg = (uint8) ((dg * (255 - srca) + sg * srca) / ONE);
				rb = (uint8) ((db * (255 - srca) + sb * srca) / ONE);
				ra = (uint8) ((da * (255 - srca)) / ONE + srca);
			}
			else if (da == 0) {
				rr = (uint8) sr;
				rg = (uint8) sg;
				rb = (uint8) sb;
				ra = (uint8) sa;
			}
			else {
				int dsta = (int) da;
				int resa = 255 - (255 - dsta) * (255 - srca) / ONE;
				rr = (uint8) ((dr * dsta * (255 - srca) + sr * srca) / resa);
				rg = (uint8) ((dg * dsta * (255 - srca) + sg * srca) / resa);
				rb = (uint8) ((db * dsta * (255 - srca) + sb * srca) / resa);
				ra = (uint8) resa;
			}

			set_rgba(dst_format, dst, rr, rg, rb, ra);
		}
	}

	static inline void overlay_pixels(const DynamicFormat& dst_format, uint8* dst,
									  const DynamicFormat& src_format, const uint8* src, int n) {
		if (has_colorkey(src_format) || has_alpha(src_format))
			for (int i = 0; i < n; i++)
				overlay_pixel(dst_format, dst + i * bytes, src_format, src + i * bytes);
		else
			copy_pixels(dst, src, n);
	}

	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		rgba_traits_type::get_rgba(format, p, r, g, b, a);
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		rgba_traits_type::set_rgba(format, p, r, g, b, a);
	}

	static inline bool match(const DynamicFormat& ref, const DynamicFormat& format) {
		return
			format.r.mask == r_mask(ref) &&
			format.g.mask == g_mask(ref) &&
			format.b.mask == b_mask(ref) &&
			format.a.mask == a_mask(ref);
	}

	static inline DynamicFormat Format(const DynamicFormat& format) {
		return DynamicFormat(
			r_bits(format), r_mask(format),
			g_bits(format), g_mask(format),
			b_bits(format), b_mask(format),
			a_bits(format), a_mask(format),
			format.colorkey,
			has_colorkey(format));
	}
};

#ifndef USE_BIG_ENDIAN
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,16,8,8,8,0,8,24> format_B8G8R8A8;
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,0,8,8,8,16,8,24> format_R8G8B8A8;
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,24,8,16,8,8,8,0> format_A8B8G8R8;
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,8,8,16,8,24,8,0> format_A8R8G8B8;
#else
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,8,8,16,8,24,8,0> format_B8G8R8A8;
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,24,8,16,8,8,8,0> format_R8G8B8A8;
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,0,8,8,8,16,8,24> format_A8B8G8R8;
typedef PixelFormat<32,StaticMasks,StaticAlpha,HasAlpha,NoColorkey,IsAligned,8,16,8,8,8,0,8,24> format_A8R8G8B8;
#endif

typedef PixelFormat<32,DynamicMasks,DynamicAlpha,HasAlpha,NoColorkey,IsAligned,0,0,0,0,0,0,0,0> format_dynamic_32_a;
typedef PixelFormat<32,DynamicMasks,DynamicAlpha,NoAlpha,HasColorkey,IsAligned,0,0,0,0,0,0,0,0> format_dynamic_32_ck;

typedef PixelFormat<24,DynamicMasks,DynamicAlpha,NoAlpha,HasColorkey,IsAligned,0,0,0,0,0,0,0,0> format_dynamic_24_ck;

typedef PixelFormat<16,DynamicMasks,DynamicAlpha,HasAlpha,NoColorkey,NotAligned,0,0,0,0,0,0,0,0> format_dynamic_16_a;
typedef PixelFormat<16,DynamicMasks,DynamicAlpha,NoAlpha,HasColorkey,NotAligned,0,0,0,0,0,0,0,0> format_dynamic_16_ck;

typedef PixelFormat<32,DynamicMasks,StaticAlpha,NoAlpha,NoColorkey,IsAligned,0,0,0,0,0,0,0,0> format_dynamic_32;
typedef PixelFormat<24,DynamicMasks,StaticAlpha,NoAlpha,NoColorkey,IsAligned,0,0,0,0,0,0,0,0> format_dynamic_24;
typedef PixelFormat<16,DynamicMasks,StaticAlpha,NoAlpha,NoColorkey,NotAligned,0,0,0,0,0,0,0,0> format_dynamic_16;

////////////////////////////////////////////////////////////

#undef NoAlpha
#undef HasAlpha

#undef NoColorkey
#undef HasColorkey

#undef StaticMasks
#undef DynamicMasks

#undef StaticAlpha
#undef DynamicAlpha

#undef NotAligned
#undef IsAligned

////////////////////////////////////////////////////////////

#endif

