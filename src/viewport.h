#ifndef __viewport__
#define __viewport__

#include <map>
#include "SDL.h"
#include "rect.h"
#include "color.h"
#include "tone.h"
#include "graphics.h"

class Viewport {

public:
	Viewport(int ix, int iy, int iwidth, int iheight);
	Viewport(Rect *irect);
	~Viewport();
	
	void dispose();
	bool is_disposed();
	void flash(Color *flash_color, int duration);
	void update();

	void draw(SDL_Surface *screen);
	
	Rect* get_rect();
	bool get_visible();
	int get_z();
	int get_ox();
	int get_oy();
	Color* get_color();
	Tone* get_tone();
	
	void set_rect(Rect* nrect);
	void set_visible(bool nvisible);
	void set_z(int nz);
	void set_ox(int nox);
	void set_oy(int noy);
	void set_color(Color* ncolor);
	void set_tone(Tone* ntone);
	
	static std::map<int, Viewport*> viewports;
	static void add_viewport(int id, Viewport* viewport);
	static void remove_viewport(int id);
	
private:
	Rect rect;
	bool visible;
	int z;
	int ox;
	int oy;
	Color* color;
	Tone* tone;

	bool disposed;
	
	int id;
	static int count;
};
#endif // __viewport__
