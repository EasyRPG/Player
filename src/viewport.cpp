#include "viewport.h"
#include "graphics.h"
#include "zobj.h"

Viewport::Viewport(int ix, int iy, int iwidth, int iheight)
{
	disposed = false;
	id = count;
//	Graphics::add_viewport(count, this);
	count++;
	
    rect.x = ix;
    rect.y = iy;
    rect.w = iwidth;
    rect.h = iheight;

	visible = true;
	z = 0;
	ox = 0;
	oy = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);
}

Viewport::Viewport(Rect *irect)
{
	disposed = false;
	id = count;
//	Graphics::add_viewport(count, this);
	count++;
	
//	viewport = viewport;
	rect.x = irect->x;
    rect.y = irect->y;
    rect.w = irect->w;
    rect.h = irect->h;

	visible = true;
	z = 0;
	ox = 0;
	oy = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);
}

Viewport::~Viewport()
{
//	Graphics::remove_viewport(id);
}

std::map<int, Viewport*> Viewport::viewports;
int Viewport::count = 0;

void Viewport::dispose()
{
	disposed = true;
}

bool Viewport::is_disposed()
{
	return disposed;
}

void Viewport::flash(Color *flash_color, int duration)
{
	
}

void Viewport::update()
{
	
}

void Viewport::draw(SDL_Surface *screen)
{

}

Rect* Viewport::get_rect()
{
	return &rect;
}

bool Viewport::get_visible()
{
	return visible;
}

int Viewport::get_z()
{
	return z;
}

int Viewport::get_ox()
{
	return ox;
}

int Viewport::get_oy()
{
	return oy;
}

Color* Viewport::get_color()
{
	return color;
}

Tone* Viewport::get_tone()
{
	return tone;
}

void Viewport::set_rect(Rect* nrect)
{
	rect.x = nrect->x;
    rect.y = nrect->y;
    rect.w = nrect->w;
    rect.h = nrect->h;
}

void Viewport::set_visible(bool nvisible)
{
	visible = nvisible;
}

void Viewport::set_z(int nz)
{
	z = nz;
}

void Viewport::set_ox(int nox)
{
	ox = nox;
}

void Viewport::set_oy(int noy)
{
	oy = noy;
}

void Viewport::set_color(Color* ncolor)
{
	color = ncolor;
}

void Viewport::set_tone(Tone* ntone)
{
	tone = ntone;
}

void Viewport::add_viewport(int id, Viewport *viewport)
{
	viewports[id] = viewport;
	ZObj zobj((*viewport).get_z(), id, TYPE_VIEWPORT, 0);
	ZObj::zlist.push_back(zobj);
}

void Viewport::remove_viewport(int id)
{
	viewports.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
