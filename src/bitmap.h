#ifndef __bitmap__
#define __bitmap__

#include <map>
#include <string>
#include "defines.h"
#include "SDL.h"
#include "rect.h"
#include "color.h"
#include "font.h"

class Bitmap {

public:
	Bitmap(int width, int height);
	Bitmap(std::string filename);
	~Bitmap();
	
	void dispose();
	bool is_disposed(); 
	int width();
	int height();
	Rect *rect();
	void blt(int x, int y, Bitmap *src_bitmap, Rect *src_rect);
	void blt(int x, int y, Bitmap *src_bitmap, Rect *src_rect, int opacity);
	void stretch_blt(Rect *dest_rect, Bitmap *src_bitmap, Rect *src_rect);
	void stretch_blt(Rect *dest_rect, Bitmap *src_bitmap, Rect *src_rect, int opacity);
	void fill_rect(int x, int y, int width, int height, Color *color);
	void fill_rect(Rect *rect, Color *color);
	void clear();
	Color get_pixel(int x, int y);
	void set_pixel(int x, int y, Color *color);
	void hue_change(int hue);
	void draw_text(int x, int y, int width, int height, std::string str);
	void draw_text(int x, int y, int width, int height, std::string str, int align);
	void draw_text(Rect *rect, std::string str);
	void draw_text(Rect *rect, std::string str, int align);
	int text_size(std::string str);
	
	Font *get_font();
	void set_font(Font *nfont);
	
	static std::map<int, Bitmap*> bitmaps;
	static void add_bitmap(int id, Bitmap* bitmap);
	static void remove_bitmap(int id);
	
	SDL_Surface *surface;
	
private:
	Font *font;

	bool disposed;
	
	int id;
	static int count;
};
#endif // __bitmap__
