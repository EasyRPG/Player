#include "tone.h"

Tone::Tone()
{
	red = 0;
	green = 0;
	blue = 0;
	gray = 0;
}

Tone::Tone(int r, int g, int b)
{
	red = r;
	green = g;
	blue = b;
	gray = 0;
}


Tone::Tone(int r, int g, int b, int g2)
{
	red = r;
	green = g;
	blue = b;
	gray = g2;
}

Tone::~Tone()
{
}

Uint32 Tone::get_uint32() {
	//return SDL_MapRGBA(Graphics::get_pixelformat(), red, green, blue, gray);
}

int Tone::get_r()
{
	return red;
}

int Tone::get_g()
{
	return green;
}

int Tone::get_b()
{
	return blue;
}

int Tone::get_gray ()
{
	return gray ;
}

void Tone::set_r(int r)
{
	red = r;
}

void Tone::set_g(int g)
{
	green = g;
}

void Tone::set_b(int b)
{
	blue = b;
}

void Tone::set_gray(int g)
{
	gray = g;
}
