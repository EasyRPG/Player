#include "window.h"
#include "zobj.h"
#include "SDL_rotozoom.h"

Window::Window():
    windowskin(NULL),
    contents(NULL),
    stretch(true),
    active(true),
    visible(true),
    pause(false),
    x(0),
    y(0),
    width(0),
    height(0),
    z(0),
    ox(0),
    oy(0),
    opacity(255),
    back_opacity(255),
    contents_opacity(255),
    disposed(false),
    needs_refresh(false),
    id(count)
{
    cursor_rect.x = 0;
    cursor_rect.y = 0;
    cursor_rect.w = 0;
    cursor_rect.h = 0;
	count++;
}

Window::Window(Viewport *iviewport)
{
	disposed = false;
	id = count;
//	Graphics::add_window(count, *this);
	count++;
	
	_viewport = iviewport;
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

/*std::map<int, Window*> Window::windows;*/
int Window::count = 0;

void Window::dispose()
{
	disposed = true;
}

bool Window::is_disposed() const
{
	return disposed;
}

void Window::update()
{
	
}

/*
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

bool Window::make_window() {
    
    if (windowskin == NULL)
        return false;

    SDL_Surface *back;
    SDL_Surface *tmp;
    SDL_Rect rect = {0, 0, 32, 32};

    tmp = Graphics::get_empty_dummy_surface(32, 32);

    SDL_BlitSurface(windowskin->surface, &rect, tmp, NULL);

    back = zoomSurface(tmp, 2.5, 2.5, 0);


    contents

    Graphics::draw(this);    
    

}

void Window::refresh()
{
    SDL_Surface* background;
    SDL_FreeSurface(background);

	SDL_Rect srcrect;
	SDL_Rect dstrect;
	SDL_Surface* temp;
	SDL_Surface* temp2;
    
	
    background = Graphics::get_empty_dummy_surface(width, height);
	
	// Back
	temp = Graphics::get_empty_dummy_surface(32, 32);
	
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
    temp = Graphics::get_empty_dummy_surface(16, 8);
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
	temp = Graphics::get_empty_dummy_surface(16, 8);
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
	temp = Graphics::get_empty_dummy_surface(8, 16);
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
	temp = Graphics::get_empty_dummy_surface(8, 16);
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
*/

Viewport* Window::viewport()
{
	return _viewport;
}
/*
void Window::add_window(int id, Window *window)
{
	windows[id] = window;
	ZObj zobj((*window).get_z(), id, TYPE_WINDOW, Graphics::get_frame_count());
	ZObj::zlist.push_back(zobj);
}

void Window::remove_window(int id)
{
	windows.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
*/