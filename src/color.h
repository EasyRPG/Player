#ifndef __color__
#define __color__

#include "SDL.h"

class Color {

public:
    Color();
	Color(int r, int g, int b);
	Color(int r, int g, int b, int a);
	~Color();

	Uint32 get_uint32();
	
	int get_r() const;
	int get_g() const;
	int get_b() const;
	int get_a() const;
	
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
