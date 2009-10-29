#include "graphics.h"

// Sorting method for ZList
bool compare_zobj(ZObj &first, ZObj &second) {
    if (first.get_z() < second.get_z())
        return true;
    else if (first.get_z() > second.get_z())
		return false;
    else
        if (first.get_creation() < second.get_creation())
            return true;
        else
            return false;
}

// Initialize Graphics
void Graphics::initialize()
{
	// Create screen
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	
	// Set default frame rate
	frame_rate = 60;
	
	// Set frame count to zero
	frame_count = 0;
}

// Dispose Graphics resources
void Graphics::dispose()
{
	
}

// Update Graphics
void Graphics::update()
{
	SDL_FillRect(screen, &screen->clip_rect, 0); 
	
	frame_count += 1;
	
	ZObj::zlist.sort(compare_zobj);
	
    for(ZObj::zlist_it = ZObj::zlist.begin(); ZObj::zlist_it != ZObj::zlist.end(); ZObj::zlist_it++) {
        switch(ZObj::zlist_it->get_type())
		{
			case TYPE_VIEWPORT:
                if (Viewport::viewports.count(ZObj::zlist_it->get_id()) == 1) {
					Viewport::viewports[ZObj::zlist_it->get_id()]->draw(screen);
                }
				break;
			case TYPE_SPRITE:
                if (Sprite::sprites.count(ZObj::zlist_it->get_id()) == 1) {
					Sprite::sprites[ZObj::zlist_it->get_id()]->draw(screen);
                }
				break;
			case TYPE_PLANE:
                if (Plane::planes.count(ZObj::zlist_it->get_id()) == 1) {
					Plane::planes[ZObj::zlist_it->get_id()]->draw(screen);
                }
				break;
			case TYPE_WINDOW:
				// Yeah Windows Rulez!!!!!!!!!!!
                if (Window::windows.count(ZObj::zlist_it->get_id()) == 1) {
					Window::windows[ZObj::zlist_it->get_id()]->draw(screen);
                }
				break;
			case TYPE_TILEMAP:
                if (Tilemap::tilemaps.count(ZObj::zlist_it->get_id()) == 1) {
					Tilemap::tilemaps[ZObj::zlist_it->get_id()]->draw(screen);
                }
				break;
		}
    }
}

void Graphics::transition()
{
	
}

void Graphics::transition(int duration)
{
	
}

void Graphics::transition(int duration, std::string filename)
{
	
}

void Graphics::transition(int duration, std::string filename, int vague)
{
	
}

int Graphics::get_frame_rate()
{
	return frame_rate;
}

int Graphics::get_frame_count()
{
	return frame_count;
}

void Graphics::set_frame_rate(int fr)
{
	frame_rate = fr;
}

void Graphics::set_frame_count(int fc)
{
	frame_count = fc;
}
