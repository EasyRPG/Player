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
#include "options.h"
#include "output.h"
#include "player.h"
#include "drawable.h"
#include "SDL_ttf.h"
#include "SDL_rotozoom.h"

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Graphics {
	int fps;
	int framerate;
	int framecount;
	double framerate_interval;
	unsigned long ID;
	unsigned long last_ticks;
	unsigned long last_ticks_wait;
	unsigned long next_ticks_fps;

	std::list<Drawable*> drawable_list;
	std::list<Drawable*>::iterator it_drawable_list;
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void Graphics::Init() {
	fps = 0;
	framerate = DEFAULT_FPS;
	framecount = 0;
	ID = 0;
	framerate_interval = 1000.0 / DEFAULT_FPS;
	last_ticks = SDL_GetTicks() + (long)framerate_interval;
	next_ticks_fps = last_ticks + 1000;

	if (TTF_Init() == -1) {
		Output::Error("Couldn't initialize SDL_ttf library.\n%s\n", TTF_GetError());
	}
}

////////////////////////////////////////////////////////////
/// Timer Wait
////////////////////////////////////////////////////////////
void Graphics::TimerWait(){
	last_ticks_wait = SDL_GetTicks();
}

////////////////////////////////////////////////////////////
/// Timer Continue
////////////////////////////////////////////////////////////
void Graphics::TimerContinue() {
	last_ticks += SDL_GetTicks() - last_ticks_wait;
	next_ticks_fps += SDL_GetTicks() - last_ticks_wait;
}

////////////////////////////////////////////////////////////
/// Update
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

////////////////////////////////////////////////////////////
/// Draw Frame
////////////////////////////////////////////////////////////
void Graphics::DrawFrame() {
	SDL_FillRect(Player::main_window, &Player::main_window->clip_rect, DEFAULT_BACKCOLOR);

	for (it_drawable_list = drawable_list.begin(); it_drawable_list != drawable_list.end(); ++it_drawable_list) {
		(*it_drawable_list)->Draw();
	}

	// 2x Zoom - Bit slow, but works
	if (Player::zoom) {
		SDL_Surface* videoSurface = SDL_GetVideoSurface();
		SDL_Surface* zoomed_window = rotozoomSurface(Player::main_window, 0, 2, 0);
		SDL_BlitSurface(zoomed_window, NULL, videoSurface, NULL);
		SDL_FreeSurface(zoomed_window);
		SDL_Flip(videoSurface);
	} else {
		if (SDL_Flip(Player::main_window) == -1) {
			Output::Error("Couldn't update screen.\n%s\n", SDL_GetError());
		}
	}
}

////////////////////////////////////////////////////////////
/// Freeze screen
////////////////////////////////////////////////////////////
void Graphics::Freeze() {
	// TODO
}

////////////////////////////////////////////////////////////
/// Transition
////////////////////////////////////////////////////////////
void Graphics::Transition(int type, int time) {
	// TODO
}

////////////////////////////////////////////////////////////
/// Reset frames
////////////////////////////////////////////////////////////
void Graphics::FrameReset() {
	last_ticks = SDL_GetTicks();
}

////////////////////////////////////////////////////////////
/// Wait frames
////////////////////////////////////////////////////////////
void Graphics::Wait(int duration) {
	for (int i = duration; i > 0; i--) {
		Update();
	}
}

////////////////////////////////////////////////////////////
/// Snap screen to bitmap
////////////////////////////////////////////////////////////
Bitmap* Graphics::SnapToBitmap() {
	// TODO
	return new Bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
int Graphics::GetFrameCount() {
	return framecount;
}
void Graphics::SetFrameCount(int nframecount) {
	framecount = nframecount;
}

////////////////////////////////////////////////////////////
/// Sort Drawable
////////////////////////////////////////////////////////////
bool Graphics::SortDrawable(Drawable* &first, Drawable* &second) {
	if (first->GetZ() < second->GetZ()) return true;
	else if (first->GetZ() > second->GetZ()) return false;
	return first->GetId() < second->GetId();
}

////////////////////////////////////////////////////////////
/// Remove Drawable
////////////////////////////////////////////////////////////
struct remove_drawable_id : public std::binary_function<Drawable*, Drawable*, bool> {
	remove_drawable_id(unsigned long val) : ID(val) {}
	bool operator () (Drawable* &obj) const {return obj->GetId() == ID;}
	unsigned long ID;
};
void Graphics::RemoveDrawable(unsigned long ID) {
	drawable_list.remove_if(remove_drawable_id(ID));
}
