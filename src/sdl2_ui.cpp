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

#if USE_SDL==2

#include "sdl2_ui.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#elif defined(__ANDROID__)
#  include <jni.h>
#  include <SDL_system.h>
#elif defined(EMSCRIPTEN)
#  include <emscripten.h>
#endif
#include "icon.h"

// Prevent some XLib name clashes under Linux
#define Font Font_XLib
#define Drawable Drawable_XLib
#include "SDL_syswm.h"
#undef Font
#undef Drawable

#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"

#include "audio.h"

#ifdef SUPPORT_AUDIO

#  ifdef HAVE_SDL_MIXER
#    include "audio_sdl_mixer.h"
#  elif defined(HAVE_OPENAL)
#    include "audio_al.h"
#  else
#    include "audio_sdl.h"
#  endif

AudioInterface& Sdl2Ui::GetAudio() {
	return *audio_;
}
#endif

static int FilterUntilFocus(const SDL_Event* evnt);

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
	static Input::Keys::InputKey SdlKey2InputKey(SDL_Keycode sdlkey);
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	static Input::Keys::InputKey SdlJKey2InputKey(int button_index);
#endif

Sdl2Ui::Sdl2Ui(long width, long height, bool fullscreen, int zoom) :
	BaseUi(),
	zoom_available(false),
	toggle_fs_available(false),
	mode_changing(false) {

	uint32_t flags = SDL_INIT_VIDEO;

#ifndef EMSCRIPTEN
	flags |= SDL_INIT_TIMER;
#endif

	// Set some SDL environment variables before starting. These are platform
	// dependent, so every port needs to set them manually
#ifdef __LINUX__
	// Set the application class name
	setenv("SDL_VIDEO_X11_WMCLASS", GAME_TITLE, 0);
#endif

	if (SDL_Init(flags) < 0) {
		Output::Error("Couldn't initialize SDL.\n%s\n", SDL_GetError());
	}

	sdl_window = NULL;

	BeginDisplayModeChange();
		if (!RequestVideoMode(width, height, zoom)) {
			Output::Error("No suitable video resolution found. Aborting.");
		}
	EndDisplayModeChange();

	// Work around some SDL bugs, window properties are incorrect when started
	// as full screen, e.g. height lacks title bar size, icon is not added, etc.
	if (fullscreen)
		ToggleFullscreen();

	SetTitle(GAME_TITLE);

#if (defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)) || (defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS)) || (defined(USE_JOYSTICK_HAT) && defined(SUPPORT_JOYSTICK_HAT))
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		Output::Warning("Couldn't initialize joystick. %s", SDL_GetError());
	}

	SDL_JoystickEventState(1);
	SDL_JoystickOpen(0);
#endif

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	ShowCursor(true);
#else
	ShowCursor(false);
#endif

#ifdef SUPPORT_AUDIO
#  ifdef HAVE_SDL_MIXER
	if (!Player::no_audio_flag) {
		audio_.reset(new SdlMixerAudio());
		return;
	}
#  elif defined(HAVE_OPENAL)
	if (!Player::no_audio_flag) {
		audio_.reset(new ALAudio());
		return;
	}
#  else
	if (!Player::no_audio_flag) {
		audio_.reset(new SdlAudio());
		return;
	}
#  endif
#else
	audio_.reset(new EmptyAudio());
#endif
}

Sdl2Ui::~Sdl2Ui() {
	SDL_Quit();
}

uint32_t Sdl2Ui::GetTicks() const {
	return SDL_GetTicks();
}

void Sdl2Ui::Sleep(uint32_t time) {
#ifndef EMSCRIPTEN
	SDL_Delay(time);
#endif
}

bool Sdl2Ui::RequestVideoMode(int width, int height, int zoom) {
	// SDL2 documentation says that resolution dependent code should not be used
	// anymore. The library takes care of it now.
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	toggle_fs_available = true;

	current_display_mode.zoom = zoom;
#ifdef SUPPORT_ZOOM
	zoom_available = true;
#else
	zoom_available = false;
#endif

	return true;
}

void Sdl2Ui::BeginDisplayModeChange() {
	last_display_mode = current_display_mode;
	current_display_mode.effective = false;
	mode_changing = true;
}

void Sdl2Ui::EndDisplayModeChange() {
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
						Output::Error("Couldn't rollback to last display mode.\n%s", SDL_GetError());
					}
				} else {
					Output::Error("Couldn't set display mode.\n%s", SDL_GetError());
				}
			}

			current_display_mode.effective = true;

			mode_changing = false;
	}
}

bool Sdl2Ui::RefreshDisplayMode() {
	uint32_t flags = current_display_mode.flags;
	int display_width = current_display_mode.width;
	int display_height = current_display_mode.height;

	if (zoom_available) {
		display_width *= current_display_mode.zoom;
		display_height *= current_display_mode.zoom;
	}

	if (!sdl_window) {
		#ifdef __ANDROID__
		// Workaround SDL bug: https://bugzilla.libsdl.org/show_bug.cgi?id=2291
		// Set back buffer format to 565
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		#endif

		// Create our window
		sdl_window = SDL_CreateWindow(GAME_TITLE,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			display_width, display_height,
			SDL_WINDOW_RESIZABLE | flags);

		if (!sdl_window)
			return false;

		SetAppIcon();

		// OS X needs the rendered to be vsync
		#if defined(__APPLE__) && defined(__MACH__)
			uint32_t rendered_flag = SDL_RENDERER_PRESENTVSYNC;
		#else
			uint32_t rendered_flag = 0;
		#endif

		sdl_renderer = SDL_CreateRenderer(sdl_window, -1, rendered_flag);
		if (!sdl_renderer)
			return false;

		// Flush display
		SDL_RenderClear(sdl_renderer);
		SDL_RenderPresent(sdl_renderer);

		SDL_RenderSetLogicalSize(sdl_renderer, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT);

		uint32_t const texture_format =
			SDL_BYTEORDER == SDL_LIL_ENDIAN
			? SDL_PIXELFORMAT_ABGR8888
			: SDL_PIXELFORMAT_RGBA8888;

		sdl_texture = SDL_CreateTexture(sdl_renderer,
			texture_format,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT);

		if (!sdl_texture)
			return false;
	} else {
		// Browser handles fast resizing for emscripten, TODO: use fullscreen API
#ifndef EMSCRIPTEN
		bool is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
		if (is_fullscreen) {
			SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		} else {
			SDL_SetWindowFullscreen(sdl_window, 0);
			if ((last_display_mode.flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
					== SDL_WINDOW_FULLSCREEN_DESKTOP) {
				// Restore to pre-fullscreen size
				SDL_SetWindowSize(sdl_window, 0, 0);
			} else {
				SDL_SetWindowSize(sdl_window, display_width, display_height);
			}
		}
#endif
	}
	// Need to set up icon again, some platforms recreate the window when
	// creating the renderer (i.e. Windows), see also comment in SetAppIcon()
	SetAppIcon();

	#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
	#else
	const DynamicFormat format(
		32,
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		PF::NoAlpha);
	#endif

	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	if (!main_surface) {
		// Drawing surface will be the window itself
		main_surface = Bitmap::Create(
			SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, Color(0, 0, 0, 255));
	}

	return true;
}

#ifdef SUPPORT_FULL_SCALING
void Sdl2Ui::Resize(long width, long height) {
	if (mode_changing) {
		current_display_mode.width = width;
		current_display_mode.height = height;
	}
}
#else
void Sdl2Ui::Resize(long /*width*/, long /*height*/) {
}
#endif

void Sdl2Ui::ToggleFullscreen() {
	BeginDisplayModeChange();
	if (toggle_fs_available && mode_changing) {
		if ((current_display_mode.flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
			current_display_mode.flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
		else
			current_display_mode.flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	EndDisplayModeChange();
}

void Sdl2Ui::ToggleZoom() {
	BeginDisplayModeChange();
	// Work around a SDL bug which doesn't demaximize the window when the size
	// is changed
	int flags = SDL_GetWindowFlags(sdl_window);
	if ((flags & SDL_WINDOW_MAXIMIZED) == SDL_WINDOW_MAXIMIZED) {
		SDL_RestoreWindow(sdl_window);
	}

	if (zoom_available && mode_changing) {
		// get current window size, calculate next bigger zoom factor
		int w, h;
		SDL_GetWindowSize(sdl_window, &w, &h);
		last_display_mode.zoom = std::min(w / SCREEN_TARGET_WIDTH, h / SCREEN_TARGET_HEIGHT);
		current_display_mode.zoom = last_display_mode.zoom + 1;

		// get maximum usable window size
		int display_index = SDL_GetWindowDisplayIndex(sdl_window);
		SDL_Rect max_mode;
#if SDL_VERSION_ATLEAST(2, 0, 5)
		// this takes account of the menu bar and dock on macOS and task bar on windows
		SDL_GetDisplayUsableBounds(display_index, &max_mode);
#else
		SDL_GetDisplayBounds(display_index, &max_mode);
#endif

		// reset zoom, if it does not fit
		if ((max_mode.h < SCREEN_TARGET_HEIGHT * current_display_mode.zoom) ||
			(max_mode.w < SCREEN_TARGET_WIDTH * current_display_mode.zoom)) {
			current_display_mode.zoom = 1;
		}
	}
	EndDisplayModeChange();
}

void Sdl2Ui::ProcessEvents() {
	SDL_Event evnt;

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	// Reset Mouse scroll
	if (Player::mouse_flag) {
		keys[Input::Keys::MOUSE_SCROLLUP] = false;
		keys[Input::Keys::MOUSE_SCROLLDOWN] = false;
	}
#endif

	// Poll SDL events and process them
	while (SDL_PollEvent(&evnt)) {
		ProcessEvent(evnt);

		if (Player::exit_flag)
			break;
	}
}

void Sdl2Ui::UpdateDisplay() {
	SDL_UpdateTexture(sdl_texture, NULL, main_surface->pixels(), main_surface->pitch());
	SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
}

void Sdl2Ui::SetTitle(const std::string &title) {
	SDL_SetWindowTitle(sdl_window, title.c_str());
}

bool Sdl2Ui::ShowCursor(bool flag) {
#ifdef __WINRT__
	// Prevent cursor hide in WinRT because it is hidden everywhere while the app runs...
	return flag;
#else
	bool temp_flag = cursor_visible;
	cursor_visible = flag;
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
	return temp_flag;
#endif
}

void Sdl2Ui::ProcessEvent(SDL_Event &evnt) {
	switch (evnt.type) {
		case SDL_WINDOWEVENT:
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

		case SDL_MOUSEMOTION:
			ProcessMouseMotionEvent(evnt);
			return;

		case SDL_MOUSEWHEEL:
			ProcessMouseWheelEvent(evnt);
			return;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			ProcessMouseButtonEvent(evnt);
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

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			ProcessFingerEvent(evnt);
			return;
	}
}

void Sdl2Ui::ProcessActiveEvent(SDL_Event &evnt) {
	int state = evnt.window.event;
#if PAUSE_GAME_WHEN_FOCUS_LOST
	if (state == SDL_WINDOWEVENT_FOCUS_LOST) {

		Player::Pause();

		bool last = ShowCursor(true);

#ifndef EMSCRIPTEN
		// Filter SDL events until focus is regained

		SDL_Event wait_event;

		while (SDL_WaitEvent(&wait_event)) {
			if (FilterUntilFocus(&wait_event)) {
				break;
			}
		}
#endif

		ShowCursor(last);

		Player::Resume();
		ResetKeys();

		return;
	}
#endif
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	if (state == SDL_WINDOWEVENT_ENTER) {
		mouse_focus = true;
	} else if (state == SDL_WINDOWEVENT_LEAVE) {
		mouse_focus = false;
	}
#endif
}

void Sdl2Ui::ProcessKeyDownEvent(SDL_Event &evnt) {
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
	keys[SdlKey2InputKey(evnt.key.keysym.scancode)] = true;
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessKeyUpEvent(SDL_Event &evnt) {
#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
	keys[SdlKey2InputKey(evnt.key.keysym.scancode)] = false;
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessMouseMotionEvent(SDL_Event& evnt) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	mouse_focus = true;
	mouse_x = evnt.motion.x;
	mouse_y = evnt.motion.y;
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessMouseWheelEvent(SDL_Event& evnt) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	if (!Player::mouse_flag)
		return;

	// Ignore Finger (touch) events here
	if (evnt.wheel.which == SDL_TOUCH_MOUSEID)
		return;

	int amount = evnt.wheel.y;

	// FIXME: Debian ships older SDL2
#if SDL_VERSION_ATLEAST(2, 0, 4)
	// translate direction
	if (evnt.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
		amount *= -1;
#endif

	keys[Input::Keys::MOUSE_SCROLLUP] = amount > 0;
	keys[Input::Keys::MOUSE_SCROLLDOWN] = amount < 0;
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessMouseButtonEvent(SDL_Event& evnt) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	if (!Player::mouse_flag)
		return;

	// Ignore Finger (touch) events here
	if (evnt.button.which == SDL_TOUCH_MOUSEID)
		return;

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
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessJoystickButtonEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	keys[SdlJKey2InputKey(evnt.jbutton.button)] = evnt.jbutton.state == SDL_PRESSED;
#endif
}

void Sdl2Ui::ProcessJoystickHatEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK_HAT)  && defined(SUPPORT_JOYSTICK_HAT)
	// Set all states to false
	keys[Input::Keys::JOY_HAT_LOWER_LEFT] = false;
	keys[Input::Keys::JOY_HAT_DOWN] = false;
	keys[Input::Keys::JOY_HAT_LOWER_RIGHT] = false;
	keys[Input::Keys::JOY_HAT_LEFT] = false;
	keys[Input::Keys::JOY_HAT_RIGHT] = false;
	keys[Input::Keys::JOY_HAT_UPPER_LEFT] = false;
	keys[Input::Keys::JOY_HAT_UP] = false;
	keys[Input::Keys::JOY_HAT_UPPER_RIGHT] = false;

	// Check hat states
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
#endif
}

void Sdl2Ui::ProcessJoystickAxisEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
	// Horizontal axis
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

	// Vertical Axis
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
#endif
}

void Sdl2Ui::ProcessFingerEvent(SDL_Event& evnt) {
#if defined(USE_TOUCH) && defined(SUPPORT_TOUCH)
	SDL_TouchID touchid;
	int fingers = 0;

	if (!Player::touch_flag)
		return;

	// We currently ignore swipe gestures
	if (evnt.type != SDL_FINGERMOTION) {
		/* FIXME: To simplify things, we lazily only get the current number of
		   fingers touching the first device (hoping nobody actually uses
		   multiple devices). This way we do not need to keep track on finger
		   IDs and deal with the timing.
		*/
		touchid = SDL_GetTouchDevice(0);
		if (touchid != 0)
			fingers = SDL_GetNumTouchFingers(touchid);

		keys[Input::Keys::ONE_FINGER] = fingers == 1;
		keys[Input::Keys::TWO_FINGERS] = fingers == 2;
	}
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::SetAppIcon() {
#if !defined(_WIN32)
	/* SDL handles transfering the application icon to new or recreated windows,
	   if initially set through it (see below). So no need to set again for all
	   platforms relying on it. Platforms defined above need special treatment.
	*/
	static bool icon_set = false;

	if (icon_set)
		return;
#endif
	bool load_error = false;
#ifdef _WIN32
#ifndef __WINRT__
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version)
	SDL_bool success = SDL_GetWindowWMInfo(sdl_window, &wminfo);

	if (success < 0)
		Output::Error("Wrong SDL version");

	HWND window;
	HINSTANCE handle = GetModuleHandle(NULL);
	HICON icon = LoadIcon(handle, MAKEINTRESOURCE(23456));
	HICON icon_small = (HICON) LoadImage(handle, MAKEINTRESOURCE(23456), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	load_error = (icon == NULL || icon_small == NULL);
#endif
#else
	//Linux, OS X
	#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		uint32_t Rmask = 0x000000FF;
		uint32_t Gmask = 0x0000FF00;
		uint32_t Bmask = 0x00FF0000;
		uint32_t Amask = 0xFF000000;
	#else
		uint32_t Rmask = 0xFF000000;
		uint32_t Gmask = 0x00FF0000;
		uint32_t Bmask = 0x0000FF00;
		uint32_t Amask = 0x000000FF;
	#endif
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(icon32, ICON_SIZE, ICON_SIZE, 32, ICON_SIZE*4, Rmask, Gmask, Bmask, Amask);
	load_error = (icon == NULL);
#endif
	if (load_error)
		Output::Warning("Could not load window icon.");

#ifdef _WIN32
#ifndef __WINRT__
	window = wminfo.info.win.window;
	SetClassLongPtr(window, GCLP_HICON, (LONG_PTR) icon);
	SetClassLongPtr(window, GCLP_HICONSM, (LONG_PTR) icon_small);
#endif
#else
	SDL_SetWindowIcon(sdl_window, icon);
	SDL_FreeSurface(icon);
	icon_set = true;
#endif
}

void Sdl2Ui::ResetKeys() {
	for (size_t i = 0; i < keys.size(); i++) {
		keys[i] = false;
	}
}

bool Sdl2Ui::IsFullscreen() {
	return (current_display_mode.flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
}

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
Input::Keys::InputKey SdlKey2InputKey(SDL_Keycode sdlkey) {
	switch (sdlkey) {
		case SDL_SCANCODE_BACKSPACE		: return Input::Keys::BACKSPACE;
		case SDL_SCANCODE_TAB			: return Input::Keys::TAB;
		case SDL_SCANCODE_CLEAR			: return Input::Keys::CLEAR;
		case SDL_SCANCODE_RETURN		: return Input::Keys::RETURN;
		case SDL_SCANCODE_PAUSE			: return Input::Keys::PAUSE;
		case SDL_SCANCODE_ESCAPE		: return Input::Keys::ESCAPE;
		case SDL_SCANCODE_SPACE			: return Input::Keys::SPACE;
		case SDL_SCANCODE_PAGEUP		: return Input::Keys::PGUP;
		case SDL_SCANCODE_PAGEDOWN		: return Input::Keys::PGDN;
		case SDL_SCANCODE_END			: return Input::Keys::ENDS;
		case SDL_SCANCODE_HOME			: return Input::Keys::HOME;
		case SDL_SCANCODE_LEFT			: return Input::Keys::LEFT;
		case SDL_SCANCODE_UP			: return Input::Keys::UP;
		case SDL_SCANCODE_RIGHT			: return Input::Keys::RIGHT;
		case SDL_SCANCODE_DOWN			: return Input::Keys::DOWN;
		case SDL_SCANCODE_PRINTSCREEN	: return Input::Keys::SNAPSHOT;
		case SDL_SCANCODE_INSERT		: return Input::Keys::INSERT;
		case SDL_SCANCODE_DELETE		: return Input::Keys::DEL;
		case SDL_SCANCODE_LSHIFT		: return Input::Keys::LSHIFT;
		case SDL_SCANCODE_RSHIFT		: return Input::Keys::RSHIFT;
		case SDL_SCANCODE_LCTRL			: return Input::Keys::LCTRL;
		case SDL_SCANCODE_RCTRL			: return Input::Keys::RCTRL;
		case SDL_SCANCODE_LALT			: return Input::Keys::LALT;
		case SDL_SCANCODE_RALT			: return Input::Keys::RALT;
		case SDL_SCANCODE_0				: return Input::Keys::N0;
		case SDL_SCANCODE_1				: return Input::Keys::N1;
		case SDL_SCANCODE_2				: return Input::Keys::N2;
		case SDL_SCANCODE_3				: return Input::Keys::N3;
		case SDL_SCANCODE_4				: return Input::Keys::N4;
		case SDL_SCANCODE_5				: return Input::Keys::N5;
		case SDL_SCANCODE_6				: return Input::Keys::N6;
		case SDL_SCANCODE_7				: return Input::Keys::N7;
		case SDL_SCANCODE_8				: return Input::Keys::N8;
		case SDL_SCANCODE_9				: return Input::Keys::N9;
		case SDL_SCANCODE_A				: return Input::Keys::A;
		case SDL_SCANCODE_B				: return Input::Keys::B;
		case SDL_SCANCODE_C				: return Input::Keys::C;
		case SDL_SCANCODE_D				: return Input::Keys::D;
		case SDL_SCANCODE_E				: return Input::Keys::E;
		case SDL_SCANCODE_F				: return Input::Keys::F;
		case SDL_SCANCODE_G				: return Input::Keys::G;
		case SDL_SCANCODE_H				: return Input::Keys::H;
		case SDL_SCANCODE_I				: return Input::Keys::I;
		case SDL_SCANCODE_J				: return Input::Keys::J;
		case SDL_SCANCODE_K				: return Input::Keys::K;
		case SDL_SCANCODE_L				: return Input::Keys::L;
		case SDL_SCANCODE_M				: return Input::Keys::M;
		case SDL_SCANCODE_N				: return Input::Keys::N;
		case SDL_SCANCODE_O				: return Input::Keys::O;
		case SDL_SCANCODE_P				: return Input::Keys::P;
		case SDL_SCANCODE_Q				: return Input::Keys::Q;
		case SDL_SCANCODE_R				: return Input::Keys::R;
		case SDL_SCANCODE_S				: return Input::Keys::S;
		case SDL_SCANCODE_T				: return Input::Keys::T;
		case SDL_SCANCODE_U				: return Input::Keys::U;
		case SDL_SCANCODE_V				: return Input::Keys::V;
		case SDL_SCANCODE_W				: return Input::Keys::W;
		case SDL_SCANCODE_X				: return Input::Keys::X;
		case SDL_SCANCODE_Y				: return Input::Keys::Y;
		case SDL_SCANCODE_Z				: return Input::Keys::Z;
		case SDL_SCANCODE_MENU			: return Input::Keys::MENU;
		case SDL_SCANCODE_KP_0			: return Input::Keys::KP0;
		case SDL_SCANCODE_KP_1			: return Input::Keys::KP1;
		case SDL_SCANCODE_KP_2			: return Input::Keys::KP2;
		case SDL_SCANCODE_KP_3			: return Input::Keys::KP3;
		case SDL_SCANCODE_KP_4			: return Input::Keys::KP4;
		case SDL_SCANCODE_KP_5			: return Input::Keys::KP5;
		case SDL_SCANCODE_KP_6			: return Input::Keys::KP6;
		case SDL_SCANCODE_KP_7			: return Input::Keys::KP7;
		case SDL_SCANCODE_KP_8			: return Input::Keys::KP8;
		case SDL_SCANCODE_KP_9			: return Input::Keys::KP9;
		case SDL_SCANCODE_KP_MULTIPLY	: return Input::Keys::MULTIPLY;
		case SDL_SCANCODE_KP_PLUS		: return Input::Keys::ADD;
		case SDL_SCANCODE_KP_ENTER		: return Input::Keys::RETURN;
		case SDL_SCANCODE_KP_MINUS		: return Input::Keys::SUBTRACT;
		case SDL_SCANCODE_KP_PERIOD		: return Input::Keys::PERIOD;
		case SDL_SCANCODE_KP_DIVIDE		: return Input::Keys::DIVIDE;
		case SDL_SCANCODE_F1			: return Input::Keys::F1;
		case SDL_SCANCODE_F2			: return Input::Keys::F2;
		case SDL_SCANCODE_F3			: return Input::Keys::F3;
		case SDL_SCANCODE_F4			: return Input::Keys::F4;
		case SDL_SCANCODE_F5			: return Input::Keys::F5;
		case SDL_SCANCODE_F6			: return Input::Keys::F6;
		case SDL_SCANCODE_F7			: return Input::Keys::F7;
		case SDL_SCANCODE_F8			: return Input::Keys::F8;
		case SDL_SCANCODE_F9			: return Input::Keys::F9;
		case SDL_SCANCODE_F10			: return Input::Keys::F10;
		case SDL_SCANCODE_F11			: return Input::Keys::F11;
		case SDL_SCANCODE_F12			: return Input::Keys::F12;
		case SDL_SCANCODE_CAPSLOCK		: return Input::Keys::CAPS_LOCK;
		case SDL_SCANCODE_NUMLOCKCLEAR	: return Input::Keys::NUM_LOCK;
		case SDL_SCANCODE_SCROLLLOCK	: return Input::Keys::SCROLL_LOCK;
		case SDL_SCANCODE_AC_BACK		: return Input::Keys::AC_BACK;
		case SDL_SCANCODE_SELECT		: return Input::Keys::SELECT;

		default					: return Input::Keys::NONE;
	}
}
#endif

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

int FilterUntilFocus(const SDL_Event* evnt) {
	// Prevent throwing events away received after focus gained but filter
	// not detached.
	switch (evnt->type) {
	case SDL_QUIT:
		Player::exit_flag = true;
		return 1;

	case SDL_WINDOWEVENT:
		return (evnt->window.event == SDL_WINDOWEVENT_FOCUS_GAINED);

	default:
		return 0;
	}
}

#endif // USE_SDL
