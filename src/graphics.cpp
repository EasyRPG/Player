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
#include "sprig.h"
#include "output.h"
#include "player.h"
#include "drawable.h"
#include "SDL_ttf.h"
#include "SDL_rotozoom.h"

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

	if (TTF_Init() == -1) {
		Output::Error("Couldn't initialize SDL_ttf library.\n%s\n", TTF_GetError());
	}
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

////////////////////////////////////////////////////////////
// Draw Frame
////////////////////////////////////////////////////////////
void Graphics::DrawFrame() {
	SDL_FillRect(Player::main_window, &Player::main_window->clip_rect, DEFAULT_BACKCOLOR);

	for (it_zlist = zlist.begin(); it_zlist != zlist.end(); it_zlist++) {
		drawable_map[it_zlist->GetId()]->Draw(it_zlist->GetZ());
	}

	// 2x Zoom - Bit slow, but works
    // Update (11/6/10) SPG library gives a faster zoomer, not having to allocate surfaces each time ;)
	if (Player::zoom) {
        SDL_Surface* videoSurface = SDL_GetVideoSurface();
//        SDL_Rect r = SPG_TransformX(Player::main_window, videoSurface, 0.0, 2.0, 2.0, 160, 120, 320, 240, SPG_NONE);
		register int i, j;
		int bpp = Player::main_window->format->BytesPerPixel;
		bpp == videoSurface->format->BytesPerPixel;
		if (SDL_MUSTLOCK(videoSurface) != 0)
		{
			if (SDL_LockSurface(videoSurface) < 0)
			{
				fprintf(stderr, "screen locking failed\n");
				return;
			}
		}
		switch (bpp)
		{
		case 2:
		{
			int tpitch = videoSurface->pitch / 2;
			int spitch = Player::main_window->pitch / 2;
			/* :COMMENT: 051223.15: pitch is always in bytes
			* However, incrementing is done in sizeof(Uint16)
			* and sizeof(Uint16) is two bytes. */
			Uint16* tp = (Uint16*) videoSurface->pixels;
			Uint16* sp = (Uint16*) Player::main_window->pixels;
			const int wd = ((videoSurface->w / 2) < (Player::main_window->w))
				? (videoSurface->w / 2) : (Player::main_window->w);
			const int hg = ((videoSurface->h) < (Player::main_window->w))
				? (videoSurface->h) : (Player::main_window->w);
			for (j = 0; j < hg; ++j)
			{
				for (i = 0; i < wd; ++i)
				{
					tp[i*2] = sp[i];
					tp[i*2 + 1] = sp[i];
				}
				tp += tpitch;
				if (j % 2 != 0)  sp += spitch;
			}
			break;
		}
		case 3:
		{
			/* :COMMENT: 051223.18: This case has only been tested on
			*                      little-endian machine. */
			int tpitch = videoSurface->pitch;
			int spitch = Player::main_window->pitch;
			const int wd = ((videoSurface->w / 2) < (Player::main_window->w))
				? (videoSurface->w / 2) : (Player::main_window->w);
			const int hg = ((videoSurface->h) < (Player::main_window->w))
				? (videoSurface->h) : (Player::main_window->w);
			Uint8* tp = (Uint8*) videoSurface->pixels;
			Uint8* sp = (Uint8*) Player::main_window->pixels;
			for (j = 0; j < hg; ++j)
			{
				for (i = 0; i < 3 * wd; i += 3)
				{
					int i2 = i * 2;
					tp[i2 + 0] = sp[i];
					tp[i2 + 1] = sp[i + 1];
					tp[i2 + 2] = sp[i + 2];
					tp[i2 + 3] = sp[i];
					tp[i2 + 4] = sp[i + 1];
					tp[i2 + 5] = sp[i + 2];
				}
				tp += tpitch;
				if (j % 2 != 0)  sp += spitch;
			}
			break;
		}
		case 4:
		{
			int tpitch = videoSurface->pitch / 4;
			int spitch = Player::main_window->pitch / 4;
			/* :COMMENT: 051223.15: pitch is always in bytes
			* However, incrementing is done in sizeof(Uint32)
			* and sizeof(Uint32) is four bytes. */
			Uint32* tp = (Uint32*) videoSurface->pixels;
			Uint32* sp = (Uint32*) Player::main_window->pixels;
			const int wd = ((videoSurface->w / 2) < (Player::main_window->w))
				? (videoSurface->w / 2) : (Player::main_window->w);
			const int hg = ((videoSurface->h) < (Player::main_window->w))
				? (videoSurface->h) : (Player::main_window->w);
			for (j = 0; j < hg; ++j)
			{
				for (i = 0; i < wd; ++i)
				{
					tp[i*2] = sp[i];
					tp[i*2 + 1] = sp[i];
				}
				tp += tpitch;
				if (j % 2 != 0)  sp += spitch;
			}
			break;
		}
		default:
		/* :COMMENT: 051223.17: This should never happen. */
		fprintf(stderr, "Unknown bitdepth.\n");
		break;
    }
	if (SDL_MUSTLOCK(videoSurface) != 0)
	{
		SDL_UnlockSurface(videoSurface);
	}
	SDL_Flip(videoSurface);
	} else {
		if (SDL_Flip(Player::main_window) == -1) {
			Output::Error("Couldn't update screen.\n%s\n", SDL_GetError());
		}
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
void Graphics::Transition(int type, int time) {
	// TODO
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
