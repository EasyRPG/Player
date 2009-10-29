#include "window.h"
#include "zobj.h"

Window::Window()
{
	disposed = false;
	id = count;
	Graphics::add_window(count, *this);
	count++;
	
	stretch = true;
	cursor_rect = new Rect(0, 0, 0, 0);
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
	Graphics::add_window(count, *this);
	count++;
	
	viewport = iviewport;
	stretch = true;
	cursor_rect = new Rect(0, 0, 0, 0);
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
	Graphics::remove_window(id);
}

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

}

Viewport* get_viewport()
{
	return viewport;
}

Bitmap* get_windowskin()
{
	return windowskin;
}

Bitmap* get_contents()
{
	return contents;
}

bool get_stretch()
{
	return stretch;
}

Rect get_cursor_rect()
{
	return cursor_rect;
}

bool get_active()
{
	return active;
}

bool get_visible()
{
	return visible;
}

bool get_pause()
{
	return pause;
}

int get_x()
{
	return x;
}

int get_y()
{
	return y;
}

int get_width()
{
	return width;
}

int get_height()
{
	return height;
}

int get_z()
{
	return z;
}

int get_ox()
{
	return ox;
}

int get_oy(){
	return oy;
}

int get_opacity()
{
	return opacity;
}

int get_back_opacity(){
	return back_opacity;
}

int get_contents_opacity()
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
}

void Window::set_contents(Bitmap* ncontents)
{
	contents = ncontents;
}

void Window::set_stretch(bool nstretch)
{
	stretch = nstretch;
}

void Window::set_cursor_rect(Rect ncursor_rect)
{
	cursor_rect = ncursor_rect;
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
}

void Window::set_height(int nheihgt)
{
	heihgt = nheihgt;
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
	ZObj zobj((*window).get_z(), id, TYPE_WINDOW, frame_count);
	ZObj::zlist.push_back(zobj);
}

void Window::remove_window(int id)
{
	windows.erase(id);
	ZObj::zlist.remove_if(remove_zobj_id(id));
}
