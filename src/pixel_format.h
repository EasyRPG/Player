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
		shift = count_bits(mask_lo);
		bits = bit_count - shift;
		byte = shift / 8;
	}

	Component() {}

	Component(unsigned int bits, unsigned int shift) :
		bits(bits),
		shift(shift),
		byte(shift / 8),
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

	DynamicFormat() {}

	DynamicFormat(int rb, int rs,
				  int gb, int gs,
				  int bb, int bs,
				  int ab, int as,
				  int colorkey) :
		r(rb, rs), g(gb, gs), b(bb, bs), a(ab, as), colorkey(colorkey) {}

	DynamicFormat(uint32 rmask,
				  uint32 gmask,
				  uint32 bmask,
				  uint32 amask,
				  int colorkey) :
		r(rmask), g(gmask), b(bmask), a(amask),
		colorkey(colorkey) {}

	void Set(int rb, int rs,
			 int gb, int gs,
			 int bb, int bs,
			 int ab, int as,
			 int colorkey) {
		r = Component(rb, rs);
		g = Component(gb, gs);
		b = Component(bb, bs);
		a = Component(ab, as);
		colorkey = colorkey;
	}

	void Set(uint32 rmask,
			 uint32 gmask,
			 uint32 bmask,
			 uint32 amask,
			 int colorkey) {
		r = Component(rmask);
		g = Component(gmask);
		b = Component(bmask);
		a = Component(amask);
		colorkey = colorkey;
	}
};

////////////////////////////////////////////////////////////
/// PixelFormat class
////////////////////////////////////////////////////////////

#define NoAlpha false
#define HasAlpha true

#define NoColorkey false
#define HasColorkey true

#define NotDynamic false
#define IsDynamic true

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
		 bool has_alpha,
		 bool has_colorkey>
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
};

// colorkey
template <class PF, bool aligned>
struct alpha_traits<PF, aligned, NoAlpha, HasColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		uint32 pix = PF::get_uint32(p);
		return (pix == format.colorkey) ? 0 : 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		if (alpha == 0)
			PF::set_uint32(p, format.colorkey);
	}
};

// no alpha or colorkey
template<class PF, bool aligned>
struct alpha_traits<PF, aligned, NoAlpha, NoColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
	}
};

// aligned, with alpha
template<class PF>
struct alpha_traits<PF, IsAligned, HasAlpha, HasColorkey> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return p[PF::a_byte(format)];
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		p[PF::a_byte(format)] = alpha;
	}
};

////////////////////////////////////////////////////////////
/// RGBA traits
////////////////////////////////////////////////////////////

template<class PF,
		 bool aligned,
		 bool has_alpha,
		 bool has_colorkey>
struct rgba_traits {
};

// aligned, has alpha
template<class PF>
struct rgba_traits<PF, IsAligned, HasAlpha, NoColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::r_byte(format)];
		g = p[PF::g_byte(format)];
		b = p[PF::b_byte(format)];
		a = p[PF::a_byte(format)];
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		p[PF::r_byte(format)] = r;
		p[PF::g_byte(format)] = g;
		p[PF::b_byte(format)] = b;
		p[PF::a_byte(format)] = a;
	}
};

// aligned, has colorkey
template<class PF>
struct rgba_traits<PF, IsAligned, NoAlpha, HasColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::r_byte(format)];
		g = p[PF::g_byte(format)];
		b = p[PF::b_byte(format)];
		a = (PF::get_uint32(p) == format.colorkey) ? 0 : 255;
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		if (a == 0)
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
struct rgba_traits<PF, IsAligned, NoAlpha, NoColorkey> {
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

// unaligned, has alpha
template<class PF>
struct rgba_traits<PF, NotAligned, HasAlpha, NoColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		PF::uint32_to_rgba(format, pix, r, g, b, a);
	}
	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = PF::rgba_to_uint32(format, r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

// unaligned, has colorkey
template<class PF>
struct rgba_traits<PF, NotAligned, NoAlpha, HasColorkey> {
	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		if (pix == format.colorkey)
			a = 0;
		PF::uint32_to_rgba(format, pix, r, g, b, a);
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = (a == 0) ? format.colorkey : PF::rgba_to_uint32(format, r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

// unaligned, no alpha or colorkey
template<class PF>
struct rgba_traits<PF, NotAligned, NoAlpha, NoColorkey> {
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
struct mask_traits<PF, NotDynamic, _bits, _shift> {
	static const int _byte = _shift / 8;
	static const int _mask = ((1 << _bits)-1) << _shift;
	static inline int bits(const Component& c) { return _bits; }
	static inline int shift(const Component& c) { return _shift; }
	static inline int byte(const Component& c) { return _byte; }
	static inline int mask(const Component& c) { return _mask; }
};

template<class PF, int _bits, int _shift>
struct mask_traits<PF, IsDynamic, _bits, _shift> {
	static inline int bits(const Component& c) { return c.bits; }
	static inline int shift(const Component& c) { return c.shift; }
	static inline int byte(const Component& c) { return c.byte; }
	static inline int mask(const Component& c) { return c.mask; }
};

////////////////////////////////////////////////////////////
/// PixelFormat class
////////////////////////////////////////////////////////////

template <int BITS,
		  bool DYNAMIC, bool HAS_ALPHA, bool COLORKEY, bool ALIGNED,
		  int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS>
class PixelFormat {
public:
	typedef class PixelFormat<BITS,DYNAMIC,HAS_ALPHA,COLORKEY,ALIGNED,
							  RB,RS,GB,GS,BB,BS,AB,AS> my_type;

	static const int bits = BITS;
	static const int bytes = (BITS + 7) / 8;

	static const bool dynamic = DYNAMIC;
	static const bool has_alpha = HAS_ALPHA;
	static const bool has_colorkey = COLORKEY;
	static const bool aligned = ALIGNED;

	typedef bits_traits<my_type, bits> bits_traits_type;
	typedef alpha_traits<my_type,aligned,has_alpha,has_colorkey> alpha_traits_type;
	typedef rgba_traits<my_type,aligned,has_alpha,has_colorkey> rgba_traits_type;
	typedef mask_traits<my_type,dynamic,RB,RS> mask_r_traits_type;
	typedef mask_traits<my_type,dynamic,GB,GS> mask_g_traits_type;
	typedef mask_traits<my_type,dynamic,BB,BS> mask_b_traits_type;
	typedef mask_traits<my_type,dynamic,AB,AS> mask_a_traits_type;

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

	static inline int r_mask(const DynamicFormat& format) {		return mask_r_traits_type::mask(format.r);	}
	static inline int g_mask(const DynamicFormat& format) {		return mask_g_traits_type::mask(format.g);	}
	static inline int b_mask(const DynamicFormat& format) {		return mask_b_traits_type::mask(format.b);	}
	static inline int a_mask(const DynamicFormat& format) {		return mask_a_traits_type::mask(format.a);	}

	static inline int r_bits(const DynamicFormat& format) {		return mask_r_traits_type::bits(format.r);	}
	static inline int g_bits(const DynamicFormat& format) {		return mask_g_traits_type::bits(format.g);	}
	static inline int b_bits(const DynamicFormat& format) {		return mask_b_traits_type::bits(format.b);	}
	static inline int a_bits(const DynamicFormat& format) {		return mask_a_traits_type::bits(format.a);	}

	static inline int r_shift(const DynamicFormat& format) {	return mask_r_traits_type::shift(format.r);	}
	static inline int g_shift(const DynamicFormat& format) {	return mask_g_traits_type::shift(format.g);	}
	static inline int b_shift(const DynamicFormat& format) {	return mask_b_traits_type::shift(format.b);	}
	static inline int a_shift(const DynamicFormat& format) {	return mask_a_traits_type::shift(format.a);	}

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

	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return alpha_traits_type::get_alpha(format, p);
	}

	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		alpha_traits_type::set_alpha(format, p, alpha);
	}

	static inline void get_rgba(const DynamicFormat& format, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		rgba_traits_type::get_rgba(format, p, r, g, b, a);
	}

	static inline void set_rgba(const DynamicFormat& format, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		rgba_traits_type::set_rgba(format, p, r, g, b, a);
	}
};

#ifndef USE_BIG_ENDIAN
typedef PixelFormat<32,NotDynamic,HasAlpha,NoColorkey,IsAligned,8,16,8,8,8,0,8,24> format_B8G8R8A8;
typedef PixelFormat<32,NotDynamic,HasAlpha,NoColorkey,IsAligned,8,0,8,8,8,16,8,24> format_R8G8B8A8;
#else
typedef PixelFormat<32,NotDynamic,HasAlpha,NoColorkey,IsAligned,8,8,8,16,8,24,8,0> format_B8G8R8A8;
typedef PixelFormat<32,NotDynamic,HasAlpha,NoColorkey,IsAligned,8,24,8,16,8,8,8,0> format_R8G8B8A8;
#endif

////////////////////////////////////////////////////////////

#undef NoAlpha
#undef HasAlpha

#undef NoColorkey
#undef HasColorkey

#undef NotDynamic
#undef IsDynamic

#undef NotAligned
#undef IsAligned

////////////////////////////////////////////////////////////

#endif

