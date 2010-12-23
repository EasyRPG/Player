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

#ifdef USE_SDL

///////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////
#include "sdl_ui.h"
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include "SDL_syswm.h"
#elif GEKKO
	#include <gccore.h>
	#include <wiiuse/wpad.h>
#endif
#include "color.h"
#include "font_render_8x8.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"

///////////////////////////////////////////////////////////
#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
static Input::Keys::InputKey SdlKey2InputKey(SDLKey sdlkey);
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
static Input::Keys::InputKey SdlJKey2InputKey(int button_index);
#endif

static int FilterUntilFocus(const SDL_Event* evnt);

#ifdef GEKKO
static void GekkoResetCallback();
#endif

SdlUi* DisplaySdlUi;

///////////////////////////////////////////////////////////
SdlUi::SdlUi(long width, long height, const std::string title, bool fs_flag) :
	zoom_available(true),
	toggle_fs_available(false),
	mode_changing(false),
	main_window(NULL),
	main_surface(NULL),
	back_color(0),
	mouse_focus(false),
	mouse_x(0),
	mouse_y(0),
	cursor_visible(false) {

	keys.resize(Input::Keys::KEYS_COUNT, false);

#ifdef GEKKO
	// Initialize the video system
	VIDEO_Init();
	GXRModeObj* rmode = VIDEO_GetPreferredMode(NULL);
	void* xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	WPAD_Init();

	SYS_SetResetCallback(GekkoResetCallback);
#endif

	uint32 flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
#ifdef DEBUG
	flags |= SDL_INIT_NOPARACHUTE;
#endif

	if (SDL_Init(flags) < 0) {
		Output::Error("Couldn't initialize SDL.\n%s\n", SDL_GetError());
	}

	SetAppIcon();

	StartDisplayModeChange();

	if (!RequestVideoMode(width, height, fs_flag)) {
		Output::Error("No suitable video resolution found. Aborting.");
	}

	EndDisplayModeChange();

	SetTitle(title);

#if (defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)) || (defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)) || (defined(USE_JOYSTICK_HAT)  && defined(SUPPORT_JOYSTICK_HAT))
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		Output::Warning("Couldn't initialize joystick.\n%s\n", SDL_GetError());
	}

	SDL_JoystickEventState(1);
	SDL_JoystickOpen(0);
#endif

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	ShowCursor(true);
#else
	ShowCursor(false);
#endif

	DisplaySdlUi = this;
}

///////////////////////////////////////////////////////////
SdlUi::~SdlUi() {
	SDL_FreeSurface(main_surface);
	SDL_Quit();
}

bool SdlUi::RequestVideoMode(int width, int height, bool fullscreen) {
	const SDL_VideoInfo *vinfo;
	SDL_Rect **modes;
	uint32 flags = SDL_SWSURFACE;

	vinfo = SDL_GetVideoInfo();

	current_display_mode.height = height;
	current_display_mode.width = width;

	if (vinfo->wm_available) {
		toggle_fs_available = true;
		do {
			if (fullscreen) {
				flags |= SDL_FULLSCREEN;
			}

			modes = SDL_ListModes(NULL, flags);

			if (modes != NULL) {
				// Set up...
				current_display_mode.flags = flags;

				if (modes == (SDL_Rect **)-1) {
					// All modes available
					// If we have a high res, turn zoom on
					current_display_mode.zoom = (vinfo->current_h > height*2 && vinfo->current_w > width*2);
					zoom_available = current_display_mode.zoom;
					return true;
				} else {
					int len = 0;
					while (modes[len]) 
						++len;

					for (int i = len-1; i >= 0; --i) {
						if (
							(modes[i]->h == height && modes[i]->w == width) ||
							(modes[i]->h == height*2 && modes[i]->w == width*2)
						) {
							current_display_mode.zoom = ((modes[i]->w >> 1) == width);
							zoom_available = current_display_mode.zoom;
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
		} while (true);
	} // wm_available

	if (!fullscreen) {
		// Stop here since we need a window manager for non fullscreen modes
		return false;
	}

	if (vinfo->hw_available) {
		zoom_available = false;
		flags |= SDL_FULLSCREEN | SDL_HWSURFACE;
		
		current_display_mode.flags = flags;
		current_display_mode.zoom = false;

		modes = SDL_ListModes(NULL, flags);
		if (modes != NULL) {
			if (modes == (SDL_Rect **)-1) {
				// All modes available... Yay!
				return true;
			} else {
				int len = 0;
				while (modes[len]) 
					++len;
				for (int i = len-1; i > 0; --i) {
					if (modes[i]->h == height && modes[i]->w == width) {
						return true;
					}
				}
			}
		}
	} // hw_available

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
		current_display_mode.zoom = false;
		zoom_available = current_display_mode.zoom;
		return true;
	}

	int len = 0;
	while (modes[len]) 
		++len;
	for (int i = len-1; i > 0; --i) {
		if (
			(modes[i]->h == height && modes[i]->w == width) ||
			(modes[i]->h == height*2 && modes[i]->w == width*2)
			) {
				current_display_mode.flags = flags;
				current_display_mode.zoom = ((modes[i]->w >> 1) == width);
				zoom_available = current_display_mode.zoom;
				return true;
		}
	}

	// Didn't find a suitable video mode
	return false;
}

////////////////////////////////////////////////////////////
void SdlUi::StartDisplayModeChange() {
	last_display_mode = current_display_mode;
	current_display_mode.effective = false;
	mode_changing = true;
}

////////////////////////////////////////////////////////////
void SdlUi::EndDisplayModeChange() {
	if (mode_changing && (
		current_display_mode.flags != last_display_mode.flags ||
		current_display_mode.zoom != last_display_mode.zoom ||
		current_display_mode.width != last_display_mode.width ||
		current_display_mode.height != last_display_mode.height)) {

			if (!RefreshDisplayMode()) {
				if (last_display_mode.effective) {
					current_display_mode = last_display_mode;
					if (!RefreshDisplayMode()) {
						Output::Error("Couldn't rollback to last display mode.\n%s\n", SDL_GetError());
					}
				} else {
					Output::Error("Couldn't set display mode.\n%s\n", SDL_GetError());
				}
			}

			current_display_mode.effective = true;

			mode_changing = false;
	}
}

////////////////////////////////////////////////////////////
bool SdlUi::RefreshDisplayMode() {

	uint32 flags = current_display_mode.flags;
	int display_width = current_display_mode.width;
	int display_height = current_display_mode.height;
	int bpp = current_display_mode.bpp;

	Graphics::fps_showing = (flags & SDL_FULLSCREEN) == SDL_FULLSCREEN;

	if (zoom_available && current_display_mode.zoom) {
		display_width *= 2;
		display_height *= 2;
	}

	if (main_surface != main_window) {
		SDL_FreeSurface(main_surface);
		main_surface = NULL;
	}
	
	main_window = SDL_SetVideoMode(display_width, display_height, bpp, flags);
	if (!main_window)
		return false;

	if (main_window->format->BitsPerPixel == 16 ||
		main_window->format->BitsPerPixel == 32) {
		current_display_mode.bpp = main_window->format->BitsPerPixel;
	} else {
		// 16 or 32 bits are required
		return false;
	}

	if (zoom_available && current_display_mode.zoom) {
		main_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			current_display_mode.width,
			current_display_mode.height,
			main_window->format->BitsPerPixel,
			main_window->format->Rmask,
			main_window->format->Gmask,
			main_window->format->Bmask,
			main_window->format->Amask
		);
		if (!main_surface) 
			return false;
	} else {
		main_surface = main_window;
	}

	return true;
}

void SdlUi::SetBackcolor(const Color& color) {
	back_color = SDL_MapRGB(main_surface->format, color.red, color.green, color.blue);
}

///////////////////////////////////////////////////////////
#ifdef SUPPORT_FULL_SCALING
void SdlUi::Resize(long width, long height) {
	if (mode_changing) {
		current_display_mode.width = width;
		current_display_mode.height = height;
	}
}
#else
void SdlUi::Resize(long /*width*/, long /*height*/) {
}
#endif

///////////////////////////////////////////////////////////
void SdlUi::ToggleFullscreen() {
	if (toggle_fs_available && mode_changing) {
		if ((current_display_mode.flags & SDL_FULLSCREEN) == SDL_FULLSCREEN) {
			current_display_mode.flags &= ~SDL_FULLSCREEN;
		} else {
			current_display_mode.flags |= SDL_FULLSCREEN;
		}
	}
}

///////////////////////////////////////////////////////////
void SdlUi::ToggleZoom() {
	if (zoom_available && mode_changing) {
		current_display_mode.zoom = !current_display_mode.zoom;
	}
}

///////////////////////////////////////////////////////////
void SdlUi::ProcessEvents() {
	SDL_Event evnt;

	while ( SDL_PollEvent(&evnt) ) {

		ProcessEvent(evnt);

		if (Player::exit_flag)
			break;
	}
}

///////////////////////////////////////////////////////////
void SdlUi::CleanDisplay() {
	SDL_FillRect(main_surface, &main_surface->clip_rect, back_color);
}

///////////////////////////////////////////////////////////
void SdlUi::UpdateDisplay() {
	if (zoom_available && current_display_mode.zoom)
		Blit2X(main_surface, main_window);

	SDL_UpdateRect(main_window, 0, 0, 0, 0);
}

///////////////////////////////////////////////////////////
void SdlUi::SetTitle(const std::string title) {
	SDL_WM_SetCaption(title.c_str(), NULL);
}

///////////////////////////////////////////////////////////
void SdlUi::DrawScreenText(const std::string& text) {
	if (SDL_MUSTLOCK(main_surface))
		SDL_LockSurface(main_surface);

	static uint32 color = SDL_MapRGB(main_surface->format, 255, 255, 255);

	FontRender8x8::TextDraw(text, (uint8*)main_surface->pixels, 10, 10, main_surface->w, main_surface->h, main_surface->format->BytesPerPixel, color);
	
	if (SDL_MUSTLOCK(main_surface))
		SDL_UnlockSurface(main_surface);
}

///////////////////////////////////////////////////////////
bool SdlUi::ShowCursor(bool flag) {
	bool temp_flag = cursor_visible;
	cursor_visible = flag;
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
	return temp_flag;
}

void stretch16(uint16* s, uint16* d, int w) {
	for(int i = 0; i < w; i++) {
		*d++ = *s;
		*d++ = *s++;
	}
}

void stretch32(uint32* s, uint32* d, int w)
{
	for(int i = 0; i < w; i++) {
		*d++ = *s;
		*d++ = *s++;
	}
}

///////////////////////////////////////////////////////////
void SdlUi::Blit2X(SDL_Surface* src, SDL_Surface* dst) {
	int src_pitch, dst_pitch;

	if (src == dst) {
		return;
	}
	
	if (SDL_MUSTLOCK(src))
		SDL_LockSurface(src);

	if (SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);

	//uint32* src_pixels = (uint32*)src->pixels;
	//uint32* dst_pixels = (uint32*)dst->pixels;

	src_pitch = src->pitch / src->format->BytesPerPixel;
	dst_pitch = dst->pitch / dst->format->BytesPerPixel;

	if (current_display_mode.bpp == 16) {
		uint16* src_pixels = (uint16*) src->pixels;
		uint16* dst_pixels = (uint16*) dst->pixels;
		for (register int i = 0; i < src->h; i++) {
			stretch16(src_pixels, dst_pixels, src->w);
			dst_pixels += dst_pitch;
			stretch16(src_pixels, dst_pixels, src->w);
			src_pixels += src_pitch;
			dst_pixels += dst_pitch;
		}
	} else {
		uint32* src_pixels = (uint32*) src->pixels;
		uint32* dst_pixels = (uint32*) dst->pixels;
		for (register int i = 0; i < src->h; i++) {
			stretch32(src_pixels, dst_pixels, src->w);
			dst_pixels += dst_pitch;
			stretch32(src_pixels, dst_pixels, src->w);
			src_pixels += src_pitch;
			dst_pixels += dst_pitch;
		}
	}

	if (SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);

	if (SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
}

///////////////////////////////////////////////////////////
void SdlUi::ProcessEvent(SDL_Event &evnt) {
	switch (evnt.type) {
		case SDL_ACTIVEEVENT:
			switch (evnt.active.state) {
#ifdef PAUSE_GAME_WHEN_FOCUS_LOST
				case SDL_APPINPUTFOCUS:
					if (!evnt.active.gain) {
#ifdef _WIN32
						// Prevent the player from hanging when it receives a
						// focus changed event but actually has focus.
						// This happens when a MsgBox appears.
						if (GetActiveWindow() != NULL) {
							return;
						}
#endif

						Player::Pause();

						SDL_SetEventFilter(&FilterUntilFocus);

						bool last = ShowCursor(true);

						SDL_WaitEvent(NULL);

						ShowCursor(last);

						SDL_SetEventFilter(NULL);

						ResetKeys();

						Player::Resume();
					}
					return;
#endif
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
				case SDL_APPMOUSEFOCUS:
					mouse_focus = evnt.active.gain == 1;
					return;
#endif
				default: return;
			}

		case SDL_QUIT:
			Player::exit_flag = true;
			return;

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
		case SDL_KEYDOWN:
			switch (evnt.key.keysym.sym) {
				case SDLK_F4:
					// AltGr+F4 does nothing
					if (!(evnt.key.keysym.mod & KMOD_RALT)) {
#ifdef _WIN32
						// Close Program on LeftAlt+F4
						if (evnt.key.keysym.mod & KMOD_LALT) {
							Player::exit_flag = true;
							return;
						}
#endif
						// Otherwise F4 toggles fullscreen
						StartDisplayModeChange();
							ToggleFullscreen();
						EndDisplayModeChange();
					}
					return;

				case SDLK_F5:
					StartDisplayModeChange();
						ToggleZoom();
					EndDisplayModeChange();
					return;

				case SDLK_F12:
					Player::reset_flag = true;
					return;

				case SDLK_RETURN:
				case SDLK_KP_ENTER:
					if (evnt.key.keysym.mod & KMOD_LALT || (evnt.key.keysym.mod & KMOD_RALT)) {
						StartDisplayModeChange();
							ToggleFullscreen();
						EndDisplayModeChange();
						return;
					}

				default:
					keys[SdlKey2InputKey(evnt.key.keysym.sym)] = true;
					return;
			}

		case SDL_KEYUP:
			keys[SdlKey2InputKey(evnt.key.keysym.sym)] = false;
			return;
#endif

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
		case SDL_MOUSEMOTION:
			mouse_focus = true;
			mouse_x = evnt.motion.x;
			mouse_y = evnt.motion.y;
			return;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			switch (evnt.button.button) {
				case SDL_BUTTON_LEFT:
					keys[Input::Keys::MOUSE_LEFT] = evnt.button.state == SDL_PRESSED;
					break;
				case SDL_BUTTON_MIDDLE:
					keys[Input::Keys::MOUSE_MIDDLE] = evnt.button.state == SDL_PRESSED;
					break;
				case SDL_BUTTON_RIGHT:
					keys[Input::Keys::MOUSE_RIGHT] = evnt.button.state == SDL_PRESSED;
					break;
			}
			return;
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			keys[SdlJKey2InputKey(evnt.jbutton.button)] = evnt.jbutton.state == SDL_PRESSED;
			return;
#endif

#if defined(USE_JOYSTICK_HAT)  && defined(SUPPORT_JOYSTICK_HAT)
		case SDL_JOYHATMOTION:
			keys[Input::Keys::JOY_HAT_LOWER_LEFT] = false;
			keys[Input::Keys::JOY_HAT_DOWN] = false;
			keys[Input::Keys::JOY_HAT_LOWER_RIGHT] = false;
			keys[Input::Keys::JOY_HAT_LEFT] = false;
			keys[Input::Keys::JOY_HAT_RIGHT] = false;
			keys[Input::Keys::JOY_HAT_UPPER_LEFT] = false;
			keys[Input::Keys::JOY_HAT_UP] = false;
			keys[Input::Keys::JOY_HAT_UPPER_RIGHT] = false;

			if ((evnt.jhat.value & SDL_HAT_RIGHTUP) == SDL_HAT_RIGHTUP)
				keys[Input::Keys::JOY_HAT_UPPER_RIGHT] = true;	

			else if ((evnt.jhat.value & SDL_HAT_RIGHTDOWN)  == SDL_HAT_RIGHTDOWN)
				keys[Input::Keys::JOY_HAT_LOWER_RIGHT] = true;

			else if ((evnt.jhat.value & SDL_HAT_LEFTUP)  == SDL_HAT_LEFTUP)
				keys[Input::Keys::JOY_HAT_UPPER_LEFT] = true;

			else if ((evnt.jhat.value & SDL_HAT_LEFTDOWN)  == SDL_HAT_LEFTDOWN)
				keys[Input::Keys::JOY_HAT_LOWER_LEFT] = true;

			else if (evnt.jhat.value & SDL_HAT_UP)
				keys[Input::Keys::JOY_HAT_UP] = true;

			else if (evnt.jhat.value & SDL_HAT_RIGHT)
				keys[Input::Keys::JOY_HAT_RIGHT] = true;

			else if (evnt.jhat.value & SDL_HAT_DOWN)
				keys[Input::Keys::JOY_HAT_DOWN] = true;

			else if (evnt.jhat.value & SDL_HAT_LEFT)
				keys[Input::Keys::JOY_HAT_LEFT] = true;

			return;
#endif

#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
		case SDL_JOYAXISMOTION:
			if (evnt.jaxis.axis == 0) {
				if (evnt.jaxis.value < -JOYSTICK_AXIS_SENSIBILITY) {
					keys[Input::Keys::JOY_AXIS_X_LEFT] = true;
					keys[Input::Keys::JOY_AXIS_X_RIGHT] = false;
				} else if (evnt.jaxis.value > JOYSTICK_AXIS_SENSIBILITY) {
					keys[Input::Keys::JOY_AXIS_X_LEFT] = false;
					keys[Input::Keys::JOY_AXIS_X_RIGHT] = true;
				} else {
					keys[Input::Keys::JOY_AXIS_X_LEFT] = false;
					keys[Input::Keys::JOY_AXIS_X_RIGHT] = false;
				}
			} else if (evnt.jaxis.axis == 1) {
				if (evnt.jaxis.value < -JOYSTICK_AXIS_SENSIBILITY) {
					keys[Input::Keys::JOY_AXIS_Y_UP] = true;
					keys[Input::Keys::JOY_AXIS_Y_DOWN] = false;
				} else if (evnt.jaxis.value > JOYSTICK_AXIS_SENSIBILITY) {
					keys[Input::Keys::JOY_AXIS_Y_UP] = false;
					keys[Input::Keys::JOY_AXIS_Y_DOWN] = true;
				} else {
					keys[Input::Keys::JOY_AXIS_Y_UP] = false;
					keys[Input::Keys::JOY_AXIS_Y_DOWN] = false;
				}
			}
			return;
#endif
	}
}

///////////////////////////////////////////////////////////
void SdlUi::SetAppIcon() {
#ifdef _WIN32
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version)

	if (SDL_GetWMInfo(&wminfo) < 0)
		Output::Error("Wrong SDL version");

	HINSTANCE handle = GetModuleHandle(NULL);
	HICON icon = LoadIcon(handle, MAKEINTRESOURCE(45678));

	if (icon == NULL)
		Output::Error("Couldn't load icon.");

	SetClassLongPtr(wminfo.window, GCLP_HICON, (LONG_PTR) icon);
#endif
}

///////////////////////////////////////////////////////////
void SdlUi::ResetKeys() {
	for (uint i = 0; i < keys.size(); i++) {
		keys[i] = false;
	}
}

///////////////////////////////////////////////////////////
bool SdlUi::IsFullscreen() {
	return (current_display_mode.flags & SDL_FULLSCREEN) == SDL_FULLSCREEN;
}
long SdlUi::GetWidth() {
	return current_display_mode.width;
}

long SdlUi::GetHeight() {
	return current_display_mode.height;
}

std::vector<bool> &SdlUi::GetKeyStates() {
	return keys;
}

bool SdlUi::GetMouseFocus() {
	return mouse_focus;
}

int SdlUi::GetMousePosX() {
	return mouse_x;
}

int SdlUi::GetMousePosY() {
	return mouse_y;
}

SDL_Surface* SdlUi::GetDisplaySurface() {
	return main_surface;
}

///////////////////////////////////////////////////////////
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
		case SDLK_KP_MULTIPLY	: return Input::Keys::MULTIPLY;
		case SDLK_KP_PLUS		: return Input::Keys::ADD;
		case SDLK_KP_ENTER		: return Input::Keys::RETURN;
		case SDLK_KP_MINUS		: return Input::Keys::SUBTRACT;
		case SDLK_KP_PERIOD		: return Input::Keys::PERIOD;
		case SDLK_KP_DIVIDE		: return Input::Keys::DIVIDE;
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

///////////////////////////////////////////////////////////
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
Input::Keys::InputKey SdlJKey2InputKey(int button_index) {
	switch (button_index) {
		case 0	: return Input::Keys::JOY_0;
		case 1	: return Input::Keys::JOY_1;
		case 2	: return Input::Keys::JOY_2;
		case 3	: return Input::Keys::JOY_3;
		case 4	: return Input::Keys::JOY_4;
		case 5	: return Input::Keys::JOY_5;
		case 6	: return Input::Keys::JOY_6;
		case 7	: return Input::Keys::JOY_7;
		case 8	: return Input::Keys::JOY_8;
		case 9	: return Input::Keys::JOY_9;
		case 10	: return Input::Keys::JOY_10;
		case 11	: return Input::Keys::JOY_11;
		case 12	: return Input::Keys::JOY_12;
		case 13	: return Input::Keys::JOY_13;
		case 14	: return Input::Keys::JOY_14;
		case 15	: return Input::Keys::JOY_15;
		case 16	: return Input::Keys::JOY_16;
		case 17	: return Input::Keys::JOY_17;
		case 18	: return Input::Keys::JOY_18;
		case 19	: return Input::Keys::JOY_19;
		case 20	: return Input::Keys::JOY_20;
		case 21	: return Input::Keys::JOY_21;
		case 22	: return Input::Keys::JOY_22;
		case 23	: return Input::Keys::JOY_23;
		case 24	: return Input::Keys::JOY_24;
		case 25	: return Input::Keys::JOY_25;
		case 26	: return Input::Keys::JOY_23;
		case 27	: return Input::Keys::JOY_27;
		case 28	: return Input::Keys::JOY_28;
		case 29	: return Input::Keys::JOY_29;
		case 30	: return Input::Keys::JOY_30;
		case 31	: return Input::Keys::JOY_31;
		default : return Input::Keys::NONE;
	}
}
#endif

///////////////////////////////////////////////////
int FilterUntilFocus(const SDL_Event* evnt) {
	switch (evnt->type) {
	case SDL_QUIT:
		Player::exit_flag = true;
		return 1;

	case SDL_ACTIVEEVENT:
		return evnt->active.state & SDL_APPINPUTFOCUS;

	default:
		return 0;
	}
}

///////////////////////////////////////////////////
#ifdef GEKKO
void GekkoResetCallback() {
	Player::reset_flag = true;
}
#endif

#endif
