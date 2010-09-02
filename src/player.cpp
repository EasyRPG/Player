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

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Player {
	SDL_Surface* main_window;
	bool focus;
	//bool alt_pressing;
	bool fullscreen;
	bool zoom;
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void Player::Init() {
	Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
#ifdef DINGOO
	Uint32 videoFlags = SDL_SWSURFACE;
#else
	Uint32 videoFlags = SDL_HWSURFACE;
#endif
#ifdef DEBUG
	flags |= SDL_INIT_NOPARACHUTE;
#endif
	if ((SDL_Init(flags) < 0)) { 
		Output::Error("EasyRPG Player couldn't initialize SDL.\n%s\n", SDL_GetError());
	}
	atexit(SDL_Quit);

	main_window = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BPP, videoFlags);
	if (!main_window) {
		Output::Error("EasyRPG Player couldn't initialize %dx%dx%d video mode.\n%s\n", SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);
	
	SDL_WM_SetCaption(GAME_TITLE, NULL);

	focus = true;
	//alt_pressing = false;
	fullscreen = false;
	zoom = false;
}

////////////////////////////////////////////////////////////
/// Run
////////////////////////////////////////////////////////////
void Player::Run() {

	// Create Scene Title
	Main_Data::scene = new Scene_Title();

	// Main loop
	while (Main_Data::scene_type != SCENE_NULL) {
		Main_Data::scene->MainFunction();
	}
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Player::Update() {
	SDL_Event evnt;

	while (true) {
		int result = SDL_PollEvent(&evnt);
		if (evnt.type == SDL_QUIT) {
			Exit();
			exit(0);
		} else if (evnt.type == SDL_KEYDOWN) {
			/*if (evnt.key.keysym == SDLK_LALT || evnt.key.keysym == SDLK_RALT) {
				alt_pressing = true;
			} else if (evnt.key.keysym == SDLK_RETURN && alt_pressing) {
				ToggleFullscreen();
			}*/
			switch (evnt.key.keysym.sym) {
			case SDLK_F4:
				ToggleFullscreen();
				break;
			case SDLK_F5:
				ToggleZoom();
				break;
			case SDLK_F12:
				Main_Data::scene = new Scene_Title();
				Cache::Clear();
				break;
			default:
				break;
			}
		} /*else if (evnt.type == SDL_KEYUP) {
			if (evnt.key.keysym == SDLK_LALT || evnt.key.keysym == SDLK_RALT) {
				alt_pressing = false;
			}
		}*/	else if (PAUSE_GAME_WHEN_FOCUS_LOST && evnt.type == SDL_ACTIVEEVENT) {
			if (evnt.active.type == SDL_APPACTIVE) {
				if (evnt.active.gain && !focus) {
					focus = true;
					Graphics::TimerContinue();
					if (PAUSE_AUDIO_WHEN_FOCUS_LOST) {
						//Audio::Continue();
					}
				} else if (!evnt.active.gain && focus) {
					focus = false;
					Input::ClearKeys();
					Graphics::TimerWait();
					if (PAUSE_AUDIO_WHEN_FOCUS_LOST) {
						//Audio::Pause();
					}
				}
			}
		}

		if (!result && !(PAUSE_GAME_WHEN_FOCUS_LOST && !focus)) {
			break;
		}
	}
}

////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////
void Player::Exit() {
	Main_Data::Cleanup();
	SDL_Quit();
}

////////////////////////////////////////////////////////////
/// Switch fullscreen
////////////////////////////////////////////////////////////
void Player::ToggleFullscreen() {
	#ifdef DINGOO
		fullscreen = false;
		return;
	#endif

	if (zoom) {
		ToggleZoom();
	}
	Uint32 flags = main_window->flags;
	SDL_FreeSurface(main_window);
	main_window = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, flags ^ SDL_FULLSCREEN);
	if (main_window == NULL) {
		main_window = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, flags);
	} else {
		fullscreen = !fullscreen;
	}
}

////////////////////////////////////////////////////////////
/// Switch zoom
////////////////////////////////////////////////////////////
void Player::ToggleZoom() {
	#ifdef DINGOO
		zoom = false;
		return;
	#endif
	if (fullscreen) {
		ToggleFullscreen();
	}
	Uint32 flags = main_window->flags;

	if (!zoom) {
		main_window = SDL_ConvertSurface(main_window, main_window->format, main_window->flags);
		SDL_SetVideoMode(SCREEN_WIDTH*2, SCREEN_HEIGHT*2, 32, flags);
	} else {
		SDL_FreeSurface(main_window);
		main_window = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, flags);
	}

	zoom = !zoom;
}

////////////////////////////////////////////////////////////
/// Get window width
////////////////////////////////////////////////////////////
int Player::GetWidth() {
	return main_window->w;
}

////////////////////////////////////////////////////////////
/// Get window height
////////////////////////////////////////////////////////////
int Player::GetHeight() {
	return main_window->h;
}

