#ifndef __tone__
#define __tone__

#include "SDL.h"

class Tone {

public:
	Tone();
	Tone(int r, int g, int b);
	Tone(int r, int g, int b, int g2);
	~Tone();
	
	Uint32 get_uint32();

	int get_r();
	int get_g();
	int get_b();
	int get_gray();
	
	void set_r(int r);
	void set_g(int g);
	void set_b(int b);
	void set_gray(int g);
	
protected:
	int red;
	int green;
	int blue;
	int gray;
};
#endif // __tone__
