#include "bitmap.h"

Bitmap::Bitmap(int width, int height)
{
	bitmap = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, SCREEN_BPP, 0, 0, 0, 0);
	disposed = false;
	
	id = count;
	add_bitmap(id, this);
	count++;
}

Bitmap::Bitmap(std::string filename)
{
	/* TODO */
	disposed = false;
}

Bitmap::~Bitmap()
{
	remove_bitmap(id);
}

int Bitmap::count = 0;

void Bitmap::dispose()
{
	SDL_FreeSurface(bitmap);
	disposed = true;
}

bool Bitmap::is_disposed()
{
	return disposed;
}

int Bitmap::width()
{
	return bitmap->w;
}

int Bitmap::height()
{
	return bitmap->h;
}

Rect* Bitmap::rect()
{
	return new Rect(0, 0, bitmap->w, bitmap->h);
}

void Bitmap::blt(int x, int y, Bitmap *src_bitmap, Rect *src_rect)
{
	
}

void Bitmap::blt(int x, int y, Bitmap *src_bitmap, Rect *src_rect, int opacity)
{
	
}

void Bitmap::stretch_blt(Rect *dest_rect, Bitmap *src_bitmap, Rect *src_rect)
{
	
}

void Bitmap::stretch_blt(Rect *dest_rect, Bitmap *src_bitmap, Rect *src_rect, int opacity)
{
	
}

void Bitmap::fill_rect(int x, int y, int width, int height, Color *color)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = width;
	rect.h = height;
	SDL_FillRect(bitmap, &rect, color->get_uint32());
}

void Bitmap::fill_rect(Rect *rect, Color *color)
{
	//SDL_FillRect(bitmap, &rect->get_sdlrect(), (*color).get_uint32());
}

void Bitmap::clear()
{
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = bitmap->w;
	rect.h = bitmap->h;
	SDL_FillRect(bitmap, &rect, 0);
}

Color Bitmap::get_pixel(int x, int y)
{
	Uint8* pixel = (Uint8*)bitmap->pixels + (x + y * bitmap->w) * 4;
	Color color;
	//Uint8 r, g, b, a;
	//SDL_GetRGBA(pixel[x + y * bitmap->w], Graphics::get_pixelformat(), &r, &g, &b, &a);
	color.set_r(pixel[0]);
	color.set_g(pixel[1]);
	color.set_b(pixel[2]);
	color.set_a(pixel[3]);
	return color;
}

void Bitmap::set_pixel(int x, int y, Color* color)
{
	Uint32* pixel = reinterpret_cast<Uint32*>(bitmap->pixels) + (x + y * bitmap->w) * 4;
	pixel[0] = (*color).get_uint32();
}

void Bitmap::hue_change(int hue){
	
}

void Bitmap::draw_text(int x, int y, int width, int height, std::string str)
{
	
}

void Bitmap::draw_text(int x, int y, int width, int height, std::string str, int align)
{
	
}

void Bitmap::draw_text(Rect* rect, std::string str)
{
	
}

void Bitmap::draw_text(Rect* rect, std::string str, int align)
{
	
}

int Bitmap::text_size(std::string str)
{
	
}

Font* Bitmap::get_font()
{
	return font;
}

void Bitmap::set_font(Font* nfont)
{
	font = nfont;
}

void Bitmap::add_bitmap(int id, Bitmap* bitmap)
{
	bitmaps[id] = bitmap;
}

void Bitmap::remove_bitmap(int id)
{
	bitmaps.erase(id);
}
