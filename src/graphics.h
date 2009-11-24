#ifndef __graphics__
#define __graphics__

#include <string>
#include <list>
#include "SDL.h"
#include "SDL_framerate.h"
#include "viewport.h"
#include "bitmap.h"
#include "sprite.h"
#include "tilemap.h"
#include "window.h"
#include "plane.h"
#include "zobj.h"

#ifdef PSP
	#define SCREEN_WIDTH 480
	#define SCREEN_HEIGHT 272
#else
	#define SCREEN_WIDTH 320
	#define SCREEN_HEIGHT 240
#endif
#define SCREEN_BPP 16

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

namespace Graphics {
	void initialize();
	void dispose();
	void update();
	void transition();
	void transition(int duration);
	void transition(int duration, std::string filename);
	void transition(int duration, std::string filename, int vague);

	int get_frame_rate();
	int get_frame_count();
	void set_frame_rate(int fr);
	void set_frame_count(int fc);

    SDL_Surface* get_empty_dummy_surface(int w, int h);
    SDL_Surface* get_empty_real_surface(int w, int h);

    extern FPSmanager fps_manager;
	
	extern SDL_Surface *screen;
	
	extern std::list<ZObj> zlist;
	extern std::list<ZObj>::iterator zlist_it;
}
#endif // __graphics__
