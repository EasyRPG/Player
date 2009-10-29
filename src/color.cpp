#include "color.h"

Color::Color(int r, int g, int b)
{
	red = r;
	green = g;
	blue = b;
	alpha = 255;
}


Color::Color(int r, int g, int b, int a)
{
	red = r;
	green = g;
	blue = b;
	alpha = a;
}

Color::~Color()
{
}

Uint32 Color::get_uint32() {
	//return SDL_MapRGBA(Graphics::get_pixelformat(), red, green, blue, alpha);
}

int Color::get_r() const
{
	return red;
}

int Color::get_g() const
{
	return green;
}

int Color::get_b() const
{
	return blue;
}

int Color::get_a() const
{
	return alpha;
}

void Color::set_r(int r)
{
	red = r;
}

void Color::set_g(int g)
{
	green = g;
}

void Color::set_b(int b)
{
	blue = b;
}

void Color::set_a(int a)
{
	alpha = a;
}
