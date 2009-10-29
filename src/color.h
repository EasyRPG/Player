#ifndef __color__
#define __color__

#include "SDL.h"

class Color {

public:
	Color(int r, int g, int b);
	Color(int r, int g, int b, int a);
	~Color();

	Uint32 get_uint32();
	
	int get_r();
	int get_g();
	int get_b();
	int get_a();
	
	void set_r(int r);
	void set_g(int g);
	void set_b(int b);
	void set_a(int a);
	
private:
	int red;
	int green;
	int blue;
	int alpha;
};
#endif // __color__
