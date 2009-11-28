#ifndef __window__
#define __window__

#include <map>
#include "SDL.h"
#include "viewport.h"
#include "bitmap.h"
#include "rect.h"

class Window {

public:
	Window();
	Window(Viewport* iviewport);
	~Window();
	
	void dispose();
	bool is_disposed();
	void update();

	void draw(SDL_Surface *screen);

	Viewport* get_viewport();
	Bitmap* get_windowskin();
	Bitmap* get_contents();
	bool get_stretch();
	Rect* get_cursor_rect();
	bool get_active();
	bool get_visible();
	bool get_pause();
	int get_x();
	int get_y();
	int get_width();
	int get_height();
	int get_z();
	int get_ox();
	int get_oy();
	int get_opacity();
	int get_back_opacity();
	int get_contents_opacity();

	void set_viewport(Viewport* nviewport);
	void set_windowskin(Bitmap* nwindowskin);
	void set_contents(Bitmap* ncontents);
	void set_stretch(bool nstretch);
	void set_cursor_rect(Rect* ncursor_rect);
	void set_active(bool nactive);
	void set_visible(bool nvisible);
	void set_pause(bool npause);
	void set_x(int nx);
	void set_y(int ny);
	void set_width(int nwidth);
	void set_height(int nheihgt);
	void set_z(int nz);
	void set_ox(int nox);
	void set_oy(int noy);
	void set_opacity(int nopacity);
	void set_back_opacity(int nback_opacity);
	void set_contents_opacity(int ncontents_opacity);

	static std::map<int, Window*> windows;
	static void add_window(int id, Window* window);
	static void remove_window(int id);

	void refresh();

protected:
	SDL_Surface* background;
	bool needs_refresh;

	Viewport* viewport;
	Bitmap* windowskin;
	Bitmap* contents;
	bool stretch;
	Rect cursor_rect;
	bool active;
	bool visible;
	bool pause;
	int x;
	int y;
	int width;
	int height;
	int z;
	int ox;
	int oy;
	int opacity;
	int back_opacity;
	int contents_opacity;
	
	bool disposed;
	
	int id;
	static int count;
};
#endif // __window__
