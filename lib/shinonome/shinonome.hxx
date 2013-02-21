#ifndef _INC_SHINONOME_HXX_
#define _INC_SHINONOME_HXX_

#ifdef _MSC_VER
typedef unsigned short uint16_t;
#else
#include <boost/cstdint.hpp>
#endif

struct ShinonomeGlyph {
	uint16_t code;
	bool is_full;
	uint16_t data[12];
};

extern ShinonomeGlyph const SHINONOME_GOTHIC[7187];
extern ShinonomeGlyph const SHINONOME_MINCHO[488];

#endif // _INC_SHINONOME_HXX_
