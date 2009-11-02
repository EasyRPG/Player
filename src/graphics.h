#ifndef __graphics__
#define __graphics__

#include <string>
#include <list>
#include "SDL.h"
#include "viewport.h"
#include "bitmap.h"
#include "sprite.h"
#include "tilemap.h"
#include "window.h"
#include "plane.h"
#include "zobj.h"

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
	
	extern SDL_Surface *screen;
	extern int frame_rate;
	extern int frame_count;
	
	extern std::list<ZObj> zlist;
	extern std::list<ZObj>::iterator zlist_it;
}
#endif // __graphics__
