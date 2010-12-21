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
#include <algorithm>
#include <sstream>
#include <vector>
#include "graphics.h"
#include "cache.h"
#include "drawable.h"
#include "font_render_8x8.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "SDL_ttf.h"
#include "sdl_ui.h"
#include "system.h"
#include "util_macro.h"

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

	bool fps_showing;

	std::map<unsigned long, Drawable*> drawable_map;
	std::map<unsigned long, Drawable*>::iterator it_drawable_map;
	std::list<ZObj*> zlist;
	std::list<ZObj*>::iterator it_zlist;
}

namespace {
	bool zlist_needs_sorting;
	bool prepare_transition;
	bool skip_draw;
	int transition_frames;
	int transition_current_frame;
	int transition_increment;
	int increment_left, increment_left_acc;
	int frames_left;
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

namespace {
	int gcd(int a, int b) {
		if (a==0) return b;
		return gcd(b%a,a);
	}
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
	blank_screen = SDL_DisplayFormat(DisplaySdlUi->GetDisplaySurface());
	SDL_FillRect(blank_screen, NULL, 0);

	is_in_transition_yet = false;
	wait_for_transition = false;
	prepare_transition = false;
	frozen = false;
	skip_draw = false;

	if (TTF_Init() == -1) {
		Output::Error("Couldn't initialize SDL_ttf library.\n%s\n", TTF_GetError());
	}

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
	for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
		delete *it_zlist;
	}
	SDL_FreeSurface(blank_screen);
	Cache::Clear();
	delete font;
	TTF_Quit();
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
	
	for (;;) {
		ticks = SDL_GetTicks();

																 // FIXME: This code reduces speed of zoomed windows a lot
		if ((ticks - last_ticks) >= (unsigned long)framerate_interval /*|| (framerate_interval - ticks + last_ticks) < 10*/) {
			cyclesleftover = waitframes;
			waitframes = (double)(ticks - last_ticks) / framerate_interval - cyclesleftover;

			//last_ticks += (ticks - last_ticks) - (unsigned long)cyclesleftover;
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

			break;

		} else {
			SDL_Delay((long)(framerate_interval) - (ticks - last_ticks));
		}
	}
}
#endif

void Graphics::DoTransition() {
	// Preparation is done
	prepare_transition = false;
	if (transition_current_frame < transition_frames) {

		uint8 inc;
		if ( ++transition_current_frame % frames_left == 0 ) {
			increment_left_acc += increment_left;
		}
		inc = (uint8)(transition_current_frame * transition_increment+increment_left_acc);

		switch (actual_transition) {
			case FadeIn:
				SDL_FillRect(DisplaySdlUi->GetDisplaySurface(), NULL, 0);
				SDL_SetAlpha(fake_screen, SDL_SRCALPHA, inc);
				SDL_BlitSurface(fake_screen, NULL, DisplaySdlUi->GetDisplaySurface(), NULL);
				////////
				break;

			case FadeOut:
				SDL_SetAlpha(blank_screen, SDL_SRCALPHA, inc);
				SDL_BlitSurface(fake_screen, NULL, DisplaySdlUi->GetDisplaySurface(), NULL);
				SDL_BlitSurface(blank_screen, NULL, DisplaySdlUi->GetDisplaySurface(), NULL);
				break;

			default:
				break;
		}
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

void Graphics::PrintFPS() {
	std::stringstream text;
	SDL_Color fg_color = { 255, 255, 255, 0 };
	SDL_Surface* text_surface;
	text << "FPS: " << fps;
	SDL_Rect dst_pos = { 10, 10, 0, 0 };
	text_surface = TTF_RenderText_Solid(font->GetTTF(), text.str().c_str(), fg_color);
	SDL_BlitSurface(text_surface, NULL, DisplaySdlUi->GetDisplaySurface(), &dst_pos);
	SDL_FreeSurface(text_surface);
}

////////////////////////////////////////////////////////////
// Draw Frame
////////////////////////////////////////////////////////////
void Graphics::DrawFrame() {
	if ( !frozen && !skip_draw ) {
		if (zlist_needs_sorting) {
			zlist.sort(SortZObj);
			zlist_needs_sorting = false;
		}

		DisplayUi->CleanDisplay();

		DrawableType type;
		for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
			type = drawable_map[(*it_zlist)->GetId()]->GetType();
			if (( (!is_in_transition_yet) || (type != WINDOW) )
				|| (!wait_for_transition)) // Make sure not to draw Windows until transition's finished
				drawable_map[(*it_zlist)->GetId()]->Draw((*it_zlist)->GetZ());
		}
	} else {
		skip_draw = false;
	}

	// If we are preparing for transition
	// we're done here
	if (prepare_transition) {
		return;
	}

	// Print FPS if needed
	if (fps_showing)
		PrintFPS();

	DisplayUi->UpdateDisplay();
}

////////////////////////////////////////////////////////////
// Freeze screen
////////////////////////////////////////////////////////////
void Graphics::Freeze() {
	// TODO
	// Make a copy of current screen
	fake_screen = SDL_DisplayFormat(DisplaySdlUi->GetDisplaySurface());
	// Screen is frozen now
	frozen = true;
}

////////////////////////////////////////////////////////////
// Transition
////////////////////////////////////////////////////////////
void Graphics::Transition(TransitionType type, int time, bool wait) {
	return;

	//////
	prepare_transition = true;
	skip_draw = false;
	DrawFrame();
	/////

	if (time <= 0) time = 1;
	transition_frames = time;
	transition_increment = 255 / time;
	transition_current_frame = 0;
	increment_left_acc = 0;
	actual_transition = type;
	wait_for_transition = wait;

	int div = gcd(255%time, time);

	increment_left = (255%time) / div;
	frames_left = time / div;

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
	return Bitmap::CreateBitmap(DisplayUi->GetWidth(), DisplayUi->GetHeight());
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
inline bool Graphics::SortZObj(const ZObj* first, const ZObj* second) {
	if (first->GetZ() < second->GetZ()) return true;
	else if (first->GetZ() > second->GetZ()) return false;
	else return first->GetCreation() < second->GetCreation();
}

///////////////////////////////////////////////////////////
// Register ZObj
///////////////////////////////////////////////////////////
ZObj* Graphics::RegisterZObj(long z, unsigned long ID) {
	creation += 1;
	ZObj* zobj = new ZObj(z, creation, ID);

	zlist.push_back(zobj);
	zlist_needs_sorting = true;

	return zobj;
}
void Graphics::RegisterZObj(long z, unsigned long ID, bool multiz) {
	ZObj* zobj = new ZObj(z, 999999, ID);
	zlist.push_back(zobj);
	zlist_needs_sorting = true;
}

///////////////////////////////////////////////////////////
// Remove ZObj
///////////////////////////////////////////////////////////
void Graphics::RemoveZObj(unsigned long ID) {
	RemoveZObj(ID, false);
}
void Graphics::RemoveZObj(unsigned long ID, bool multiz) {
	std::vector<std::list<ZObj*>::iterator> to_erase;
	size_t i = 0;
	for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
		if ((*it_zlist)->GetId() == ID) {
			delete *it_zlist;
			to_erase.push_back(it_zlist);
			if (!multiz) break;
		}

	}
	for (i = 0; i < to_erase.size(); i++) {
		zlist.erase(to_erase[i]);
	}
}

///////////////////////////////////////////////////////////
// Update ZObj Z
///////////////////////////////////////////////////////////
void Graphics::UpdateZObj(ZObj* zobj, long z) {
	zobj->SetZ(z);
	zlist_needs_sorting = true;
}
