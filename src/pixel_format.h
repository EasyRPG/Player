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
/// BPP traits
////////////////////////////////////////////////////////////

template <int BPP>
struct bpp_traits {
};

template <>
struct bpp_traits<16> {
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
struct bpp_traits<24> {
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
struct bpp_traits<32> {
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
		 int bpp = PF::bpp,
		 bool aligned = PF::aligned,
		 bool has_alpha = PF::has_alpha,
		 bool has_colorkey = PF::has_colorkey>
struct alpha_traits {
	static inline uint8 get_alpha(uint32 colorkey, const uint8* p) {
		uint8 r, g, b, a;
		PF::get_rgba(p, r, g, b, a);
		return a;
	}
	static inline void set_alpha(uint32 colorkey, uint8* p, uint8 alpha) {
		uint8 r, g, b, a;
		PF::get_rgba(p, r, g, b, a);
		PF::set_rgba(p, r, g, b, alpha);
	}
};

// colorkey
template <class PF, int bpp, bool aligned>
struct alpha_traits<PF, bpp, aligned, false, true> {
	static inline uint8 get_alpha(uint32 colorkey, const uint8* p) {
		uint32 pix = PF::get_uint32(p);
		return (pix == colorkey) ? 0 : 255;
	}
	static inline void set_alpha(uint32 colorkey, uint8* p, uint8 alpha) {
		if (alpha == 0)
			PF::set_uint32(colorkey);
	}
};

// no alpha or colorkey
template<class PF>
struct alpha_traits<PF, 32, true, false, false> {
	static inline uint8 get_alpha(uint32 colorkey, const uint8* p) {
		return 255;
	}
	static inline void set_alpha(uint32 colorkey, uint8* p, uint8 alpha) {
	}
};

// aligned, with alpha
template<class PF>
struct alpha_traits<PF, 32, true, true, false> {
	static inline uint8 get_alpha(uint32 colorkey, const uint8* p) {
		return p[PF::abyte];
	}
	static inline void set_alpha(uint32 colorkey, uint8* p, uint8 alpha) {
		p[PF::abyte] = alpha;
	}
};

////////////////////////////////////////////////////////////
/// RGBA traits
////////////////////////////////////////////////////////////

template<class PF,
		 int bpp = PF::bpp,
		 bool aligned = PF::aligned,
		 bool has_alpha = PF::has_alpha,
		 bool has_colorkey = PF::has_colorkey>
struct rgba_traits {
};

// aligned, has alpha
template<class PF, int bpp>
struct rgba_traits<PF, bpp, true, true, false> {
	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::rbyte];
		g = p[PF::gbyte];
		b = p[PF::bbyte];
		a = p[PF::abyte];
	}

	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		p[PF::rbyte] = r;
		p[PF::gbyte] = g;
		p[PF::bbyte] = b;
		p[PF::abyte] = a;
	}
};

// aligned, has colorkey
template<class PF, int bpp>
struct rgba_traits<PF, bpp, true, false, true> {
	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::rbyte];
		g = p[PF::gbyte];
		b = p[PF::bbyte];
		a = (PF::get_uint32(p) == colorkey) ? 0 : 255;
	}

	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		if (a == 0)
			PF::set_uint32(p, colorkey);
		else {
			p[PF::rbyte] = r;
			p[PF::gbyte] = g;
			p[PF::bbyte] = b;
		}
	}
};

// aligned, no alpha or colorkey
template<class PF, int bpp>
struct rgba_traits<PF, bpp, true, false, false> {
	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = p[PF::rbyte];
		g = p[PF::gbyte];
		b = p[PF::bbyte];
		a = 255;
	}

	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		p[PF::rbyte] = r;
		p[PF::gbyte] = g;
		p[PF::bbyte] = b;
	}
};

// unaligned, has alpha
template<class PF, int bpp>
struct rgba_traits<PF, bpp, false, true, false> {
	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		PF::uint32_to_rgba(pix, r, g, b, a);
	}
	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = PF::rgba_to_uint32(r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

// unaligned, has colorkey
template<class PF, int bpp>
struct rgba_traits<PF, bpp, false, false, true> {
	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		if (pix == colorkey)
			a = 0;
		PF::uint32_to_rgba(pix, r, g, b, a);
	}

	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = (a == 0) ? colorkey : PF::rgba_to_uint32(r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

// unaligned, no alpha or colorkey
template<class PF, int bpp>
struct rgba_traits<PF, bpp, false, false, false> {
	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		const uint32 pix = PF::get_uint32(p);
		PF::uint32_to_rgba(pix, r, g, b, a);
		a = 255;
	}
	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		const uint32 pix = PF::rgba_to_uint32(r, g, b, a);
		PF::set_uint32(p, pix);
	}
};

////////////////////////////////////////////////////////////
/// PixelFormatUtils class
////////////////////////////////////////////////////////////

template <int BPP, int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS, bool CK>
class PixelFormat {
public:
	typedef class PixelFormat<BPP,RB,RS,GB,GS,BB,BS,AB,AS,CK> my_type;

	static const int bpp = BPP;
	static const int bytes = (BPP + 7) / 8;

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

	static const bool has_alpha = AB > 0;
	static const bool has_colorkey = CK;

	static const int aligned = (RB % 8 == 0 && RS % 8 == 0 &&
								GB % 8 == 0 && GS % 8 == 0 &&
								BB % 8 == 0 && BS % 8 == 0 &&
								AB % 8 == 0 && AS % 8 == 0);

	typedef bpp_traits<BPP> bpp_traits_type;
	typedef alpha_traits<my_type> alpha_traits_type;
	typedef rgba_traits<my_type> rgba_traits_type;

	static inline void uint32_to_rgba(uint32 pix, uint8& r, uint8& g, uint8& b, uint8& a) {
		r = (uint8)(((pix >> rshift) & ((1 << rbits) - 1)) << (8 - rbits));
		g = (uint8)(((pix >> gshift) & ((1 << gbits) - 1)) << (8 - gbits));
		b = (uint8)(((pix >> bshift) & ((1 << bbits) - 1)) << (8 - bbits));
		a = (uint8)(((pix >> ashift) & ((1 << abits) - 1)) << (8 - abits));
	}

	static inline uint32 rgba_to_uint32(const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		return
			(((uint32)r >> (8 - rbits)) << rshift) | 
			(((uint32)g >> (8 - gbits)) << gshift) | 
			(((uint32)b >> (8 - bbits)) << bshift) | 
			(((uint32)a >> (8 - abits)) << ashift);
	}

	static inline uint32 get_uint32(const uint8* p) {
		return bpp_traits_type::get_uint32(p);
	}

	static inline void set_uint32(uint8* p, uint32 pix) {
		bpp_traits_type::set_uint32(p, pix);
	}

	static inline void copy_pixel(uint8* dst, const uint8* src) {
		bpp_traits_type::copy_pixel(dst, src);
	}

	static inline void copy_pixels(uint8* dst, const uint8* src, int n) {
		memcpy(dst, src, n * bytes);
	}

	static inline void set_pixels(uint8* dst, const uint8* src, int n) {
		bpp_traits_type::set_pixels(dst, src, n);
	}

	static inline uint8 get_alpha(uint32 colorkey, const uint8* p) {
		return alpha_traits_type::get_alpha(colorkey, p);
	}

	static inline void set_alpha(uint32 colorkey, uint8* p, uint8 alpha) {
		alpha_traits_type::set_alpha(colorkey, p, alpha);
	}

	static inline void get_rgba(uint32 colorkey, const uint8* p, uint8& r, uint8& g, uint8& b, uint8& a) {
		rgba_traits_type::get_rgba(colorkey, p, r, g, b, a);
	}

	static inline void set_rgba(uint32 colorkey, uint8* p, const uint8& r, const uint8& g, const uint8& b, const uint8& a) {
		rgba_traits_type::set_rgba(colorkey, p, r, g, b, a);
	}
};

#endif

