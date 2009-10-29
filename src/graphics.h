#ifndef __graphics__
#define __graphics__

#include <string>
#include "SDL.h"
#include "viewport.h"
#include "bitmap.h"
#include "sprite.h"
#include "tilemap.h"
#include "window.h"
#include "plane.h"
#include "zobj.h"

class Graphics {
public:
	// Public methods
	static void initialize();
	static void dispose();
	static void update();
	static void transition();
	static void transition(int duration);
	static void transition(int duration, std::string filename);
	static void transition(int duration, std::string filename, int vague);

	static int get_frame_rate();
	static int get_frame_count();
	static void set_frame_rate(int fr);
	static void set_frame_count(int fc);
	
private:
	// SDL screen
	static SDL_Surface *screen;
	
	// Public properties
	static int frame_rate;
	static int frame_count;
};
#endif // __graphics__
