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

namespace Graphics {
	SDL_Surface *screen;

	std::list<ZObj> zlist;
	std::list<ZObj>::iterator zlist_it;

    FPSmanager fps_manager;
	
	// Initialize Graphics
	void initialize()
	{
		// Create screen
		screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

        SDL_initFramerate(&fps_manager);
		
		// Set default frame rate
		const unsigned int frame_rate = 40;
        SDL_setFramerate(&fps_manager, frame_rate);
		
		// Set frame count to zero
		// Already done by SDL_initFramerate()
	}

	// Dispose Graphics resources
	void dispose()
	{
		
	}

	// Update Graphics
	void update()
	{
		SDL_FillRect(screen, NULL, 0); 
		
		zlist.sort(compare_zobj);
		
		for(zlist_it = zlist.begin(); zlist_it != zlist.end(); zlist_it++) {
			switch(zlist_it->get_type())
			{
				case TYPE_VIEWPORT:
					if (Viewport::viewports.count(zlist_it->get_id()) == 1) {
						Viewport::viewports[zlist_it->get_id()]->draw(screen);
					}
					break;
				case TYPE_SPRITE:
					if (Sprite::sprites.count(zlist_it->get_id()) == 1) {
						Sprite::sprites[zlist_it->get_id()]->draw(screen);
					}
					break;
				case TYPE_PLANE:
					if (Plane::planes.count(zlist_it->get_id()) == 1) {
						Plane::planes[zlist_it->get_id()]->draw(screen);
					}
					break;
				case TYPE_WINDOW:
					// Yeah Windows 7 Rulez!!!!!!!!!!!
					if (Window::windows.count(zlist_it->get_id()) == 1) {
						Window::windows[zlist_it->get_id()]->draw(screen);
					}
					break;
				case TYPE_TILEMAP:
					if (Tilemap::tilemaps.count(zlist_it->get_id()) == 1) {
						Tilemap::tilemaps[zlist_it->get_id()]->draw(screen);
					}
					break;
			}
		}
        SDL_Flip(screen);
        // Regulate FPS
        SDL_framerateDelay(&fps_manager);
	}

	void transition()
	{
		
	}

	void transition(int duration)
	{
		
	}

	void transition(int duration, std::string filename)
	{
		
	}

	void transition(int duration, std::string filename, int vague)
	{
		
	}

	int get_frame_rate()
	{
		return fps_manager.rate;
	}

	int get_frame_count()
	{
		return fps_manager.framecount;
	}

	void set_frame_rate(int fr)
	{
		SDL_setFramerate(&fps_manager, fr);
	}

	void set_frame_count(int fc)
	{
		fps_manager.framecount = fc;
	}
}
