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

#ifndef EP_PIXEL_FORMAT_H
#define EP_PIXEL_FORMAT_H

// Headers
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "system.h"
#include "utils.h"

/** Enums. */
namespace PF {
	enum AlphaType { NoAlpha, Alpha };
	enum OpacityType { Opaque, Binary, Variable };
	enum { ONE = 255 };
	enum { StaticMasks = false, DynamicMasks = true };
	enum { StaticAlpha = false, DynamicAlpha = true };
	enum { NotAligned = false, IsAligned = true };
}

/** Component struct */
struct Component {
	uint8_t bits = 0;
	uint8_t shift = 0;
	uint8_t byte = 0;
	uint32_t mask = 0;

	static constexpr int count_bits(uint32_t mask) {
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

	constexpr void convert_mask() {
		int bit_count = count_bits(mask);
		uint32_t mask_ex = (~0U >> (32 - bit_count));
		uint32_t mask_lo = mask_ex - mask;
		shift = (uint8_t)count_bits(mask_lo) & 0x1F;
		bits = (uint8_t)(bit_count - shift);
		byte = shift / 8;
	}

	constexpr bool operator==(const Component& c) {
		return mask == c.mask;
	}

	constexpr bool operator!=(const Component& c) {
		return mask != c.mask;
	}

	constexpr uint8_t unpack(uint32_t pix) const {
		return (uint8_t)(((pix >> shift) & ((1 << bits) - 1)) << (8 - bits));
	}

	constexpr uint32_t pack(const uint8_t& x) const {
		return (((uint32_t)x >> (8 - bits)) << shift);
	}

	constexpr Component() {}

	constexpr Component(unsigned int bits, unsigned int shift) :
		bits((uint8_t)bits),
		shift((uint8_t)shift),
		byte((uint8_t)(shift / 8)),
		mask(((1u << bits)-1) << shift) {}

	constexpr Component(uint32_t mask) :
		mask(mask) { convert_mask(); }
};

/**
 * DynamicFormat struct
 */
class DynamicFormat {
public:
	int bits = 0;
	int bytes = 0;
	Component r, g, b, a;
	PF::AlphaType alpha_type = PF::NoAlpha;

	constexpr DynamicFormat() {}

	constexpr DynamicFormat(int bits,
				  int rb, int rs,
				  int gb, int gs,
				  int bb, int bs,
				  int ab, int as,
				  PF::AlphaType alpha_type) :
		bits(bits), bytes((bits + 7) / 8),
		r(rb, rs), g(gb, gs), b(bb, bs), a(ab, as),
		alpha_type(alpha_type) {}

	constexpr DynamicFormat(int bits,
				  uint32_t rmask,
				  uint32_t gmask,
				  uint32_t bmask,
				  uint32_t amask,
				  PF::AlphaType alpha_type) :
		bits(bits), bytes((bits + 7) / 8),
		r(rmask), g(gmask), b(bmask), a(amask),
		alpha_type(alpha_type) {}

	constexpr void Set(int _bits,
			 int rb, int rs,
			 int gb, int gs,
			 int bb, int bs,
			 int ab, int as,
			 PF::AlphaType _alpha_type) {
		bits = _bits;
		bytes = (bits + 7) / 8;
		r = Component(rb, rs);
		g = Component(gb, gs);
		b = Component(bb, bs);
		a = Component(ab, as);
		alpha_type = _alpha_type;
	}

	constexpr void Set(int _bits,
			 uint32_t rmask,
			 uint32_t gmask,
			 uint32_t bmask,
			 uint32_t amask,
			 PF::AlphaType _alpha_type) {
		bits = _bits;
		bytes = (bits + 7) / 8;
		r = Component(rmask);
		g = Component(gmask);
		b = Component(bmask);
		a = Component(amask);
		alpha_type = _alpha_type;
	}

	constexpr int code(bool shifts) const {
		int x = (int) alpha_type | ((bits - 1) << 2);
		if (!shifts)
			return x;
		return x |
			(r.shift <<  7) |
			(g.shift << 12) |
			(b.shift << 17);
	}

	constexpr int code_alpha() const {
		int x = (int) (alpha_type == PF::Alpha ? PF::Alpha : PF::NoAlpha) | ((bits - 1) << 2);
		return x |
			(r.shift <<  7) |
			(g.shift << 12) |
			(b.shift << 17) |
			(alpha_type == PF::Alpha ? (a.shift << 22) : 0);
	}

	constexpr void uint32_to_rgba(uint32_t pix, uint8_t& _r, uint8_t& _g, uint8_t& _b, uint8_t& _a) const {
		_r = r.unpack(pix);
		_g = g.unpack(pix);
		_b = b.unpack(pix);
		_a = a.unpack(pix);
	}

	constexpr uint32_t rgba_to_uint32_t(const uint8_t& _r, const uint8_t& _g, const uint8_t& _b, const uint8_t& _a) const {
		return r.pack(_r) | g.pack(_g) | b.pack(_b) | a.pack(_a);
	}

	constexpr bool operator==(const DynamicFormat& f) {
		return r ==  f.r && g == f.g && b == f.b && a == f.a && alpha_type == f.alpha_type;
	}

	constexpr bool operator!=(const DynamicFormat& f) {
		return r !=  f.r || g != f.g || b != f.b || a != f.a || alpha_type != f.alpha_type;
	}
};

// Bits traits.

template <class TPF, int bits>
struct bits_traits {
};

template <class TPF>
struct bits_traits<TPF, 8> {
	static inline uint32_t get_uint32_t(const uint8_t* p) {
		return (uint32_t)*(const uint8_t*)p;
	}
	static inline void set_uint32_t(uint8_t* p, uint32_t pix) {
		*(uint8_t*)p = (uint8_t) pix;
	}
	static inline void copy_pixel(uint8_t* dst, const uint8_t* src) {
		*(uint8_t*)dst = *(const uint8_t*)src;
	}
	static inline void set_pixels(uint8_t* dst, const uint8_t* src, int n) {
		uint8_t pixel = (uint8_t) get_uint32_t(src);
		uint8_t* dst_pix = (uint8_t*) dst;
		std::fill(dst_pix, dst_pix + n, pixel);
	}
};

template <class TPF>
struct bits_traits<TPF, 16> {
	static inline uint32_t get_uint32_t(const uint8_t* p) {
		return (uint32_t)*(const uint16_t*)p;
	}
	static inline void set_uint32_t(uint8_t* p, uint32_t pix) {
		*(uint16_t*)p = (uint16_t) pix;
	}
	static inline void copy_pixel(uint8_t* dst, const uint8_t* src) {
		*(uint16_t*)dst = *(const uint16_t*)src;
	}
	static inline void set_pixels(uint8_t* dst, const uint8_t* src, int n) {
		uint16_t pixel = (uint16_t) get_uint32_t(src);
		uint16_t* dst_pix = (uint16_t*) dst;
		std::fill(dst_pix, dst_pix + n, pixel);
	}
};

template <class TPF>
struct bits_traits<TPF, 24> {
	static inline uint32_t get_uint32_t(const uint8_t* p) {
		return
			((uint32_t)(p[TPF::endian(2)]) << 16) |
			((uint32_t)(p[TPF::endian(1)]) <<  8) |
			((uint32_t)(p[TPF::endian(0)]) <<  0);
	}
	static inline void set_uint32_t(uint8_t* p, uint32_t pix) {
		p[TPF::endian(0)] = (pix >>  0) & 0xFF;
		p[TPF::endian(1)] = (pix >>  8) & 0xFF;
		p[TPF::endian(2)] = (pix >> 16) & 0xFF;
	}
	static inline void copy_pixel(uint8_t* dst, const uint8_t* src) {
		std::memcpy(dst, src, 3);
	}
	static inline void set_pixels(uint8_t* dst, const uint8_t* src, int n) {
		for (int i = 0; i < n; i++)
			copy_pixel(dst + i * 3, src);
	}
};

template <class TPF>
struct bits_traits<TPF, 32> {
	static inline uint32_t get_uint32_t(const uint8_t* p) {
		return *(const uint32_t*) p;
	}
	static inline void set_uint32_t(uint8_t* p, uint32_t pix) {
		*(uint32_t*)p = pix;
	}
	static inline void copy_pixel(uint8_t* dst, const uint8_t* src) {
		*(uint32_t*)dst = *(const uint32_t*)src;
	}
	static inline void set_pixels(uint8_t* dst, const uint8_t* src, int n) {
		uint32_t pixel = get_uint32_t(src);
		uint32_t* dst_pix = (uint32_t*) dst;
		std::fill(dst_pix, dst_pix + n, pixel);
	}
};

// alpha_type_traits

// general case
template<class TPF, bool dynamic_alpha, PF::AlphaType alpha_type>
struct alpha_type_traits {};

template<class TPF, PF::AlphaType _alpha_type>
struct alpha_type_traits<TPF, PF::StaticAlpha, _alpha_type> {
	static inline PF::AlphaType alpha_type(const TPF* /* pf */) {
		return (PF::AlphaType) _alpha_type;
	}
};

template<class TPF, PF::AlphaType _alpha_type>
struct alpha_type_traits<TPF, PF::DynamicAlpha, _alpha_type> {
	static inline PF::AlphaType alpha_type(const TPF* pf) {
		return pf->format().alpha_type;
	}
};

// opacity_type_traits

// general case
template<class TPF, bool dynamic_alpha, PF::OpacityType opacity_type>
struct opacity_type_traits {};

template<class TPF, PF::OpacityType _opacity_type>
struct opacity_type_traits<TPF, PF::StaticAlpha, _opacity_type> {
	static inline PF::OpacityType opacity_type(const TPF* /* pf */) {
		return _opacity_type;
	}
};

template<class TPF, PF::OpacityType _opacity_type>
struct opacity_type_traits<TPF, PF::DynamicAlpha, _opacity_type> {
	static inline PF::OpacityType opacity_type(const TPF* pf) {
		return pf->format().opacity_type;
	}
};

// Alpha traits

// general case
template<class TPF,
		 bool aligned,
		 bool dynamic_alpha,
		 int alpha_type>
struct alpha_traits {
	static inline uint8_t get_alpha(const TPF* pf, const uint8_t* p) {
		uint8_t r, g, b, a;
		pf->get_rgba(p, r, g, b, a);
		return a;
	}
	static inline void set_alpha(const TPF* pf, uint8_t* p, uint8_t alpha) {
		uint8_t r, g, b, a;
		pf->get_rgba(p, r, g, b, a);
		pf->set_rgba(p, r, g, b, alpha);
	}
};

// no alpha
template<class TPF, bool aligned>
struct alpha_traits<TPF, aligned, PF::StaticAlpha, PF::NoAlpha> {
	static inline uint8_t get_alpha(const TPF* /* pf */, const uint8_t* /* p */) {
		return 255;
	}
	static inline void set_alpha(const TPF* /* pf */, uint8_t* /* p */, uint8_t /* alpha */) {
	}
};

// aligned, with alpha
template<class TPF, bool dynamic_alpha>
struct alpha_traits<TPF, PF::IsAligned, dynamic_alpha, PF::Alpha> {
	static inline uint8_t get_alpha(const TPF* pf, const uint8_t* p) {
		return pf->has_alpha() ? p[pf->a_byte()] : 255;
	}
	static inline void set_alpha(const TPF* pf, uint8_t* p, uint8_t alpha) {
		if (pf->has_alpha())
			p[pf->a_byte()] = alpha;
	}
};

// RGBA traits

template<class TPF, bool aligned, bool dynamic_alpha, int alpha>
struct rgba_traits {
};

// aligned, has alpha
template<class TPF, bool dynamic_alpha>
struct rgba_traits<TPF, PF::IsAligned, dynamic_alpha, PF::Alpha> {
	static inline void get_rgba(const TPF* pf, const uint8_t* p, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
		r = p[pf->r_byte()];
		g = p[pf->g_byte()];
		b = p[pf->b_byte()];
		a = pf->has_alpha() ? p[pf->a_byte()] : 255;
	}

	static inline void set_rgba(const TPF* pf, uint8_t* p, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
		p[pf->r_byte()] = r;
		p[pf->g_byte()] = g;
		p[pf->b_byte()] = b;
		if (pf->has_alpha())
			p[pf->a_byte()] = a;
	}
};

// aligned, no alpha
template<class TPF>
struct rgba_traits<TPF, PF::IsAligned, PF::StaticAlpha, PF::NoAlpha> {
	static inline void get_rgba(const TPF* pf, const uint8_t* p, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
		r = p[pf->r_byte()];
		g = p[pf->g_byte()];
		b = p[pf->b_byte()];
		a = 255;
	}

	static inline void set_rgba(const TPF* pf, uint8_t* p, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& /* a */) {
		p[pf->r_byte()] = r;
		p[pf->g_byte()] = g;
		p[pf->b_byte()] = b;
	}
};

// unaligned, has alpha (dynamic)
template<class TPF, bool dynamic_alpha>
struct rgba_traits<TPF, PF::NotAligned, dynamic_alpha, PF::Alpha> {
	static inline void get_rgba(const TPF* pf, const uint8_t* p, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
		const uint32_t pix = pf->get_uint32_t(p);
		pf->uint32_to_rgba(pix, r, g, b, a);
		if (!pf->has_alpha())
			a = 255;
	}
	static inline void set_rgba(const TPF* pf, uint8_t* p, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
		const uint32_t pix = pf->rgba_to_uint32_t(r, g, b, a);
		pf->set_uint32_t(p, pix);
	}
};

// unaligned, no alpha
template<class TPF>
struct rgba_traits<TPF, PF::NotAligned, PF::StaticAlpha, PF::NoAlpha> {
	static inline void get_rgba(const TPF* pf, const uint8_t* p, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
		const uint32_t pix = pf->get_uint32_t(p);
		pf->uint32_to_rgba(pix, r, g, b, a);
		a = 255;
	}
	static inline void set_rgba(const TPF* pf, uint8_t* p, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
		const uint32_t pix = pf->rgba_to_uint32_t(r, g, b, a);
		pf->set_uint32_t(p, pix);
	}
};

// Mask traits

template<class TPF, bool dynamic, int _bits, int _shift>
struct mask_traits {
};

template<class TPF, int _bits, int _shift>
struct mask_traits<TPF, PF::StaticMasks, _bits, _shift> {
	static const int _byte = _shift / 8;
	static const int _mask = ((1 << _bits)-1) << _shift;
	static inline int bits(const Component&) { return _bits; }
	static inline int shift(const Component&) { return _shift; }
	static inline int byte(const Component&) { return _byte; }
	static inline int mask(const Component&) { return _mask; }
};

template<class TPF, int _bits, int _shift>
struct mask_traits<TPF, PF::DynamicMasks, _bits, _shift> {
	static const int _mask = 0;
	static inline int bits(const Component& c) { return c.bits; }
	static inline int shift(const Component& c) { return c.shift; }
	static inline int byte(const Component& c) { return c.byte; }
	static inline int mask(const Component& c) { return c.mask; }
};

// Dynamic traits

template <bool DYNAMIC_MASKS, bool DYNAMIC_ALPHA,
		  int BITS, int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS, int ALPHA>
struct dynamic_traits_t {
	DynamicFormat format;
	dynamic_traits_t(const DynamicFormat &format) : format(format) {}
	void set_format(const DynamicFormat &_format) { format = _format; }
};

template <int BITS, int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS, int ALPHA>
struct dynamic_traits_t<false, false, BITS, RB, RS, GB, GS, BB, BS, AB, AS, ALPHA> {
	static const DynamicFormat format;
	dynamic_traits_t(const DynamicFormat &) {}
	void set_format(const DynamicFormat &) const {}
};

template <int BITS, int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS, int ALPHA>
const DynamicFormat dynamic_traits_t<false, false, BITS, RB, RS, GB, GS, BB, BS, AB, AS, ALPHA>::format(BITS, RB, RS, GB, GS, BB, BS, AB, AS, (PF::AlphaType) ALPHA);

/**
 * PixelFormat abstract base class.
 */
class PixelFormat {
public:
	PixelFormat() {}

	virtual bool Match(const DynamicFormat& ref) const = 0;
	virtual int Bits() const = 0;
	virtual bool HasAlpha() const = 0;
	virtual const DynamicFormat& Format() const  = 0;
};

// PixelFormatT template

template <int BITS,
		  bool DYNAMIC_MASKS, bool DYNAMIC_ALPHA, int ALPHA, bool ALIGNED,
		  int RB, int RS, int GB, int GS, int BB, int BS, int AB, int AS>
class PixelFormatT : public PixelFormat {
public:
	typedef PixelFormatT<BITS,DYNAMIC_MASKS,DYNAMIC_ALPHA,ALPHA,ALIGNED,RB,RS,GB,GS,BB,BS,AB,AS> my_type;

	static const int bits = BITS;
	static const int bytes = (BITS + 7) / 8;

	static const bool dynamic_masks = DYNAMIC_MASKS;
	static const bool dynamic_alpha = DYNAMIC_ALPHA;
	static const PF::AlphaType alpha = (PF::AlphaType) ALPHA;
	static const PF::OpacityType opacity =
		ALPHA == PF::NoAlpha ? PF::Opaque :
		(AB == 1) ? PF::Binary :
		PF::Variable;

	static const bool aligned = ALIGNED;

	static const int ONE = 255;
	// static const int ONE = 256; // faster but less accurate

	typedef bits_traits<my_type, bits> bits_traits_type;
	typedef dynamic_traits_t<DYNAMIC_MASKS,DYNAMIC_ALPHA,BITS,RB,RS,GB,GS,BB,BS,AB,AS,ALPHA> dynamic_traits_type;
	typedef alpha_type_traits<my_type, dynamic_alpha, alpha> alpha_type_traits_type;
	typedef opacity_type_traits<my_type, dynamic_alpha, opacity> opacity_type_traits_type;
	typedef alpha_traits<my_type, aligned, dynamic_alpha, alpha> alpha_traits_type;
	typedef rgba_traits<my_type, aligned, dynamic_alpha, alpha> rgba_traits_type;
	typedef mask_traits<my_type, dynamic_masks, RB, RS> mask_r_traits_type;
	typedef mask_traits<my_type, dynamic_masks, GB, GS> mask_g_traits_type;
	typedef mask_traits<my_type, dynamic_masks, BB, BS> mask_b_traits_type;
	typedef mask_traits<my_type, dynamic_masks, AB, AS> mask_a_traits_type;
	dynamic_traits_type dynamic_traits;

	PixelFormatT() : dynamic_traits(DynamicFormat(BITS, RB, RS, GB, GS, BB, BS, AB, AS, (PF::AlphaType) ALPHA)) {}
	PixelFormatT(const DynamicFormat& format) : dynamic_traits(format) {}

	static inline int endian(int byte) {
		return Utils::IsBigEndian()? (bytes - 1 - byte) : byte;
	}

	inline void uint32_to_rgba(uint32_t pix, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
		r = (uint8_t)(((pix >> r_shift()) & ((1 << r_bits()) - 1)) << (8 - r_bits()));
		g = (uint8_t)(((pix >> g_shift()) & ((1 << g_bits()) - 1)) << (8 - g_bits()));
		b = (uint8_t)(((pix >> b_shift()) & ((1 << b_bits()) - 1)) << (8 - b_bits()));
		a = (uint8_t)(((pix >> a_shift()) & ((1 << a_bits()) - 1)) << (8 - a_bits()));
	}

	inline uint32_t rgba_to_uint32_t(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) const {
		return
			(((uint32_t)r >> (8 - r_bits())) << r_shift()) |
			(((uint32_t)g >> (8 - g_bits())) << g_shift()) |
			(((uint32_t)b >> (8 - b_bits())) << b_shift()) |
			(((uint32_t)a >> (8 - a_bits())) << a_shift());
	}

	inline const DynamicFormat& format() const {
		return dynamic_traits.format;
	}

	inline int r_byte() const {		return endian(mask_r_traits_type::byte(format().r));	}
	inline int g_byte() const {		return endian(mask_g_traits_type::byte(format().g));	}
	inline int b_byte() const {		return endian(mask_b_traits_type::byte(format().b));	}
	inline int a_byte() const {		return endian(mask_a_traits_type::byte(format().a));	}

	inline uint32_t r_mask() const {	return mask_r_traits_type::mask(format().r);	}
	inline uint32_t g_mask() const {	return mask_g_traits_type::mask(format().g);	}
	inline uint32_t b_mask() const {	return mask_b_traits_type::mask(format().b);	}
	inline uint32_t a_mask() const {	return mask_a_traits_type::mask(format().a);	}

	inline int r_bits() const {		return mask_r_traits_type::bits(format().r);	}
	inline int g_bits() const {		return mask_g_traits_type::bits(format().g);	}
	inline int b_bits() const {		return mask_b_traits_type::bits(format().b);	}
	inline int a_bits() const {		return mask_a_traits_type::bits(format().a);	}

	inline int r_shift() const {	return mask_r_traits_type::shift(format().r);	}
	inline int g_shift() const {	return mask_g_traits_type::shift(format().g);	}
	inline int b_shift() const {	return mask_b_traits_type::shift(format().b);	}
	inline int a_shift() const {	return mask_a_traits_type::shift(format().a);	}

	inline PF::AlphaType alpha_type() const {
		return alpha_type_traits_type::alpha_type(this);
	}

	inline bool opacity_type() const {
		return opacity_type_traits_type::opacity_type(this);
	}

	inline bool has_alpha() const {
		return alpha_type() != PF::NoAlpha;
	}

	inline uint32_t get_uint32_t(const uint8_t* p) const {
		return bits_traits_type::get_uint32_t(p);
	}

	inline void set_uint32_t(uint8_t* p, uint32_t pix) const {
		bits_traits_type::set_uint32_t(p, pix);
	}

	inline void copy_pixel(uint8_t* dst, const uint8_t* src) const {
		bits_traits_type::copy_pixel(dst, src);
	}

	inline void copy_pixels(uint8_t* dst, const uint8_t* src, int n) const {
		std::memcpy(dst, src, n * bytes);
	}

	inline void set_pixels(uint8_t* dst, const uint8_t* src, int n) const {
		bits_traits_type::set_pixels(dst, src, n);
	}

	inline uint8_t opaque() const {
		return (a_bits() > 0) ? (uint8_t) (0xFF << (8 - a_bits())) : (uint8_t) 255;
	}

	inline uint8_t get_alpha(const uint8_t* p) const {
		return alpha_traits_type::get_alpha(this, p);
	}

	inline void set_alpha(uint8_t* p, uint8_t alpha) const {
		alpha_traits_type::set_alpha(this, p, alpha);
	}

	inline void get_rgba(const uint8_t* p, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
		rgba_traits_type::get_rgba(this, p, r, g, b, a);
	}

	inline void set_rgba(uint8_t* p, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) const {
		rgba_traits_type::set_rgba(this, p, r, g, b, a);
	}

	bool Match(const DynamicFormat& ref) const override {
		return
			bits == ref.bits &&
			(dynamic_alpha || alpha_type() == ref.alpha_type) &&
			(dynamic_masks || (
				 r_mask() == ref.r.mask &&
				 g_mask() == ref.g.mask &&
				 b_mask() == ref.b.mask &&
				 (a_mask() == ref.a.mask || alpha_type() != PF::Alpha)));
	}

	int Bits() const override {
		return bits;
	}

	const DynamicFormat& Format() const override {
		return dynamic_traits.format;
	}

	void SetFormat(const DynamicFormat& format) {
		dynamic_traits.set_format(format);
	}

	bool HasAlpha() const override {
		return has_alpha();
	}
};

#ifndef WORDS_BIGENDIAN
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,16,8,8,8,0,8,24> format_B8G8R8A8_a;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,0,8,8,8,16,8,24> format_R8G8B8A8_a;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,24,8,16,8,8,8,0> format_A8B8G8R8_a;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,8,8,16,8,24,8,0> format_A8R8G8B8_a;

typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,16,8,8,8,0,8,24> format_B8G8R8A8_n;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,0,8,8,8,16,8,24> format_R8G8B8A8_n;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,24,8,16,8,8,8,0> format_A8B8G8R8_n;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,8,8,16,8,24,8,0> format_A8R8G8B8_n;
#else
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,8,8,16,8,24,8,0> format_B8G8R8A8_a;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,24,8,16,8,8,8,0> format_R8G8B8A8_a;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,0,8,8,8,16,8,24> format_A8B8G8R8_a;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,8,16,8,8,8,0,8,24> format_A8R8G8B8_a;

typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,8,8,16,8,24,8,0> format_B8G8R8A8_n;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,24,8,16,8,8,8,0> format_R8G8B8A8_n;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,0,8,8,8,16,8,24> format_A8B8G8R8_n;
typedef PixelFormatT<32,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,16,8,8,8,0,8,24> format_A8R8G8B8_n;
#endif

typedef PixelFormatT< 8,PF::StaticMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,8,0,8,0,8,0,0,0> format_L8;

typedef PixelFormatT<32,PF::DynamicMasks,PF::StaticAlpha,PF::Alpha,PF::IsAligned,0,0,0,0,0,0,0,0> format_dynamic_32_a;
typedef PixelFormatT<32,PF::DynamicMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,0,0,0,0,0,0,0,0> format_dynamic_32_n;
typedef PixelFormatT<32,PF::DynamicMasks,PF::DynamicAlpha,PF::NoAlpha,PF::IsAligned,0,0,0,0,0,0,0,0> format_dynamic_32_d;

typedef PixelFormatT<24,PF::DynamicMasks,PF::StaticAlpha,PF::NoAlpha,PF::IsAligned,0,0,0,0,0,0,0,0> format_dynamic_24_n;
typedef PixelFormatT<24,PF::DynamicMasks,PF::DynamicAlpha,PF::NoAlpha,PF::IsAligned,0,0,0,0,0,0,0,0> format_dynamic_24_d;

typedef PixelFormatT<16,PF::DynamicMasks,PF::StaticAlpha,PF::Alpha,PF::NotAligned,0,0,0,0,0,0,0,0> format_dynamic_16_a;
typedef PixelFormatT<16,PF::DynamicMasks,PF::StaticAlpha,PF::NoAlpha,PF::NotAligned,0,0,0,0,0,0,0,0> format_dynamic_16_n;
typedef PixelFormatT<16,PF::DynamicMasks,PF::DynamicAlpha,PF::NoAlpha,PF::NotAligned,0,0,0,0,0,0,0,0> format_dynamic_16_d;

#endif
