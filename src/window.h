#ifndef __window__
#define __window__

#include <map>
#include "SDL.h"
#include "graphics.h"
#include "bitmap.h"
#include "rect.h"

class Viewport;

class Window {

public:
	Window();
	Window(Viewport* iviewport);
	~Window();
	
	void dispose();
	bool is_disposed() const;
	void update();

	/*void draw(SDL_Surface *screen);*/

	Viewport* viewport();

	void refresh();

protected:
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

protected:
	bool needs_refresh;	
	int id;
	static int count;
	Viewport* _viewport;
};
#endif // __window__
