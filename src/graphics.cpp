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

	int transition_frames;
	int transition_current_frame;
	int transition_increment;
	bool is_in_transition_yet;
	bool wait_for_transition;
	TransitionType actual_transition;
	SDL_Surface* fake_background;
	SDL_Surface* blank_screen;

	std::map<unsigned long, Drawable*> drawable_map;
	std::map<unsigned long, Drawable*>::iterator it_drawable_map;
	std::list<ZObj> zlist;
	std::list<ZObj>::iterator it_zlist;
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

	transition_frames = 0;
	transition_current_frame = 0;
	transition_increment = 0;
	actual_transition = NoTransition;
	fake_background = NULL;
	blank_screen = SDL_ConvertSurface(Player::main_window, Player::main_window->format, Player::main_window->flags);
	SDL_FillRect(blank_screen, NULL, 0);

	is_in_transition_yet = false;
	wait_for_transition = false;

	if (TTF_Init() == -1) {
		Output::Error("Couldn't initialize SDL_ttf library.\n%s\n", TTF_GetError());
	}
}

void Graphics::Quit() {
	std::map<unsigned long, Drawable*>::iterator i;
	for (i = drawable_map.begin(); i != drawable_map.end(); i++) {
		delete i->second;
	}
	SDL_FreeSurface(blank_screen);
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

void Graphics::DoTransition() {

	if (transition_current_frame <= transition_frames) {
		switch (actual_transition) {
			case FadeIn:
				// Pretty damn slow . May need to be optimised.
				fake_background = SDL_ConvertSurface(Player::main_window, Player::main_window->format, Player::main_window->flags);
				SDL_FillRect(Player::main_window, NULL, 0);
				SDL_SetAlpha(fake_background, SDL_SRCALPHA, transition_current_frame*transition_increment);
				SDL_BlitSurface(fake_background, NULL, Player::main_window, NULL);
				SDL_FreeSurface(fake_background);
				////////
				break;

			case FadeOut:
				SDL_SetAlpha(blank_screen, SDL_SRCALPHA, Graphics::transition_current_frame*transition_increment);
				SDL_BlitSurface(blank_screen, NULL, Player::main_window, NULL);
				break;

			default:
				break;
		}
		transition_current_frame++;
	} else {
		if (actual_transition == FadeOut)
			SDL_BlitSurface(blank_screen, NULL, Player::main_window, NULL);
		is_in_transition_yet = false;
	}
}

////////////////////////////////////////////////////////////
// Draw Frame
////////////////////////////////////////////////////////////
void Graphics::DrawFrame() {

	SDL_FillRect(Player::main_window, &Player::main_window->clip_rect, DEFAULT_BACKCOLOR);
	DrawableType type;
	for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
		type = drawable_map[it_zlist->GetId()]->GetType();
		if (( (!is_in_transition_yet) || (type != WINDOW) ) 
			|| (!wait_for_transition))   // Make sure not to draw Windows until transition's finished
			drawable_map[it_zlist->GetId()]->Draw(it_zlist->GetZ());
	}

	if (is_in_transition_yet) DoTransition();

	if (Player::zoom) {
		// TODO: Resize zoom code for BPP != 4 (and  maybe zoom != x2)
		SDL_Surface* surface = Player::main_window;
		register int i, j;
		SDL_LockSurface(surface);
		int w = Player::GetWidth();
		int zoom_w = surface->w;
		int zoom_h = surface->h;
		int pitch = surface->pitch / 4;
		Uint32* src = (Uint32*) surface->pixels + pitch * Player::GetHeight();
		Uint32* dst = (Uint32*) surface->pixels + pitch * (zoom_h - 1);
		for (j = zoom_h - 1; j >= 0; j--) {
			for (i = w - 1; i >= 0 ; i--) {
				dst[i * 2] = src[i];
				dst[i * 2 + 1] = src[i];
			}
			dst -= pitch;
			if (j % 2 != 0)  src -= pitch;
		}
		SDL_UnlockSurface(surface);
		SDL_Flip(surface);
	} else {
		SDL_Flip(Player::main_window);
	}
}

////////////////////////////////////////////////////////////
// Freeze screen
////////////////////////////////////////////////////////////
void Graphics::Freeze() {
	// TODO
}

////////////////////////////////////////////////////////////
// Transition
////////////////////////////////////////////////////////////
void Graphics::Transition(TransitionType type, int time, bool wait) {
	if (time > 255) time = 255;
	if (time == 0) time = 1;
	transition_frames = time;
	transition_increment = 255/time;
	transition_current_frame = 0;
	is_in_transition_yet = true;
	actual_transition = type;
	wait_for_transition = wait;
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
	zlist.sort(SortZObj);
}
void Graphics::RegisterZObj(long z, unsigned long ID, bool multiz) {
	ZObj zobj(z, 999999, ID);
	zlist.push_back(zobj);
	zlist.sort(SortZObj);
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
			break;
		}
	}
	zlist.sort(SortZObj);
}
