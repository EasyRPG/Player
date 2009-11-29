#include "bitmap.h"
#include "SDL_image.h"
#include "tools.h"
#include "graphics.h"

Bitmap::Bitmap(int width, int height):
      disposed(false),
      id(-1)
{
    surface = Graphics::get_empty_real_surface(width, height);
}

Bitmap::Bitmap(std::string& filename, int _id):
      disposed(false)
{
	SDL_Surface* temp = NULL;
    std::string serr;
    
#ifdef WIN32
    int file_ext;
    file_ext = get_img_extension(filename);
    switch (file_ext) {
        case PNG:
        case BMP:
            temp = IMG_Load(filename.c_str());
            break;
        case XYZ:
            temp = load_XYZ(filename);
            break;
        default:
            serr = "Couldn't open ";
            serr.append(filename);
            _fatal_error(serr.c_str());
            zombie = true;
            return;
    }
#else
    // TODO Implement file extension guessing for non WIN32 systems
    filename.append(".png");
    temp = IMG_Load(filename.c_str());
#endif
 
	if (temp == NULL) {
        std::string s(IMG_GetError());
        _fatal_error(s.c_str());
        zombie = true;
        return;
	}
	else
	{
		surface = SDL_DisplayFormat(temp);
		SDL_FreeSurface(temp);
	}
    
    id = _id;
    zombie = false;
}

Bitmap::~Bitmap()
{
//	remove_bitmap(id);
}

//int Bitmap::count = 0;

void Bitmap::dispose()
{
	if(!disposed) {
		SDL_FreeSurface(surface);
		disposed = true;
	}
}

bool Bitmap::is_disposed() const
{
	return disposed;
}

int Bitmap::width() const
{
	return surface->w;
}

int Bitmap::height() const
{
	return surface->h;
}

Rect* Bitmap::rect()
{
    Rect *r;
    r = new Rect;
    r->x = 0;
    r->y = 0;
    r->w = surface->w;
    r->h = surface->h;
	return r;
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
	SDL_FillRect(surface, &rect, color->get_uint32());
}

void Bitmap::fill_rect(Rect *rect, Color *color)
{
	//SDL_FillRect(surface, &rect->get_sdlrect(), (*color).get_uint32());
}

void Bitmap::clear()
{
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = surface->w;
	rect.h = surface->h;
	SDL_FillRect(surface, &rect, 0);
}

Color Bitmap::get_pixel(int x, int y)
{
	Uint8* pixel = (Uint8*)surface->pixels + (x + y * surface->w) * 4;
	Color color;
	//Uint8 r, g, b, a;
	//SDL_GetRGBA(pixel[x + y * surface->w], Graphics::get_pixelformat(), &r, &g, &b, &a);
	color.set_r(pixel[0]);
	color.set_g(pixel[1]);
	color.set_b(pixel[2]);
	color.set_a(pixel[3]);
	return color;
}

void Bitmap::set_pixel(int x, int y, Color* color)
{
	Uint32* pixel = reinterpret_cast<Uint32*>(surface->pixels) + (x + y * surface->w) * 4;
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
	return 0;
}

Font* Bitmap::get_font()
{
	return font;
}

void Bitmap::set_font(Font* nfont)
{
	font = nfont;
}

/*void Bitmap::add_bitmap(int id, Bitmap* bitmap)
{
	bitmaps[id] = bitmap;
}*/

/*void Bitmap::remove_bitmap(int id)
{
	bitmaps.erase(id);
}*/
