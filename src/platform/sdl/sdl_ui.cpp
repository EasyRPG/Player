/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <cstring>

#include "system.h"

#include "sdl_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"

#ifdef GEKKO
#  include "platform/wii/main.h"
#endif

#ifdef SUPPORT_AUDIO
#  include "audio.h"

AudioInterface& SdlUi::GetAudio() {
	return *audio_;
}
#endif

static int FilterUntilFocus(const SDL_Event* evnt);

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
	static Input::Keys::InputKey SdlKey2InputKey(SDLKey sdlkey);
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	static Input::Keys::InputKey SdlJKey2InputKey(int button_index);
#endif

SdlUi::SdlUi(long width, long height, const Game_ConfigVideo& cfg) : BaseUi(cfg)
{
	auto fs_flag = cfg.fullscreen.Get();
	uint32_t flags = SDL_INIT_VIDEO;

#ifndef NDEBUG
	flags |= SDL_INIT_NOPARACHUTE;
#endif

	// Set some SDL environment variables before starting. These are platform
	// dependent, so every port needs to set them manually
#ifndef GEKKO
	// Set window position to the middle of the screen
	putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=center"));
#endif

	if (SDL_Init(flags) < 0) {
		Output::Error("Couldn't initialize SDL.\n{}\n", SDL_GetError());
	}

	sdl_surface = NULL;

	BeginDisplayModeChange();
		if (!RequestVideoMode(width, height, fs_flag)) {
			Output::Error("No suitable video resolution found. Aborting.");
		}
	EndDisplayModeChange();

#ifdef GEKKO
	// Eliminate debug spew in on-screen console
	Wii::SetConsole();
#endif

	SetTitle(GAME_TITLE);

#if (defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)) || (defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS))
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		Output::Warning("Couldn't initialize joystick. {}", SDL_GetError());
	}

	SDL_JoystickEventState(1);
	SDL_JoystickOpen(0);
#endif

	// No mouse support
	ShowCursor(false);

#ifdef SUPPORT_AUDIO
	if (!Player::no_audio_flag) {
		audio_.reset(new SdlAudio());
		return;
	}
#else
	audio_.reset(new EmptyAudio());
#endif
}

SdlUi::~SdlUi() {
	SDL_Quit();
}

bool SdlUi::RequestVideoMode(int width, int height, bool fullscreen) {
	// FIXME: Split method into submethods, really, this method isn't nice.
	// Note to Zhek, don't delete this fixme again.
	const SDL_VideoInfo *vinfo;
	SDL_Rect **modes;
	uint32_t flags = SDL_SWSURFACE;

	vinfo = SDL_GetVideoInfo();

	current_display_mode.height = height;
	current_display_mode.width = width;

#ifdef OPENDINGUX
	// Only one video mode for opendingux (320x240)
	// SDL_ListModes is broken, we must return here
	return true;
#endif

	if (vinfo->wm_available) {
		toggle_fs_available = true;
		// FIXME: this for may work, but is really confusing. Calling a method
		// that does this with the desired flags would be nicer.
		if (fullscreen) {
			flags |= SDL_FULLSCREEN;
		}
		for (;;) {
			modes = SDL_ListModes(NULL, flags);
			if (modes != NULL) {
				// Set up...
				current_display_mode.flags = flags;

				// All modes available
				if (modes == (SDL_Rect **)-1) {
					// If we have a high res, turn zoom on
					if (vinfo->current_h > height*2 && vinfo->current_w > width*2)
						current_display_mode.zoom = 2;
#if defined(SUPPORT_ZOOM)
					zoom_available = current_display_mode.zoom == 2;
#else
					zoom_available = false;
#endif
					return true;
				} else {
					int len = 0;
					while (modes[len])
						++len;

					for (int i = len-1; i >= 0; --i) {
						if (
							(modes[i]->h == height && modes[i]->w == width)
#if defined(SUPPORT_ZOOM)
							|| (modes[i]->h == height*2 && modes[i]->w == width*2)
#endif
						) {
							current_display_mode.zoom = modes[i]->w / width;
							zoom_available = current_display_mode.zoom == 2;
							return true;
						}
					}
				}
			}
			// No modes available
			if ((flags & SDL_FULLSCREEN) == SDL_FULLSCREEN) {
				// Try without fullscreen
				flags &= ~SDL_FULLSCREEN;
			} else {
				// No mode available :(
				return false;
			}
		}
	} // wm_available

	if (!fullscreen) {
		// Stop here since we need a window manager for non fullscreen modes
		return false;
	}

	// No hard accel and no window manager
	flags = SDL_SWSURFACE | SDL_FULLSCREEN;

	modes = SDL_ListModes(NULL, flags);
	if (modes == NULL) {
		// No video for you
		return false;
	}

	if (modes == (SDL_Rect **)-1) {
		// All modes available
		current_display_mode.flags = flags;
		current_display_mode.zoom = 1;
		zoom_available = false;
		return true;
	}

	int len = 0;
	while (modes[len])
		++len;

	for (int i = len-1; i > 0; --i) {
		if ((modes[i]->h == height && modes[i]->w == width)
#if defined(SUPPORT_ZOOM)
			|| (modes[i]->h == height*2 && modes[i]->w == width*2)
#endif
			) {
				current_display_mode.flags = flags;
				// FIXME: we have to find a way to make zoom possible only in windowed mode
				current_display_mode.zoom = modes[i]->w / width;
				zoom_available = current_display_mode.zoom == 2;
				return true;
		}
	}

	// Didn't find a suitable video mode
	return false;
}

void SdlUi::BeginDisplayModeChange() {
	last_display_mode = current_display_mode;
	current_display_mode.effective = false;
	mode_changing = true;
}

void SdlUi::EndDisplayModeChange() {
	// Check if the new display mode is different from last one
	if (mode_changing && (
		current_display_mode.flags != last_display_mode.flags ||
		current_display_mode.zoom != last_display_mode.zoom ||
		current_display_mode.width != last_display_mode.width ||
		current_display_mode.height != last_display_mode.height)) {

			if (!RefreshDisplayMode()) {
				// Mode change failed, check if last one was effective
				if (last_display_mode.effective) {
					current_display_mode = last_display_mode;

					// Try a rollback to last mode
					if (!RefreshDisplayMode()) {
						Output::Error("Couldn't rollback to last display mode.\n{}", SDL_GetError());
					}
				} else {
					Output::Error("Couldn't set display mode.\n{}", SDL_GetError());
				}
			}

			current_display_mode.effective = true;
			SetIsFullscreen((current_display_mode.flags & SDL_FULLSCREEN) == SDL_FULLSCREEN);

			mode_changing = false;
	}
}

bool SdlUi::RefreshDisplayMode() {
	uint32_t flags = current_display_mode.flags;
	int display_width = current_display_mode.width;
	int display_height = current_display_mode.height;

	if (zoom_available && current_display_mode.zoom == 2) {
		display_width *= 2;
		display_height *= 2;
	}

	int bpp = current_display_mode.bpp;

#ifdef GEKKO
	// force for SDL-wii, otherwise 16 bit is used
	bpp = 24;
#endif

	// Free non zoomed surface
	main_surface.reset();
	sdl_surface = SDL_SetVideoMode(display_width, display_height, bpp, flags);

	if (!sdl_surface)
		return false;

	// Modes below 15 bpp aren't supported
	if (sdl_surface->format->BitsPerPixel < 15)
		return false;

	current_display_mode.bpp = sdl_surface->format->BitsPerPixel;

	const DynamicFormat format(
		sdl_surface->format->BitsPerPixel,
		sdl_surface->format->Rmask,
		sdl_surface->format->Gmask,
		sdl_surface->format->Bmask,
		sdl_surface->format->Amask,
		PF::NoAlpha);

	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	if (zoom_available && current_display_mode.zoom == 2) {
		// Create a non zoomed surface as drawing surface
		main_surface = Bitmap::Create(current_display_mode.width,
											  current_display_mode.height,
											  false,
											  current_display_mode.bpp);

		if (!main_surface)
			return false;

	} else {
		void *pixels = (uint8_t*) sdl_surface->pixels;
		// Drawing surface will be the window itself
		main_surface = Bitmap::Create(
			pixels, sdl_surface->w, sdl_surface->h, sdl_surface->pitch, format);
	}

	return true;
}

void SdlUi::ToggleFullscreen() {
	BeginDisplayModeChange();
	if (toggle_fs_available && mode_changing) {
		if ((current_display_mode.flags & SDL_FULLSCREEN) == SDL_FULLSCREEN)
			current_display_mode.flags &= ~SDL_FULLSCREEN;
		else
			current_display_mode.flags |= SDL_FULLSCREEN;
	}
	EndDisplayModeChange();
}

void SdlUi::ToggleZoom() {
	BeginDisplayModeChange();
	if (zoom_available && mode_changing) {
		if(current_display_mode.zoom == 2)
			current_display_mode.zoom = 1;
		else
			current_display_mode.zoom = 2;
	}
	EndDisplayModeChange();
}

void SdlUi::ProcessEvents() {
	SDL_Event evnt;

	// Poll SDL events and process them
	while (SDL_PollEvent(&evnt)) {
		ProcessEvent(evnt);

		if (Player::exit_flag)
			break;
	}
}

void SdlUi::UpdateDisplay() {
	if (zoom_available && current_display_mode.zoom == 2) {
		// Blit drawing surface x2 scaled over window surface
		Blit2X(*main_surface, sdl_surface);
	}
	SDL_UpdateRect(sdl_surface, 0, 0, 0, 0);
}

void SdlUi::SetTitle(const std::string &title) {
	SDL_WM_SetCaption(title.c_str(), NULL);
}

bool SdlUi::ShowCursor(bool flag) {
	bool temp_flag = cursor_visible;
	cursor_visible = flag;
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
	return temp_flag;
}

bool SdlUi::LogMessage(const std::string &message) {
#ifdef GEKKO
	return Wii::LogMessage(message);
#else
	// not logged
	return false;
#endif
}

void SdlUi::Blit2X(Bitmap const& src, SDL_Surface* dst_surf) {
	if (SDL_MUSTLOCK(dst_surf)) SDL_LockSurface(dst_surf);

#if (defined(PLAYER_AMIGA) && !defined(__AROS__)) || defined(GEKKO)
	// Quick & dirty big endian 2x zoom blitter
	int blit_height = src.height() * 2;
	int blit_width = src.width();
	int src_pitch = src.pitch();
	int dst_pitch = dst_surf->pitch;
	int dst_bpp = sdl_surface->format->BitsPerPixel;

	uint8_t* src_pixels = (uint8_t*)src.pixels();
	uint8_t* dst_pixels = (uint8_t*)dst_surf->pixels;

	switch (dst_bpp) {
		case 32:
			for (int i = 0; i < blit_height; i++) {
				uint32_t* src = (uint32_t*)src_pixels;
				uint32_t* dst = (uint32_t*)dst_pixels;
				for (int j = 0; j < blit_width; j++) {
					uint32_t pixel = *src;
					*dst++ = pixel;
					*dst++ = pixel;
					src++;
				}
				dst_pixels += dst_pitch;
				if (i & 1) {
					src_pixels += src_pitch;
				}
			}
			break;
		case 24:
			for (int i = 0; i < blit_height; i++) {
				uint32_t* src = (uint32_t*)src_pixels;
				uint8_t* dst = (uint8_t*)dst_pixels;
				for (int j = 0; j < blit_width; j++) {
					uint8_t* pixels = (uint8_t*)src + 1;
					*dst++ = *pixels++;
					*dst++ = *pixels++;
					*dst++ = *pixels;
					pixels = (uint8_t*)src + 1;
					*dst++ = *pixels++;
					*dst++ = *pixels++;
					*dst++ = *pixels;
					src++;
				}
				dst_pixels += dst_pitch;
				if (i & 1) {
					src_pixels += src_pitch;
				}
			}
			break;
		case 16:
			for (int i = 0; i < blit_height; i++) {
				uint16_t* src = (uint16_t*)src_pixels;
				uint16_t* dst = (uint16_t*)dst_pixels;
				for (int j = 0; j < blit_width; j++) {
					// 5:5:5:1 RGBA to 6:5:5 RGB
					uint16_t pixel = (*src & 0x7FE0) << 1 | (*src & 0x001F);
					*dst++ = pixel;
					*dst++ = pixel;
					src++;
				}
				dst_pixels += dst_pitch;
				if (i & 1) {
					src_pixels += src_pitch;
				}
			}
			break;
		case 15:
			for (int i = 0; i < blit_height; i++) {
				uint16_t* src = (uint16_t*)src_pixels;
				uint16_t* dst = (uint16_t*)dst_pixels;
				for (int j = 0; j < blit_width; j++) {
					uint16_t pixel = *src;
					*dst++ = pixel;
					*dst++ = pixel;
					src++;
				}
				dst_pixels += dst_pitch;
				if (i & 1) {
					src_pixels += src_pitch;
				}
			}
			break;
	}
#else
	BitmapRef dst = Bitmap::Create(
		dst_surf->pixels,
		dst_surf->w,
		dst_surf->h,
		dst_surf->pitch,
		DynamicFormat(
			dst_surf->format->BitsPerPixel,
			dst_surf->format->Rmask,
			dst_surf->format->Gmask,
			dst_surf->format->Bmask,
			dst_surf->format->Amask,
			PF::NoAlpha));

	dst->Blit2x(dst->GetRect(), src, src.GetRect());
#endif

	if (SDL_MUSTLOCK(dst_surf)) SDL_UnlockSurface(dst_surf);
}

void SdlUi::ProcessEvent(SDL_Event &evnt) {
	switch (evnt.type) {
		case SDL_ACTIVEEVENT:
			ProcessActiveEvent(evnt);
			return;

		case SDL_QUIT:
			Player::exit_flag = true;
			return;

		case SDL_KEYDOWN:
			ProcessKeyDownEvent(evnt);
			return;

		case SDL_KEYUP:
			ProcessKeyUpEvent(evnt);
			return;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			ProcessJoystickButtonEvent(evnt);
			return;

		case SDL_JOYHATMOTION:
			ProcessJoystickHatEvent(evnt);
			return;

		case SDL_JOYAXISMOTION:
			ProcessJoystickAxisEvent(evnt);
			return;
	}
}

void SdlUi::ProcessActiveEvent(SDL_Event &evnt) {
	int state;
	state = evnt.active.state;

#if PAUSE_GAME_WHEN_FOCUS_LOST
	if (state == SDL_APPINPUTFOCUS && !evnt.active.gain) {
		Player::Pause();

		bool last = ShowCursor(true);

		// Filter SDL events until focus is regained
		SDL_Event wait_event;

		while (SDL_WaitEvent(&wait_event)) {
			if (FilterUntilFocus(&wait_event)) {
				break;
			}
		}

		ShowCursor(last);

		Player::Resume();
		ResetKeys();

		return;
	}
#endif
}

void SdlUi::ProcessKeyDownEvent(SDL_Event &evnt) {
#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
	if (evnt.key.keysym.sym == SDLK_F4 && (evnt.key.keysym.mod & KMOD_LALT)) {
		// Close program on LeftAlt+F4
		Player::exit_flag = true;
		return;
	} else if (evnt.key.keysym.sym == SDLK_RETURN ||
			evnt.key.keysym.sym == SDLK_KP_ENTER) {
		if (evnt.key.keysym.mod & KMOD_LALT || (evnt.key.keysym.mod & KMOD_RALT)) {
			// Toggle fullscreen on Alt+Enter
			ToggleFullscreen();
			return;
		}
	}

	// Update key state
	keys[SdlKey2InputKey(evnt.key.keysym.sym)] = true;
#else
	/* unused */
	(void) evnt;
#endif
}

void SdlUi::ProcessKeyUpEvent(SDL_Event &evnt) {
#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
	keys[SdlKey2InputKey(evnt.key.keysym.sym)] = false;
#else
	/* unused */
	(void) evnt;
#endif
}

void SdlUi::ProcessJoystickButtonEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	keys[SdlJKey2InputKey(evnt.jbutton.button)] = evnt.jbutton.state == SDL_PRESSED;
#endif
}

void SdlUi::ProcessJoystickHatEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
	// Set all states to false
	analog_input.secondary = {};

	// Check hat states
	if (evnt.jhat.value & SDL_HAT_UP)
		analog_input.secondary.y = -1.f;

	else if (evnt.jhat.value & SDL_HAT_RIGHT)
		analog_input.secondary.x = 1.f;

	else if (evnt.jhat.value & SDL_HAT_DOWN)
		analog_input.secondary.y = 1.f;

	else if (evnt.jhat.value & SDL_HAT_LEFT)
		analog_input.secondary.x = -1.f;
#endif
}

void SdlUi::ProcessJoystickAxisEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
	auto normalize = [](int value) {
		return static_cast<float>(value) / 32768.f;
	};

	switch (evnt.jaxis.axis) {
		case 0: // horizontal
			analog_input.primary.x = normalize(evnt.jaxis.value);
			break;
		case 1: // vertical
			analog_input.primary.y = -normalize(evnt.jaxis.value);
			break;
	}
#endif
}

void SdlUi::ResetKeys() {
	for (size_t i = 0; i < keys.size(); i++) {
		keys[i] = false;
	}
}

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
Input::Keys::InputKey SdlKey2InputKey(SDLKey sdlkey) {
	switch (sdlkey) {
		case SDLK_BACKSPACE		: return Input::Keys::BACKSPACE;
		case SDLK_TAB			: return Input::Keys::TAB;
		case SDLK_CLEAR			: return Input::Keys::CLEAR;
		case SDLK_RETURN		: return Input::Keys::RETURN;
		case SDLK_PAUSE			: return Input::Keys::PAUSE;
		case SDLK_ESCAPE		: return Input::Keys::ESCAPE;
		case SDLK_SPACE			: return Input::Keys::SPACE;
		case SDLK_PAGEUP		: return Input::Keys::PGUP;
		case SDLK_PAGEDOWN		: return Input::Keys::PGDN;
		case SDLK_END			: return Input::Keys::ENDS;
		case SDLK_HOME			: return Input::Keys::HOME;
		case SDLK_LEFT			: return Input::Keys::LEFT;
		case SDLK_UP			: return Input::Keys::UP;
		case SDLK_RIGHT			: return Input::Keys::RIGHT;
		case SDLK_DOWN			: return Input::Keys::DOWN;
		case SDLK_PRINT			: return Input::Keys::SNAPSHOT;
		case SDLK_INSERT		: return Input::Keys::INSERT;
		case SDLK_DELETE		: return Input::Keys::DEL;
		case SDLK_LSHIFT		: return Input::Keys::LSHIFT;
		case SDLK_RSHIFT		: return Input::Keys::RSHIFT;
		case SDLK_LCTRL			: return Input::Keys::LCTRL;
		case SDLK_RCTRL			: return Input::Keys::RCTRL;
		case SDLK_LALT			: return Input::Keys::LALT;
		case SDLK_RALT			: return Input::Keys::RALT;
		case SDLK_0				: return Input::Keys::N0;
		case SDLK_1				: return Input::Keys::N1;
		case SDLK_2				: return Input::Keys::N2;
		case SDLK_3				: return Input::Keys::N3;
		case SDLK_4				: return Input::Keys::N4;
		case SDLK_5				: return Input::Keys::N5;
		case SDLK_6				: return Input::Keys::N6;
		case SDLK_7				: return Input::Keys::N7;
		case SDLK_8				: return Input::Keys::N8;
		case SDLK_9				: return Input::Keys::N9;
		case SDLK_a				: return Input::Keys::A;
		case SDLK_b				: return Input::Keys::B;
		case SDLK_c				: return Input::Keys::C;
		case SDLK_d				: return Input::Keys::D;
		case SDLK_e				: return Input::Keys::E;
		case SDLK_f				: return Input::Keys::F;
		case SDLK_g				: return Input::Keys::G;
		case SDLK_h				: return Input::Keys::H;
		case SDLK_i				: return Input::Keys::I;
		case SDLK_j				: return Input::Keys::J;
		case SDLK_k				: return Input::Keys::K;
		case SDLK_l				: return Input::Keys::L;
		case SDLK_m				: return Input::Keys::M;
		case SDLK_n				: return Input::Keys::N;
		case SDLK_o				: return Input::Keys::O;
		case SDLK_p				: return Input::Keys::P;
		case SDLK_q				: return Input::Keys::Q;
		case SDLK_r				: return Input::Keys::R;
		case SDLK_s				: return Input::Keys::S;
		case SDLK_t				: return Input::Keys::T;
		case SDLK_u				: return Input::Keys::U;
		case SDLK_v				: return Input::Keys::V;
		case SDLK_w				: return Input::Keys::W;
		case SDLK_x				: return Input::Keys::X;
		case SDLK_y				: return Input::Keys::Y;
		case SDLK_z				: return Input::Keys::Z;
		case SDLK_LSUPER		: return Input::Keys::LOS;
		case SDLK_RSUPER		: return Input::Keys::ROS;
		case SDLK_MENU			: return Input::Keys::MENU;
		case SDLK_KP0			: return Input::Keys::KP0;
		case SDLK_KP1			: return Input::Keys::KP1;
		case SDLK_KP2			: return Input::Keys::KP2;
		case SDLK_KP3			: return Input::Keys::KP3;
		case SDLK_KP4			: return Input::Keys::KP4;
		case SDLK_KP5			: return Input::Keys::KP5;
		case SDLK_KP6			: return Input::Keys::KP6;
		case SDLK_KP7			: return Input::Keys::KP7;
		case SDLK_KP8			: return Input::Keys::KP8;
		case SDLK_KP9			: return Input::Keys::KP9;
		case SDLK_KP_MULTIPLY	: return Input::Keys::KP_MULTIPLY;
		case SDLK_KP_PLUS		: return Input::Keys::KP_ADD;
		case SDLK_KP_ENTER		: return Input::Keys::RETURN;
		case SDLK_KP_MINUS		: return Input::Keys::KP_SUBTRACT;
		case SDLK_KP_PERIOD		: return Input::Keys::KP_PERIOD;
		case SDLK_KP_DIVIDE		: return Input::Keys::KP_DIVIDE;
		case SDLK_F1			: return Input::Keys::F1;
		case SDLK_F2			: return Input::Keys::F2;
		case SDLK_F3			: return Input::Keys::F3;
		case SDLK_F4			: return Input::Keys::F4;
		case SDLK_F5			: return Input::Keys::F5;
		case SDLK_F6			: return Input::Keys::F6;
		case SDLK_F7			: return Input::Keys::F7;
		case SDLK_F8			: return Input::Keys::F8;
		case SDLK_F9			: return Input::Keys::F9;
		case SDLK_F10			: return Input::Keys::F10;
		case SDLK_F11			: return Input::Keys::F11;
		case SDLK_F12			: return Input::Keys::F12;
		case SDLK_CAPSLOCK		: return Input::Keys::CAPS_LOCK;
		case SDLK_NUMLOCK		: return Input::Keys::NUM_LOCK;
		case SDLK_SCROLLOCK		: return Input::Keys::SCROLL_LOCK;

		default					: return Input::Keys::NONE;
	}
}
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
Input::Keys::InputKey SdlJKey2InputKey(int button_index) {
	switch (button_index) {
		case 0	: return Input::Keys::JOY_OTHER_1;
		case 1	: return Input::Keys::JOY_OTHER_2;
		case 2	: return Input::Keys::JOY_OTHER_3;
		case 3	: return Input::Keys::JOY_OTHER_4;
		case 4	: return Input::Keys::JOY_OTHER_5;
		case 5	: return Input::Keys::JOY_OTHER_6;
		case 6	: return Input::Keys::JOY_OTHER_7;
		case 7	: return Input::Keys::JOY_OTHER_8;
		case 8	: return Input::Keys::JOY_OTHER_9;
		case 9	: return Input::Keys::JOY_OTHER_10;
		case 10	: return Input::Keys::JOY_OTHER_11;
		case 11	: return Input::Keys::JOY_OTHER_12;
		case 12	: return Input::Keys::JOY_OTHER_13;
		case 13	: return Input::Keys::JOY_OTHER_14;
		case 14	: return Input::Keys::JOY_OTHER_15;
		case 15	: return Input::Keys::JOY_OTHER_16;
		case 16	: return Input::Keys::JOY_OTHER_17;
		case 17	: return Input::Keys::JOY_OTHER_18;
		case 18	: return Input::Keys::JOY_OTHER_19;
		case 19	: return Input::Keys::JOY_OTHER_20;
		case 20	: return Input::Keys::JOY_OTHER_21;
		case 21	: return Input::Keys::JOY_OTHER_22;
		case 22	: return Input::Keys::JOY_OTHER_23;
		case 23	: return Input::Keys::JOY_OTHER_24;
		default : return Input::Keys::NONE;
	}
}
#endif

int FilterUntilFocus(const SDL_Event* evnt) {
	// Prevent throwing events away received after focus gained but filter
	// not detached.
	switch (evnt->type) {
	case SDL_QUIT:
		Player::exit_flag = true;
		return 1;

	case SDL_ACTIVEEVENT:
		return !!(evnt->active.state & SDL_APPINPUTFOCUS);

	default:
		return 0;
	}
}
