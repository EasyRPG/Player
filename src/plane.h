#ifndef __plane__
#define __plane__

#include <map>
#include "graphics.h"
#include "bitmap.h"
#include "rect.h"
#include "color.h"
#include "tone.h"

class Viewport;

class Plane {

public:
	Plane();
	Plane(Viewport *iviewport);
	~Plane();

	void dispose();
	bool is_disposed();
	
	void draw(SDL_Surface *screen);
	
	Viewport* get_viewport();
	Bitmap* get_bitmap();
	Rect *get_src_rect();
	bool get_visible();
	int get_x();
	int get_y();
	int get_z();
	int get_ox();
	int get_oy();
	int get_zoom_x();
	int get_zoom_y();
	int get_opacity();
	int get_blend_type();
	Color *get_color();
	Tone *get_tone();
	
	void set_viewport(Viewport* nviewport);
	void set_bitmap(Bitmap* nbitmap);
	void set_src_rect(Rect* nsrc_rect);
	void set_visible(bool nvisible);
	void set_x(int nx);
	void set_y(int ny);
	void set_z(int nz);
	void set_ox(int nox);
	void set_oy(int noy);
	void set_zoom_x(int nzoom_x);
	void set_zoom_y(int nzoom_y);
	void set_opacity(int nopacity);
	void set_blend_type(int nblend_type);
	void set_color(Color* ncolor);
	void set_tone(Tone* ntone);
	
	static std::map<int, Plane*> planes;
	static void add_plane(int id, Plane* plane);
	static void remove_plane(int id);
	
private:
	Viewport* viewport;
	Bitmap* bitmap;
	Rect src_rect;
	bool visible;
	int x;
	int y;
	int z;
	int ox;
	int oy;
	int zoom_x;
	int zoom_y;
	int opacity;
	int blend_type;
	Color *color;
	Tone *tone;
	
	bool disposed;
	
	int id;
	static int count;
};
#endif // __plane__
