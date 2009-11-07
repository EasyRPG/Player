#include "SDL.h"

#ifdef PSP
	#define SCREEN_WIDTH 480
	#define SCREEN_HEIGHT 272
#else
	#define SCREEN_WIDTH 320
	#define SCREEN_HEIGHT 240
#endif
#define SCREEN_BPP 32

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	#define rmask 0xff000000
	#define gmask 0x00ff0000
	#define bmask 0x0000ff00
	#define amask 0x000000ff
#else
	#define rmask 0x000000ff
	#define gmask 0x0000ff00
	#define bmask 0x00ff0000
	#define amask 0xff000000
#endif
