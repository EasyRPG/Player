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

namespace {
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    int bpp = SCREEN_BPP;
    Uint32 flags = 0;
}

namespace Graphics {
	SDL_Surface *screen;

	std::list<ZObj> zlist;
	std::list<ZObj>::iterator zlist_it;

    FPSmanager fps_manager;
	
	// Initialize Graphics
	void initialize()
	{
        if (!(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO)) {
            if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
                _fatal_error(SDL_GetError());
                exit(EXIT_FAILURE);
            }
        }
		// Create screen
        flags |= SDL_SWSURFACE;

		screen = SDL_SetVideoMode(width, height, bpp, flags);

        SDL_initFramerate(&fps_manager);
		
		// Set default frame rate
		const unsigned int frame_rate = 40;
        SDL_setFramerate(&fps_manager, frame_rate);

        // Disable Mouse Cursor
	    SDL_ShowCursor(SDL_DISABLE);
		
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

    /* We can blit safely on these surfaces */
    SDL_Surface* get_empty_dummy_surface(int w, int h) {
        
        SDL_Surface* dummy;

        dummy = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bpp, rmask, gmask, bmask, amask);
        if (dummy == NULL) {
            std::string serr("Internal error: Out of memory?\n");
            serr.append(SDL_GetError());
            _fatal_error(serr.c_str());
            exit(1);
        }

        return dummy;
    }

    /* We may need to lock these surfaces to blit. Could be slow!! */
    SDL_Surface* get_empty_real_surface(int w, int h) {
        
        SDL_Surface *ret_surface, *dummy;

        dummy = SDL_CreateRGBSurface(flags, w, h, bpp, rmask, gmask, bmask, amask);
        if (dummy == NULL) {
            std::string serr("Internal error: Out of memory?\n");
            serr.append(SDL_GetError());
            _fatal_error(serr.c_str());
            exit(EXIT_FAILURE);
        }
        
        ret_surface = SDL_DisplayFormat(dummy);
        if (ret_surface == NULL) {
            std::string serr("Internal error: Out of memory?\n");
            serr.append(SDL_GetError());
            _fatal_error(serr.c_str());
            exit(EXIT_FAILURE);
        }

        SDL_FreeSurface(dummy);

        return ret_surface;
    }
}
