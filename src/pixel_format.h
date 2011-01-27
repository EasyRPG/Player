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
/// PixelFormat class
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Bits traits
////////////////////////////////////////////////////////////

template <int BITS>
struct bits_traits {
};

template <>
struct bits_traits<16> {
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

template <>
struct bits_traits<24> {
	static inline uint32 get_uint32(const uint8* p) {
		return ((uint32)(p[2]) << 16) | ((uint32)(p[1]) << 8) | (uint32)(p[0]);
	}
	static inline void set_uint32(uint8* p, uint32 pix) {
		p[0] = (pix >> 0) & 0xFF;
		p[1] = (pix >> 1) & 0xFF;
		p[2] = (pix >> 2) & 0xFF;
	}
	static inline void copy_pixel(uint8* dst, const uint8* src) {
		memcpy(dst, src, 3);
	}
	static inline void set_pixels(uint8* dst, const uint8* src, int n) {
		for (int i = 0; i < n; i++)
			copy_pixel(dst + i * 3, src);
	}
};

template <>
struct bits_traits<32> {
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
		 int bits = PF::bits,
		 bool aligned = PF::aligned,
		 bool has_alpha = PF::has_alpha,
		 bool has_colorkey = PF::has_colorkey>
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
template <class PF, int bits, bool aligned>
struct alpha_traits<PF, bits, aligned, false, true> {
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
template<class PF>
struct alpha_traits<PF, 32, true, false, false> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return 255;
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
	}
};

// aligned, with alpha
template<class PF>
struct alpha_traits<PF, 32, true, true, false> {
	static inline uint8 get_alpha(const DynamicFormat& format, const uint8* p) {
		return p[PF::abyte];
	}
	static inline void set_alpha(const DynamicFormat& format, uint8* p, uint8 alpha) {
		p[PF::abyte] = alpha;
	}
};

////////////////////////////////////////////////////////////
/// RGBA traits
////////////////////////////////////////////////////////////

template<class PF,
		 bool aligned = PF::aligned,
		 bool has_alpha = PF::has_alpha,
		 bool has_colorkey = PF::has_colorkey>
struct rgba_traits {
};

// aligned, has alpha
template<class PF>
struct rgba_traits<PF, true, true, false> {
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
struct rgba_traits<PF, true, false, true> {
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
struct rgba_traits<PF, true, false, false> {
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
struct rgba_traits<PF, false, true, false> {
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
struct rgba_traits<PF, false, false, true> {
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
struct rgba_traits<PF, false, false, false> {
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

template<class PF, bool dynamic = PF::dynamic>
struct mask_traits {
};

template<class PF>
struct mask_traits<PF, false> {
	static inline void uint32_to_rgba(const DynamicFormat& format, uint32 pix, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = (uint8)(((pix >> PF::rshift) & ((1 << PF::rbits) - 1)) << (8 - PF::rbits));
		g = (uint8)(((pix >> PF::gshift) & ((1 << PF::gbits) - 1)) << (8 - PF::gbits));
		b = (uint8)(((pix >> PF::bshift) & ((1 << PF::bbits) - 1)) << (8 - PF::bbits));
		a = (uint8)(((pix >> PF::ashift) & ((1 << PF::abits) - 1)) << (8 - PF::abits));
	}

	static inline uint32 rgba_to_uint32(const DynamicFormat& format, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		return
			(((uint32)r >> (8 - PF::rbits)) << PF::rshift) | 
			(((uint32)g >> (8 - PF::gbits)) << PF::gshift) | 
			(((uint32)b >> (8 - PF::bbits)) << PF::bshift) | 
			(((uint32)a >> (8 - PF::abits)) << PF::ashift);
	}

	static inline int r_byte(const DynamicFormat& format) { return PF::rbyte; }
	static inline int g_byte(const DynamicFormat& format) { return PF::gbyte; }
	static inline int b_byte(const DynamicFormat& format) { return PF::bbyte; }
	static inline int a_byte(const DynamicFormat& format) { return PF::abyte; }
};

template<class PF>
struct mask_traits<PF, true> {
	static inline void uint32_to_rgba(const DynamicFormat& format, uint32 pix, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = (uint8)(((pix >> format.rshift) & ((1 << format.rbits) - 1)) << (8 - format.rbits));
		g = (uint8)(((pix >> format.gshift) & ((1 << format.gbits) - 1)) << (8 - format.gbits));
		b = (uint8)(((pix >> format.bshift) & ((1 << format.bbits) - 1)) << (8 - format.bbits));
		a = (uint8)(((pix >> format.ashift) & ((1 << format.abits) - 1)) << (8 - format.abits));
	}

	static inline uint32 rgba_to_uint32(const DynamicFormat& format, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		return
			(((uint32)r >> (8 - format.rbits)) << format.rshift) | 
			(((uint32)g >> (8 - format.gbits)) << format.gshift) | 
			(((uint32)b >> (8 - format.bbits)) << format.bshift) | 
			(((uint32)a >> (8 - format.abits)) << format.ashift);
	}

	static inline int r_byte(const DynamicFormat& format) { return format.rbyte; }
	static inline int g_byte(const DynamicFormat& format) { return format.gbyte; }
	static inline int b_byte(const DynamicFormat& format) { return format.bbyte; }
	static inline int a_byte(const DynamicFormat& format) { return format.abyte; }
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

	static const int rbits = RB;
	static const int rshift = RS;
	static const int rbyte = RS / 8;
	static const int rmask = ((1 << RB)-1) << RS;

	static const int gbits = GB;
	static const int gshift = GS;
	static const int gbyte = GS / 8;
	static const int gmask = ((1 << GB)-1) << GS;

	static const int bbits = BB;
	static const int bshift = BS;
	static const int bbyte = BS / 8;
	static const int bmask = ((1 << BB)-1) << BS;

	static const int abits = AB;
	static const int ashift = AS;
	static const int abyte = AS / 8;
	static const int amask = ((1 << AB)-1) << AS;

	static const bool dynamic = DYNAMIC;
	static const bool has_alpha = HAS_ALPHA;
	static const bool has_colorkey = COLORKEY;
	static const bool aligned = ALIGNED;

	typedef bits_traits<bits> bits_traits_type;
	typedef alpha_traits<my_type> alpha_traits_type;
	typedef rgba_traits<my_type> rgba_traits_type;
	typedef mask_traits<my_type> mask_traits_type;

	static inline void uint32_to_rgba(const DynamicFormat& format, uint32 pix, uint8& r, uint8& g, uint8& b, uint8& a) {
		mask_traits_type::uint32_to_rgba(format, pix, r, g, b, a);
	}

	static inline uint32 rgba_to_uint32(const DynamicFormat& format, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		return mask_traits_type::rgba_to_uint32(format, r, g, b, a);
	}

	static inline int r_byte(const DynamicFormat& format) {
		return mask_traits_type::r_byte(format);
	}

	static inline int g_byte(const DynamicFormat& format) {
		return mask_traits_type::g_byte(format);
	}

	static inline int b_byte(const DynamicFormat& format) {
		return mask_traits_type::b_byte(format);
	}

	static inline int a_byte(const DynamicFormat& format) {
		return mask_traits_type::a_byte(format);
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

#endif

