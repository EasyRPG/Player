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

	for (;;) {
		int result = SDL_PollEvent(&evnt);

#ifndef PAUSE_GAME_WHEN_FOCUS_LOST
		if (!result) {
#else
		if (!result && focus) {
#endif
			break;
		}

		if (evnt.type == SDL_QUIT) {
			Exit();
			exit(0);
		} else if (evnt.type == SDL_KEYDOWN) {
			switch (evnt.key.keysym.sym) {
			case SDLK_F4:
#ifdef _WIN32
				// Close Program on LeftAlt+F4
				if (evnt.key.keysym.mod == KMOD_LALT) {
					Exit();
					exit(0);
				}
#endif
				StartVideoModeChange();
				ToggleFullscreen();
				EndVideoModeChange();
				break;
			case SDLK_F5:
				StartVideoModeChange();
				ToggleZoom();
				EndVideoModeChange();
				break;
			case SDLK_F12:
				Main_Data::scene = new Scene_Title();
				Cache::Clear();
				break;
			case SDLK_RETURN:
				// Fullscreen on Alt+Enter
				if (evnt.key.keysym.mod == KMOD_LALT ||
					(evnt.key.keysym.mod | KMOD_RALT)) {
					StartVideoModeChange();
					ToggleFullscreen();
					EndVideoModeChange();
				}
			default:
				break;
			}
		}
#ifdef PAUSE_GAME_WHEN_FOCUS_LOST
		else if (evnt.type == SDL_ACTIVEEVENT) {
			if (evnt.active.type == SDL_APPACTIVE) {
				if (evnt.active.gain && !focus) {
					focus = true;
					Graphics::TimerContinue();
#ifdef PAUSE_AUDIO_WHEN_FOCUS_LOST
					//Audio::Continue();
#endif
				} else if (!evnt.active.gain && focus) {
					focus = false;
					Input::ClearKeys();
					Graphics::TimerWait();
#ifdef PAUSE_AUDIO_WHEN_FOCUS_LOST
					//Audio::Pause();
#endif
				}
			}
		}
#endif
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
	#else
		fullscreen = !fullscreen;
	#endif
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
		video_width *= 2;
		video_height *= 2;
	}

	main_window  = SDL_SetVideoMode(video_width, video_height, BPP, flags);
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
