#include "sprite.h"
#include "zobj.h"

Sprite::Sprite()
{
	disposed = false;
	id = count;
	add_sprite(count, this);
	count++;
	
	src_rect = new Rect(0, 0, 0, 0);
	visible = true;
	x = 0;
	y = 0;
	z = 0;
	ox = 0;
	oy = 0;
	zoom_x = 100;
	zoom_y = 100;
	angle = 0;
	mirror_x = false;
	mirror_y = false;
	bush_depth = 0;
	opacity = 255;
	blend_type = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);
	
}

Sprite::Sprite(Viewport *iviewport)
{
	disposed = false;
	id = count;
	add_sprite(count, this);
	count++;
	
	viewport = iviewport;
	src_rect = new Rect(0, 0, 0, 0);
	visible = true;
	x = 0;
	y = 0;
	z = 0;
	ox = 0;
	oy = 0;
	zoom_x = 100;
	zoom_y = 100;
	angle = 0;
	mirror_x = false;
	mirror_y = false;
	bush_depth = 0;
	opacity = 255;
	blend_type = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);
}

Sprite::~Sprite()
{
//	Graphics::remove_sprite(id);
}

std::map<int, Sprite*> Sprite::sprites;
int Sprite::count = 0;

void Sprite::dispose()
{
	disposed = true;
}

bool Sprite::is_disposed()
{
	return disposed;
}

void Sprite::flash(Color *flash_color, int duration)
{
	
}

void Sprite::update()
{
	
}

void Sprite::draw(SDL_Surface *screen)
{
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y; 
		if(SDL_BlitSurface(bitmap->surface, &src_rect->get_sdlrect(), screen, &dstrect) == -1)
		{
			// Error
		}
}

Viewport* Sprite::get_viewport()
{
	return viewport;
}

Bitmap* Sprite::get_bitmap()
{
	return bitmap;
}

Rect* Sprite::get_src_rect()
{
	return src_rect;
}

bool Sprite::get_visible()
{
	return visible;
}

int Sprite::get_x()
{
	return x;
}

int Sprite::get_y()
{
	return y;
}

int Sprite::get_z()
{
	return z;
}

int Sprite::get_ox(){
	return ox;
}

int Sprite::get_oy()
{
	return oy;
}

int Sprite::get_zoom_x()
{
	return zoom_x;
}

int Sprite::get_zoom_y()
{
	return zoom_y;
}

int Sprite::get_angle()
{
	return angle;
}

bool Sprite::get_mirror_x()
{
	return mirror_x;
}

bool Sprite::get_mirror_y()
{
	return mirror_y;
}

int Sprite::get_bush_depth()
{
	return bush_depth;
}

int Sprite::get_opacity()
{
	return opacity;
}

int Sprite::get_blend_type()
{
	return blend_type;
}

Color* Sprite::get_color()
{
	return color;
}

Tone* Sprite::get_tone()
{
	return tone;
}


void Sprite::set_viewport(Viewport* nviewport)
{
	viewport = nviewport;
}

void Sprite::set_bitmap(Bitmap* nbitmap)
{
	bitmap = nbitmap;
}

void Sprite::set_src_rect(Rect* nsrc_rect)
{
	src_rect = nsrc_rect;
}

void Sprite::set_visible(bool nvisible)
{
	visible = nvisible;
}

void Sprite::set_x(int nx)
{
	x = nx;
}

void Sprite::set_y(int ny)
{
	y = ny;
}

void Sprite::set_z(int nz)
{
	z = nz;
}

void Sprite::set_ox(int nox)
{
	ox = nox;
}

void Sprite::set_oy(int noy)
{
	oy = noy;
}

void Sprite::set_zoom_x(int nzoom_x)
{
	zoom_x = nzoom_x;
}

void Sprite::set_zoom_y(int nzoom_y)
{
	zoom_y = nzoom_y;
}

void Sprite::set_angle(int nangle)
{
	angle = nangle;
}

void Sprite::set_mirror_x(bool nmirror_x)
{
	mirror_x = nmirror_x;
}

void Sprite::set_mirror_y(bool nmirror_y)
{
	mirror_y = nmirror_y;
}

void Sprite::set_bush_depth(int nbush_depth)
{
	bush_depth = nbush_depth;
}

void Sprite::set_opacity(int nopacity)
{
	opacity = nopacity;
}

void Sprite::set_blend_type(int nblend_type)
{
	blend_type = nblend_type;
}

void Sprite::set_color(Color* ncolor)
{
	color = ncolor;
}

void Sprite::set_tone(Tone* ntone)
{
	tone = ntone;
}

void Sprite::add_sprite(int id, Sprite *sprite)
{
	sprites[id] = sprite;
	ZObj zobj((*sprite).get_z(), id, TYPE_SPRITE, 0);
	Graphics::zlist.push_back(zobj);
}

void Sprite::remove_sprite(int id)
{
	sprites.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
