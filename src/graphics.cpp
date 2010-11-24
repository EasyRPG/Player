/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "graphics.h"
#include "drawable.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "SDL_ttf.h"
#include "system.h"
#include <sstream>

namespace {
}

////////////////////////////////////////////////////////////
// Global Variables
////////////////////////////////////////////////////////////
namespace Graphics {
	int fps;
	int framerate;
	int framecount;
	double framerate_interval;
	unsigned long creation;
	unsigned long ID;
	unsigned long last_ticks;
	unsigned long last_ticks_wait;
	unsigned long next_ticks_fps;

	bool is_in_transition_yet;
	bool frozen;

	TransitionType actual_transition;

	Uint32 default_backcolor;

	bool fps_showing;

	std::map<unsigned long, Drawable*> drawable_map;
	std::map<unsigned long, Drawable*>::iterator it_drawable_map;
	std::list<ZObj> zlist;
	std::list<ZObj>::iterator it_zlist;
}

namespace {
	bool zlist_needs_sorting;
	bool prepare_transition;
	bool skip_draw;
	int transition_frames;
	int transition_current_frame;
	int transition_increment;
	bool wait_for_transition;
	SDL_Surface* fake_screen;
	SDL_Surface* blank_screen;
	Font* font;
#ifdef USE_FIXED_TIMESTEP_FPS
	const int MAXIMUM_FRAME_RATE = 60;
	const int MINIMUM_FRAME_RATE = 15;
	const double UPDATE_INTERVAL = 1.0 / MAXIMUM_FRAME_RATE;
	const int MAX_CYCLES_PER_FRAME = MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE;
	double last_frame_time = 0;
	double cycles_leftover = 0;
	double current_time;
	double update_iterations;
	bool start;
#endif
}

////////////////////////////////////////////////////////////
// Initialize
////////////////////////////////////////////////////////////
void Graphics::Init() {
	fps = 0;
	framerate = DEFAULT_FPS;
	framecount = 0;
	creation = 0;
	ID = 0;
	framerate_interval = 1000.0 / DEFAULT_FPS;
	last_ticks = SDL_GetTicks() + (long)framerate_interval;
	next_ticks_fps = last_ticks + 1000;

	zlist_needs_sorting = false;

	transition_frames = 0;
	transition_current_frame = 0;
	transition_increment = 0;
	actual_transition = NoTransition;
	fake_screen = NULL;
	blank_screen = SDL_DisplayFormat(Player::main_surface);
	SDL_FillRect(blank_screen, NULL, 0);

	is_in_transition_yet = false;
	wait_for_transition = false;
	prepare_transition = false;
	frozen = false;
	skip_draw = false;

	if (TTF_Init() == -1) {
		Output::Error("Couldn't initialize SDL_ttf library.\n%s\n", TTF_GetError());
	}

	default_backcolor = 0;

#ifdef GEKKO
	fps_showing = true;
#else
	fps_showing = false;
#endif

#ifdef USE_FIXED_TIMESTEP_FPS
	start = true;
#endif

	font = new Font(8);
}

void Graphics::Quit() {
	std::map<unsigned long, Drawable*>::iterator it;
	std::map<unsigned long, Drawable*> drawable_map_temp = drawable_map;
	for (it = drawable_map_temp.begin(); it != drawable_map_temp.end(); it++) {
		delete it->second;
	}
	SDL_FreeSurface(blank_screen);
	delete font;
}

////////////////////////////////////////////////////////////
// Timer Wait
////////////////////////////////////////////////////////////
void Graphics::TimerWait(){
	last_ticks_wait = SDL_GetTicks();
}

////////////////////////////////////////////////////////////
// Timer Continue
////////////////////////////////////////////////////////////
void Graphics::TimerContinue() {
	last_ticks += SDL_GetTicks() - last_ticks_wait;
	next_ticks_fps += SDL_GetTicks() - last_ticks_wait;
}

////////////////////////////////////////////////////////////
// Update
////////////////////////////////////////////////////////////
#ifdef USE_FIXED_TIMESTEP_FPS
void Graphics::Update() {
	while (true) {
		if (start) {
			current_time = SDL_GetTicks() / 1000.0;
			update_iterations = (current_time - last_frame_time) + cycles_leftover;

			/*if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
				update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
			}*/
			start = false;
		}

		if (update_iterations > UPDATE_INTERVAL) {
			update_iterations -= UPDATE_INTERVAL;
			
			// Proccess game logic
			return;
		}

		start = true;
		cycles_leftover = update_iterations;
		last_frame_time = current_time;

		DrawFrame();
	}
}

#else

void Graphics::Update() {
	static unsigned long ticks;
	static unsigned long frames = 0;
	static double waitframes = 0;
	static double cyclesleftover;

	if (waitframes >= 1) {
		waitframes -= 1;
		return;
	}
	ticks = SDL_GetTicks();

																 // FIXME: This code reduces speed of zoomed windows a lot
	if ((ticks - last_ticks) >= (unsigned long)framerate_interval /*|| (framerate_interval - ticks + last_ticks) < 10*/) {
		cyclesleftover = waitframes;
		waitframes = (double)(ticks - last_ticks) / framerate_interval - cyclesleftover;

		last_ticks += (ticks - last_ticks) - (unsigned long)cyclesleftover;
		last_ticks = ticks;

		DrawFrame();

		++framecount;
		++frames;

		if (ticks >= next_ticks_fps) {
			next_ticks_fps += 1000;
			fps = frames;
			frames = 0;

			char title[255];
			sprintf(title, "%s - %d FPS", GAME_TITLE, fps);

			SDL_WM_SetCaption(title, NULL);
		}
	} else {
		SDL_Delay((long)(framerate_interval) - (ticks - last_ticks));
	}
}
#endif

void Graphics::DoTransition() {
	// Preparation is done
	prepare_transition = false;
	if (transition_current_frame <= transition_frames) {
		switch (actual_transition) {
			case FadeIn:
				SDL_FillRect(Player::main_surface, NULL, 0);
				SDL_SetAlpha(fake_screen, SDL_SRCALPHA, transition_current_frame*transition_increment);
				SDL_BlitSurface(fake_screen, NULL, Player::main_surface, NULL);
				////////
				break;

			case FadeOut:
				SDL_SetAlpha(blank_screen, SDL_SRCALPHA, transition_current_frame*transition_increment);
				SDL_BlitSurface(blank_screen, NULL, fake_screen, NULL);
				SDL_BlitSurface(fake_screen, NULL, Player::main_surface, NULL);

				break;

			default:
				break;
		}
		transition_current_frame++;
	} else {
		if (actual_transition == FadeOut) {
			// Little hack to skip drawing next frame
			// when changing scenes
			skip_draw = true;
		}
		if (frozen) {
			// Free fake_screen
			SDL_FreeSurface(fake_screen);
			fake_screen = NULL;
			// Unfreeze...
			frozen = false;
		}
		// Transition is over
		is_in_transition_yet = false;
	}
}

void Graphics::zoom2X(SDL_Surface* src, SDL_Surface* dst) {
	int h, w, t, t2, w2, m = 0, m2 = 0;

	if (SDL_MUSTLOCK(src))
		SDL_LockSurface(src);

	if (SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);

	Uint32* src_pixels = (Uint32*) src->pixels;
	Uint32* dst_pixels = (Uint32*) dst->pixels;

	Uint32 pixel;

	h = src->h;
	w = src->w;

	w2 = dst->w;

	t = (src->pitch / src->format->BytesPerPixel) - w;
	t2 = (dst->pitch / dst->format->BytesPerPixel) - w2;

	for (register int i = 0, i2 = 0; i < h; i++, i2 += 2) {
		for (register int j = 0, j2 = 0; j < w; j++, j2 += 2) {
			pixel = src_pixels[i*w+j+m];
			dst_pixels[i2*w2+j2+m2] = pixel;
			dst_pixels[i2*w2+j2+m2+1] = pixel;
			dst_pixels[(i2+1)*w2+j2+m2] = pixel;
			dst_pixels[(i2+1)*w2+j2+m2+1] = pixel;
		}
		m = t*i;
		m2 = t2*i2;
	}

	if (SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);

	if (SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
}

void Graphics::PrintFPS() {
	std::stringstream text;
	SDL_Color fg_color = { 255, 255, 255, 0 };
	SDL_Surface* text_surface;
	text << "FPS: " << fps;
	text_surface = TTF_RenderText_Solid(font->GetTTF(), text.str().c_str(), fg_color);
#ifdef GEKKO
	SDL_Rect rect = { 10, 10, text_surface->w, text_surface->h };
#else
	SDL_Rect rect = { 0, 0, text_surface->w, text_surface->h };
#endif
	SDL_BlitSurface(text_surface, NULL, Player::main_surface, &rect);
	SDL_FreeSurface(text_surface);
}

////////////////////////////////////////////////////////////
// Draw Frame
////////////////////////////////////////////////////////////
void Graphics::DrawFrame() {
	if ( (!frozen) && (!skip_draw) ) {
		if (zlist_needs_sorting) {
			zlist.sort(SortZObj);
			zlist_needs_sorting = false;
		}

		SDL_FillRect(Player::main_surface, &Player::main_surface->clip_rect, default_backcolor);
		DrawableType type;
		for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
			type = drawable_map[it_zlist->GetId()]->GetType();
			if (( (!is_in_transition_yet) || (type != WINDOW) )
				|| (!wait_for_transition)) // Make sure not to draw Windows until transition's finished
				drawable_map[it_zlist->GetId()]->Draw(it_zlist->GetZ());
		}
	}
	skip_draw = false;

	// If we are preparing for transition
	// we're done here
	if (prepare_transition) {
		return;
	}

	// Print FPS if needed
	if (fps_showing)
		PrintFPS();

	if (Player::zoom) {
		zoom2X(Player::main_surface, Player::main_window);
	}
	SDL_UpdateRect(Player::main_window, 0, 0, 0, 0);
}

////////////////////////////////////////////////////////////
// Freeze screen
////////////////////////////////////////////////////////////
void Graphics::Freeze() {
	// TODO
	// Make a copy of current screen
	fake_screen = SDL_DisplayFormat(Player::main_surface);
	// Screen is frozen now
	frozen = true;
}

////////////////////////////////////////////////////////////
// Transition
////////////////////////////////////////////////////////////
void Graphics::Transition(TransitionType type, int time, bool wait) {
	//////
	prepare_transition = true;
	skip_draw = false;
	DrawFrame();
	/////

	if (time > 255) time = 255;
	if (time == 0) time = 1;
	transition_frames = time;
	transition_increment = 255/time;
	transition_current_frame = 0;
	actual_transition = type;
	wait_for_transition = wait;

	if (!frozen) {
		// Get into actual transition
		is_in_transition_yet = true;
		
		// Freeze screen
		Freeze();
	}

	do {
		DoTransition();
		Update();
	} while (is_in_transition_yet);
}

void Graphics::SetDefaultBackcolor(const SDL_Color& color) {
	default_backcolor = SDL_MapRGB(Player::main_surface->format, color.r, color.g, color.b);
}

////////////////////////////////////////////////////////////
// Reset frames
////////////////////////////////////////////////////////////
void Graphics::FrameReset() {
	last_ticks = SDL_GetTicks();
}

////////////////////////////////////////////////////////////
// Wait frames
////////////////////////////////////////////////////////////
void Graphics::Wait(int duration) {
	for (int i = duration; i > 0; i--) {
		Update();
	}
}

////////////////////////////////////////////////////////////
// Snap screen to bitmap
////////////////////////////////////////////////////////////
Bitmap* Graphics::SnapToBitmap() {
	// TODO
	return new Bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
}

////////////////////////////////////////////////////////////
// Properties
////////////////////////////////////////////////////////////
int Graphics::GetFrameCount() {
	return framecount;
}
void Graphics::SetFrameCount(int nframecount) {
	framecount = nframecount;
}

///////////////////////////////////////////////////////////
// Register Drawable
///////////////////////////////////////////////////////////
void Graphics::RegisterDrawable(unsigned long ID, Drawable* drawable) {
	drawable_map[ID] = drawable;
}

///////////////////////////////////////////////////////////
// Remove Drawable
///////////////////////////////////////////////////////////
void Graphics::RemoveDrawable(unsigned long ID) {
	std::map<unsigned long, Drawable*>::iterator it = Graphics::drawable_map.find(ID);
	drawable_map.erase(it);
}

///////////////////////////////////////////////////////////
// Sort ZObj
///////////////////////////////////////////////////////////
bool Graphics::SortZObj(ZObj &first, ZObj &second) {
	if (first.GetZ() < second.GetZ()) return true;
	else if (first.GetZ() > second.GetZ()) return false;
	else return first.GetCreation() < second.GetCreation();
}

///////////////////////////////////////////////////////////
// Register ZObj
///////////////////////////////////////////////////////////
void Graphics::RegisterZObj(long z, unsigned long ID) {
	creation += 1;
	ZObj zobj(z, creation, ID);

	zlist.push_back(zobj);
	zlist_needs_sorting = true;
}
void Graphics::RegisterZObj(long z, unsigned long ID, bool multiz) {
	ZObj zobj(z, 999999, ID);
	zlist.push_back(zobj);
	zlist_needs_sorting = true;
}

///////////////////////////////////////////////////////////
// Remove ZObj
///////////////////////////////////////////////////////////
struct remove_zobj_id : public std::binary_function<ZObj, ZObj, bool> {
	remove_zobj_id(unsigned long val) : ID(val) {}
	bool operator () (ZObj &obj) const {return obj.GetId() == ID;}
	unsigned long ID;
};
void Graphics::RemoveZObj(unsigned long ID) {
	zlist.remove_if (remove_zobj_id(ID));
}

///////////////////////////////////////////////////////////
// Update ZObj Z
///////////////////////////////////////////////////////////
void Graphics::UpdateZObj(unsigned long ID, long z) {
	for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
		if (it_zlist->GetId() == ID) {
			it_zlist->SetZ(z);
			zlist_needs_sorting = true;
			break;
		}
	}
}
