#include "window.h"
#include "zobj.h"
#include "SDL_rotozoom.h"

Window::Window()
{
	background = NULL;
	needs_refresh = false;
	
	disposed = false;
	id = count;
//	Graphics::add_window(count, *this);
	count++;
	
	stretch = true;
	cursor_rect.x = 0;
    cursor_rect.y = 0;
    cursor_rect.w = 0;
    cursor_rect.h = 0;
	active = true;
	visible = true;
	pause = false;
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	z = 0;
	ox = 0;
	oy = 0;
	opacity = 255;
	back_opacity = 255;
	contents_opacity = 255;
}

Window::Window(Viewport *iviewport)
{
	disposed = false;
	id = count;
//	Graphics::add_window(count, *this);
	count++;
	
	viewport = iviewport;
	stretch = true;
	cursor_rect.x = 0;
    cursor_rect.y = 0;
    cursor_rect.w = 0;
    cursor_rect.h = 0;
	active = true;
	visible = true;
	pause = false;
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	z = 0;
	ox = 0;
	oy = 0;
	opacity = 255;
	back_opacity = 255;
	contents_opacity = 255;
}

Window::~Window()
{
//	Graphics::remove_window(id);
}

std::map<int, Window*> Window::windows;
int Window::count = 0;

void Window::dispose()
{
	disposed = true;
}

bool Window::is_disposed()
{
	return disposed;
}

void Window::update()
{
	
}

void Window::draw(SDL_Surface *screen)
{
	if(needs_refresh) {
		refresh();
		needs_refresh = false;
	}
	
	SDL_Rect dstrect;

    dstrect.x = x;
    dstrect.y = y;
    SDL_BlitSurface(background, NULL, screen, &dstrect);
}

void Window::refresh()
{
	if(background != NULL) {
		SDL_FreeSurface(background);
	}
	SDL_Rect srcrect;
	SDL_Rect dstrect;
	SDL_Surface* temp;
	SDL_Surface* temp2;
	
	background = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, SCREEN_BPP, rmask, gmask, bmask, amask);
	
	// Back
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, SCREEN_BPP, rmask, gmask, bmask, amask);
	
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = 32;
	srcrect.h = 32;
	
	SDL_BlitSurface(windowskin->surface, &srcrect, temp, NULL);
	
	temp2 = zoomSurface(temp, ((double)width) / 32, ((double)height) / 32, 0);
	
	SDL_BlitSurface(temp2, NULL, background, NULL);
	
	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
	
	// Corners
	srcrect.x = 32;
	srcrect.w = 8;
	srcrect.h = 8;
	dstrect.x = 0;
	dstrect.y = 0;
	SDL_BlitSurface(windowskin->surface, &srcrect, background, &dstrect);
	
	srcrect.x = 64 - 8;
	dstrect.x = width - 8;
	SDL_BlitSurface(windowskin->surface, &srcrect, background, &dstrect);
	
	srcrect.y = 32 - 8;
	dstrect.y = height - 8;
	SDL_BlitSurface(windowskin->surface, &srcrect, background, &dstrect);
	
	srcrect.x = 32;
	dstrect.x = 0;
	SDL_BlitSurface(windowskin->surface, &srcrect, background, &dstrect);
	
	// Border Up
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 8, SCREEN_BPP, rmask, gmask, bmask, amask);
	srcrect.x = 32 + 8;
	srcrect.y = 0;
	srcrect.w = 16;
	srcrect.h = 8;
	SDL_BlitSurface(windowskin->surface, &srcrect, temp, NULL);
	temp2 = zoomSurface(temp, ((double)width - 16) / 16, 1, 0);
	dstrect.x = 8;
	dstrect.y = 0;
	dstrect.w = width - 16;
	dstrect.h = 8;
	SDL_BlitSurface(temp2, NULL, background, &dstrect);
	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
	
	// Border Down
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 8, SCREEN_BPP, rmask, gmask, bmask, amask);
	srcrect.x = 32 + 8;
	srcrect.y = 32 - 8;
	srcrect.w = 16;
	srcrect.h = 8;
	SDL_BlitSurface(windowskin->surface, &srcrect, temp, NULL);
	temp2 = zoomSurface(temp, ((double)width - 16) / 16, 1, 0);
	dstrect.x = 8;
	dstrect.y = height - 8;
	dstrect.w = width - 16;
	dstrect.h = 8;
	SDL_BlitSurface(temp2, NULL, background, &dstrect);
	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
	
	// Border Left
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 8, 16, SCREEN_BPP, rmask, gmask, bmask, amask);
	srcrect.x = 32;
	srcrect.y = 8;
	srcrect.w = 8;
	srcrect.h = 16;
	SDL_BlitSurface(windowskin->surface, &srcrect, temp, NULL);
	temp2 = zoomSurface(temp, 1, ((double)height - 16) / 16, 0);
	dstrect.x = 0;
	dstrect.y = 8;
	dstrect.w = 8;
	dstrect.h = height - 16;
	SDL_BlitSurface(temp2, NULL, background, &dstrect);
	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
	
	// Border Right
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 8, 16, SCREEN_BPP, rmask, gmask, bmask, amask);
	srcrect.x = 64 - 8;
	srcrect.y = 8;
	srcrect.w = 8;
	srcrect.h = 16;
	SDL_BlitSurface(windowskin->surface, &srcrect, temp, NULL);
	temp2 = zoomSurface(temp, 1, ((double)height - 16) / 16, 0);
	dstrect.x = width - 8;
	dstrect.y = 8;
	dstrect.w = 8;
	dstrect.h = height - 16;
	SDL_BlitSurface(temp2, NULL, background, &dstrect);
	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
}

Viewport* Window::get_viewport()
{
	return viewport;
}

Bitmap* Window::get_windowskin()
{
	return windowskin;
}

Bitmap* Window::get_contents()
{
	return contents;
}

bool Window::get_stretch()
{
	return stretch;
}

Rect* Window::get_cursor_rect()
{
	return &cursor_rect;
}

bool Window::get_active()
{
	return active;
}

bool Window::get_visible()
{
	return visible;
}

bool Window::get_pause()
{
	return pause;
}

int Window::get_x()
{
	return x;
}

int Window::get_y()
{
	return y;
}

int Window::get_width()
{
	return width;
}

int Window::get_height()
{
	return height;
}

int Window::get_z()
{
	return z;
}

int Window::get_ox()
{
	return ox;
}

int Window::get_oy(){
	return oy;
}

int Window::get_opacity()
{
	return opacity;
}

int Window::get_back_opacity(){
	return back_opacity;
}

int Window::get_contents_opacity()
{
	return contents_opacity;
}

void Window::set_viewport(Viewport* nviewport)
{
	viewport = nviewport;
}

void Window::set_windowskin(Bitmap* nwindowskin)
{
	windowskin = nwindowskin;
	needs_refresh = true;
}

void Window::set_contents(Bitmap* ncontents)
{
	contents = ncontents;
}

void Window::set_stretch(bool nstretch)
{
	stretch = nstretch;
	needs_refresh = true;
}

void Window::set_cursor_rect(Rect* ncursor_rect)
{
	cursor_rect.x = ncursor_rect->x;
    cursor_rect.y = ncursor_rect->y;
    cursor_rect.w = ncursor_rect->w;
    cursor_rect.h = ncursor_rect->h;
}

void Window::set_active(bool nactive)
{
	active = nactive;
}

void Window::set_visible(bool nvisible)
{
	visible = nvisible;
}

void Window::set_pause(bool npause)
{
	pause = npause;
}

void Window::set_x(int nx)
{
	x = nx;
}

void Window::set_y(int ny)
{
	y = ny;
}

void Window::set_width(int nwidth)
{
	width = nwidth;
	needs_refresh = true;
}

void Window::set_height(int nheihgt)
{
	height = nheihgt;
	needs_refresh = true;
}

void Window::set_z(int nz)
{
	z = nz;
}

void Window::set_ox(int nox)
{
	ox = nox;
}

void Window::set_oy(int noy)
{
	oy = noy;
}

void Window::set_opacity(int nopacity)
{
	opacity = nopacity;
}

void Window::set_back_opacity(int nback_opacity)
{
	back_opacity = nback_opacity;
}

void Window::set_contents_opacity(int ncontents_opacity)
{
	contents_opacity = ncontents_opacity;
}

void Window::add_window(int id, Window *window)
{
	windows[id] = window;
	ZObj zobj((*window).get_z(), id, TYPE_WINDOW, Graphics::get_frame_count());
	Graphics::zlist.push_back(zobj);
}

void Window::remove_window(int id)
{
	windows.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
