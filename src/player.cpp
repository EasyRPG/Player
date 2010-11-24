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
#include "player.h"
#include "options.h"
#include "output.h"
#include "audio.h"
#include "graphics.h"
#include "input.h"
#include "cache.h"
#include "main_data.h"
#include "scene_title.h"

#ifdef GEKKO
	#include <gccore.h>
#endif

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Player {
	SDL_Surface* main_window;
	// Everything will be blit here
	SDL_Surface* main_surface;
	bool focus;
	bool alt_pressing;
	bool fullscreen;
	bool zoom;
	int width;
	int height;

	bool last_fullscreen;
	bool last_zoom;
	int last_width;
	int last_height;
}

namespace {
		int IgnoreNonFocusEvents(const SDL_Event* e);
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void Player::Init() {
	focus = true;
	alt_pressing = false;
	zoom = true;
	fullscreen = false;
	width = SCREEN_WIDTH;
	height = SCREEN_HEIGHT;

	Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
#ifdef DEBUG
	flags |= SDL_INIT_NOPARACHUTE;
#endif

#ifdef GEKKO
	// Initialize the video system
	VIDEO_Init();
	GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
	void* xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	zoom = false;
#endif

	if ((SDL_Init(flags) < 0)) {
		Output::Error("EasyRPG Player couldn't initialize SDL.\n%s\n", SDL_GetError());
	}

	RefreshVideoMode();

	SDL_ShowCursor(SDL_DISABLE);
	
	SDL_WM_SetCaption(GAME_TITLE, NULL);
}

////////////////////////////////////////////////////////////
/// Run
////////////////////////////////////////////////////////////
void Player::Run() {
	Scene::instance = new Scene_Title();
	
	// Main loop
	while (Scene::type != Scene::Null) {
		delete Scene::old_instance;
		Scene::instance->MainFunction();		
	}

	Player::Exit();
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Player::Update() {
	SDL_Event evnt;

	for (;;) {
		int result = SDL_PollEvent(&evnt);

#if PAUSE_GAME_WHEN_FOCUS_LOST == 0
		if (!result) {
#else
		if (!result && focus) {
#endif
			break;
		}

		switch (evnt.type) {
			case SDL_QUIT:
				Exit();
				exit(EXIT_SUCCESS);
				break;
			case SDL_KEYDOWN:
				switch (evnt.key.keysym.sym) {
					case SDLK_F4:
						// AltGr+F4 does nothing
						if (!(evnt.key.keysym.mod & KMOD_RALT)) {
							#ifdef _WIN32
							// Close Program on LeftAlt+F4
							if (evnt.key.keysym.mod & KMOD_LALT) {
								Exit();
								exit(EXIT_SUCCESS);
							}
							#endif
							// Otherwise F4 toogles fullscreen
							StartVideoModeChange();
							ToggleFullscreen();
							EndVideoModeChange();
						}
						break;
					case SDLK_F5:
						// F5 for Zoom
						StartVideoModeChange();
						ToggleZoom();
						EndVideoModeChange();
						break;
					case SDLK_F12:
						// FIXME: There is a huge memory leak here because
						// the new button in the title scene allocates lots of new
						// objects without freeing the old ones
						Scene::instance = new Scene_Title();
						break;
					case SDLK_RETURN:
					case SDLK_KP_ENTER:
						// Fullscreen on Alt+Enter
						if (evnt.key.keysym.mod & KMOD_LALT ||
							(evnt.key.keysym.mod & KMOD_RALT)) {
							StartVideoModeChange();
							ToggleFullscreen();
							EndVideoModeChange();
						}
						break;
					default:
						break;
				}
				break;

#if PAUSE_GAME_WHEN_FOCUS_LOST != 0
			case SDL_ACTIVEEVENT:
					if (evnt.active.state == SDL_APPINPUTFOCUS) {
						if ( (evnt.active.state & SDL_APPACTIVE) == 0 //&& !focus
							) {
					//focus = true;
					//Graphics::TimerContinue();
								SDL_SetEventFilter(&IgnoreNonFocusEvents);
								SDL_WaitEvent(NULL);
								SDL_SetEventFilter(NULL);
//#ifdef PAUSE_AUDIO_WHEN_FOCUS_LOST
					//Audio::Continue();
//#endif
						} /*else if (!evnt.active.gain && focus) {
					focus = false;
					Input::ClearKeys();
					Graphics::TimerWait();*/
//#ifdef PAUSE_AUDIO_WHEN_FOCUS_LOST
					//Audio::Pause();
//#endif
					}
					break;
#endif
			default:
				;
		} // Switch 1
	} // for
}

///////////////////////////////////////////////////
namespace {
	int IgnoreNonFocusEvents(const SDL_Event* e) {
		switch (e->type) {
			case SDL_ACTIVEEVENT:
				if (e->active.state & SDL_APPINPUTFOCUS) {
					return 1;
				} else {
					return 0;
				}
				break;
			default:
				;
		}
		return 0;
	}
}
////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////
void Player::Exit() {
	SDL_FreeSurface(main_surface);
	Main_Data::Cleanup();
	Graphics::Quit();
	Audio::Quit();
	SDL_Quit();
}

////////////////////////////////////////////////////////////
/// Switch fullscreen
////////////////////////////////////////////////////////////
void Player::ToggleFullscreen() {
	#ifdef DINGOO
		fullscreen = false;
	#else
		fullscreen = !fullscreen;
	#endif
		Graphics::fps_showing = !Graphics::fps_showing;
}

////////////////////////////////////////////////////////////
/// Switch zoom
////////////////////////////////////////////////////////////
void Player::ToggleZoom() {
	#ifdef DINGOO
		zoom = false;
	#else
		zoom = !zoom;
	#endif
}

////////////////////////////////////////////////////////////
/// Switch zoom
////////////////////////////////////////////////////////////
void Player::SetScreenSize(int width, int height) {
	Player::width = width;
	Player::height = height;
}

////////////////////////////////////////////////////////////
/// StartVideoModeChange
////////////////////////////////////////////////////////////
void Player::StartVideoModeChange() {
	last_fullscreen = fullscreen;
	last_zoom = zoom;
	last_width = width;
	last_height = height;
}

////////////////////////////////////////////////////////////
/// EndVideoModeChange
////////////////////////////////////////////////////////////
void Player::EndVideoModeChange() {
	if (last_fullscreen != fullscreen || last_zoom != zoom ||
		last_width != width || last_height != height) {

		if (!RefreshVideoMode()) {
			fullscreen = last_fullscreen;
			zoom = last_zoom;
			width = last_width;
			height = last_height;
			if (!RefreshVideoMode()) {
				Output::Error("Couldn't set video mode.\n%s\n", SDL_GetError());
			}
		}
	}
}

////////////////////////////////////////////////////////////
/// Refresh Video Mode
////////////////////////////////////////////////////////////
bool Player::RefreshVideoMode() {
#ifdef DINGOO
	Uint32 flags = SDL_SWSURFACE;
#else
	Uint32 flags = SDL_HWSURFACE;
#endif
	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	int video_width = width;
	int video_height = height;

	if (zoom) {
		main_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, video_width, video_height, BPP, 0, 0, 0, 0);
		video_width *= 2;
		video_height *= 2;
	}

	main_window = SDL_SetVideoMode(video_width, video_height, BPP, flags);

	if (!zoom) {
		// Free resources 
		SDL_FreeSurface(main_surface);

		// Write directly to main_window
		main_surface = main_window;
	}

	return main_window != NULL;
}

////////////////////////////////////////////////////////////
/// Get window width
////////////////////////////////////////////////////////////
int Player::GetWidth() {
	return width;
}

////////////////////////////////////////////////////////////
/// Get window height
////////////////////////////////////////////////////////////
int Player::GetHeight() {
	return height;
}
