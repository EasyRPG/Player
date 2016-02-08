#ifndef _INC_SHINONOME_H_
#define _INC_SHINONOME_H_

#include <stdint.h>

struct ShinonomeGlyph {
	uint16_t code;
	bool is_full;
	uint16_t data[12];
};

extern ShinonomeGlyph const SHINONOME_GOTHIC[11217];
extern ShinonomeGlyph const SHINONOME_MINCHO[488];

#endif // _INC_SHINONOME_H_
