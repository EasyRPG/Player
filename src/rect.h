#ifndef __rect__
#define __rect__

#include "SDL.h"

class Rect {

public:
	Rect();
	Rect(int ix, int iy, int iwidth, int iheight);
	~Rect();

	void set(int nx, int ny, int nwidth, int nheight);
	SDL_Rect get_sdlrect();
	
	int get_x();
	int get_y();
	int get_width();
	int get_height();
	
	void set_x(int nx);
	void set_y(int ny);
	void set_width(int nwidth);
	void set_height(int nheight);
	
private:
	int x;
	int y;
	int width;
	int height;
};
#endif // __rect__
