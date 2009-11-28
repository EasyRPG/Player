#include "sprite.h"
#include "zobj.h"

Sprite::Sprite()
{
	disposed = false;
	id = count;

    bitmap = NULL;
    viewport = NULL;
	
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
	angle = 0;
	mirror_x = false;
	mirror_y = false;
	bush_depth = 0;
	opacity = 255;
	blend_type = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);

    add_sprite(count, this);
    count++;
	
}

Sprite::Sprite(Viewport *iviewport)
{
	disposed = false;
	id = count;

    bitmap = NULL;
	
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
	angle = 0;
	mirror_x = false;
	mirror_y = false;
	bush_depth = 0;
	opacity = 255;
	blend_type = 0;
	color = new Color(0, 0, 0);
	tone = new Tone(0, 0, 0);

    add_sprite(count, this);
    count++;
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
    if (bitmap == NULL)
        return;
    
    SDL_Rect dstrect;
	dstrect.x = x;
	dstrect.y = y; 
    dstrect.w = bitmap->width();
    dstrect.h = bitmap->height();
	
    if(SDL_BlitSurface(bitmap->surface, &src_rect, screen, &dstrect) < 0)
	{
		_fatal_error(SDL_GetError());
        exit(EXIT_FAILURE);
	}
}

Viewport* Sprite::get_viewport() const
{
	return viewport;
}

Bitmap* Sprite::get_bitmap() const
{
	return bitmap;
}

Rect* Sprite::get_src_rect()
{
	return &src_rect;
}

bool Sprite::get_visible() const 
{
	return visible;
}

int Sprite::get_x() const
{
	return x;
}

int Sprite::get_y() const 
{
	return y;
}

int Sprite::get_z() const 
{
	return z;
}

int Sprite::get_ox() const
{
	return ox;
}

int Sprite::get_oy() const
{
	return oy;
}

int Sprite::get_zoom_x() const
{
	return zoom_x;
}

int Sprite::get_zoom_y() const
{
	return zoom_y;
}

int Sprite::get_angle() const
{
	return angle;
}

bool Sprite::get_mirror_x() const
{
	return mirror_x;
}

bool Sprite::get_mirror_y() const
{
	return mirror_y;
}

int Sprite::get_bush_depth() const
{
	return bush_depth;
}

int Sprite::get_opacity() const
{
	return opacity;
}

int Sprite::get_blend_type() const
{
	return blend_type;
}

Color* Sprite::get_color() const
{
	return color;
}

Tone* Sprite::get_tone() const
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
    src_rect.w = bitmap->width();
    src_rect.h = bitmap->height();

}

void Sprite::set_src_rect(Rect* nsrc_rect)
{
	src_rect.x = nsrc_rect->x;
    src_rect.y = nsrc_rect->y;
    src_rect.w = nsrc_rect->w;
    src_rect.h = nsrc_rect->h;
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
	ZObj zobj(sprite->get_z(), TYPE_SPRITE, id, 0);
	Graphics::zlist.push_back(zobj);
}

void Sprite::remove_sprite(int id)
{
	sprites.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
