#include "rect.h"

Rect::Rect()
{
	x = 0;
	y = 0;
	width = 0;
	height = 0;
}

Rect::Rect(int ix, int iy, int iwidth, int iheight)
{
	x = ix;
	y = iy;
	width = iwidth;
	height = iheight;
}

Rect::~Rect()
{
}

void Rect::set(int nx, int ny, int nwidth, int nheight)
{
	x = nx;
	y = ny;
	width = nwidth;
	height = nheight;
}

SDL_Rect Rect::get_sdlrect()
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = width;
	rect.h = height;
	return rect;
}

int Rect::get_x()
{
	return x;
}

int Rect::get_y()
{
	return y;
}

int Rect::get_width()
{
	return width;
}

int Rect::get_height()
{
	return height;
}

void Rect::set_x(int nx)
{
	x = nx;
}

void Rect::set_y(int ny)
{
	y = ny;
}

void Rect::set_width(int nwidth)
{
	width = nwidth;
}

void Rect::set_height(int nheight)
{
	height = nheight;
}
