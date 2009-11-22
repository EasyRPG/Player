#include "plane.h"
#include "zobj.h"

Plane::Plane()
{
	disposed = false;
	id = count;
	add_plane(count, this);
	count++;
	
	src_rect.x = 0;
    src_rect.y = 0;
    src_rect.w = 0;
    src_rect.h = 0;

	visible = true;
	x = 0;
	y = 0;
	z = 0;
	ox = 0;
	oy = 0;
	zoom_x = 100;
	zoom_y = 100;
	opacity = 255;
	blend_type = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);
}

Plane::Plane(Viewport *iviewport)
{
	disposed = false;
	id = count;
	add_plane(count, this);
	count++;
	
	viewport = iviewport;
	src_rect.x = 0;
    src_rect.y = 0;
    src_rect.w = 0;
    src_rect.h = 0;
	visible = true;
	x = 0;
	y = 0;
	z = 0;
	ox = 0;
	oy = 0;
	zoom_x = 100;
	zoom_y = 100;
	opacity = 255;
	blend_type = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);
}

Plane::~Plane()
{
	remove_plane(id);
}

std::map<int, Plane*> Plane::planes;
int Plane::count = 0;

void Plane::dispose()
{
	disposed = true;
}

bool Plane::is_disposed()
{
	return disposed;
}


void Plane::draw(SDL_Surface *screen)
{

}

Viewport* Plane::get_viewport()
{
	return viewport;
}

Bitmap* Plane::get_bitmap()
{
	return bitmap;
}

Rect *Plane::get_src_rect()
{
	return &src_rect;
}

bool Plane::get_visible()
{
	return visible;
}

int Plane::get_x()
{
	return x;
}

int Plane::get_y()
{
	return y;
}

int Plane::get_z()
{
	return z;
}

int Plane::get_ox(){
	return ox;
}

int Plane::get_oy()
{
	return oy;
}

int Plane::get_zoom_x()
{
	return zoom_x;
}

int Plane::get_zoom_y()
{
	return zoom_y;
}

int Plane::get_opacity()
{
	return opacity;
}

int Plane::get_blend_type()
{
	return blend_type;
}

Color *Plane::get_color()
{
	return color;
}

Tone *Plane::get_tone()
{
	return tone;
}


void Plane::set_viewport(Viewport* nviewport)
{
	viewport = nviewport;
}

void Plane::set_bitmap(Bitmap* nbitmap)
{
	bitmap = nbitmap;
}

void Plane::set_src_rect(Rect* nsrc_rect)
{
	src_rect.x = nsrc_rect->x;
    src_rect.y = nsrc_rect->y;
    src_rect.w = nsrc_rect->w;
    src_rect.h = nsrc_rect->h;
}

void Plane::set_visible(bool nvisible)
{
	visible = nvisible;
}

void Plane::set_x(int nx)
{
	x = nx;
}

void Plane::set_y(int ny)
{
	y = ny;
}

void Plane::set_z(int nz)
{
	y = nz;
}

void Plane::set_ox(int nox)
{
	ox = nox;
}

void Plane::set_oy(int noy)
{
	oy = noy;
}

void Plane::set_zoom_x(int nzoom_x)
{
	zoom_x = nzoom_x;
}

void Plane::set_zoom_y(int nzoom_y)
{
	zoom_y = nzoom_y;
}

void Plane::set_opacity(int nopacity)
{
	opacity = nopacity;
}

void Plane::set_blend_type(int nblend_type)
{
	blend_type = nblend_type;
}

void Plane::set_color(Color* ncolor)
{
	color = ncolor;
}

void Plane::set_tone(Tone* ntone)
{
	tone = ntone;
}

void Plane::add_plane(int id, Plane *plane)
{
	planes[id] = plane;
	ZObj zobj((*plane).get_z(), id, TYPE_PLANE, 0);
	Graphics::zlist.push_back(zobj);
}

void Plane::remove_plane(int id)
{
	planes.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
