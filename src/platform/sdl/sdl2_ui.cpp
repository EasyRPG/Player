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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include "game_config.h"
#include "system.h"
#include "sdl2_ui.h"

#ifdef _WIN32
#  include <windows.h>
#  include <SDL_syswm.h>
#elif defined(__ANDROID__)
#  include <jni.h>
#  include <SDL_system.h>
#elif defined(EMSCRIPTEN)
#  include <emscripten.h>
#elif defined(__WIIU__)
#  include <whb/proc.h>
#endif
#include "icon.h"

#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include "lcf/scope_guard.h"

#if defined(__APPLE__) && TARGET_OS_OSX
#  include "platform/macos/macos_utils.h"
#endif

#ifdef SUPPORT_AUDIO
#  include "sdl_audio.h"

AudioInterface& Sdl2Ui::GetAudio() {
	return *audio_;
}
#endif

static uint32_t GetDefaultFormat() {
	return SDL_PIXELFORMAT_RGBA32;
}

/**
 * Return preference for the given sdl format.
 * Higher numbers are better, -1 means unsupported.
 * We prefer formats which have fast paths in pixman.
 */
static int GetFormatRank(uint32_t fmt) {
	switch (fmt) {
		case SDL_PIXELFORMAT_RGBA32:
			return 2;
		case SDL_PIXELFORMAT_BGRA32:
			return 2;
		case SDL_PIXELFORMAT_ARGB32:
			return 1;
		case SDL_PIXELFORMAT_ABGR32:
			return 0;
		default:
			return -1;
	}
}

static DynamicFormat GetDynamicFormat(uint32_t fmt) {
	switch (fmt) {
		case SDL_PIXELFORMAT_RGBA32:
			return format_R8G8B8A8_n().format();
		case SDL_PIXELFORMAT_BGRA32:
			return format_B8G8R8A8_n().format();
		case SDL_PIXELFORMAT_ARGB32:
			return format_A8R8G8B8_n().format();
		case SDL_PIXELFORMAT_ABGR32:
			return format_A8B8G8R8_n().format();
		default:
			return DynamicFormat();
	}
}

static uint32_t SelectFormat(const SDL_RendererInfo& rinfo, bool print_all) {
	uint32_t current_fmt = SDL_PIXELFORMAT_UNKNOWN;
	int current_rank = -1;

	for (int i = 0; i < static_cast<int>(rinfo.num_texture_formats); ++i) {
		const auto fmt = rinfo.texture_formats[i];
		int rank = GetFormatRank(fmt);
		if (rank >= 0) {
			if (rank > current_rank) {
				current_fmt = fmt;
				current_rank = rank;
			}
			Output::Debug("SDL2: Detected format ({}) {} : rank=({})",
					i, SDL_GetPixelFormatName(fmt), rank);
		} else {
			if (print_all) {
				Output::Debug("SDL2: Detected format ({}) {} : Not Supported",
						i, SDL_GetPixelFormatName(fmt));
			}
		}
	}
	return current_fmt;
}

static int FilterUntilFocus(const SDL_Event* evnt);

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
static Input::Keys::InputKey SdlKey2InputKey(SDL_Keycode sdlkey);
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
static Input::Keys::InputKey SdlJKey2InputKey(int button_index);
#endif

Sdl2Ui::Sdl2Ui(long width, long height, const Game_Config& cfg) : BaseUi(cfg)
{
	// Set some SDL environment variables before starting. These are platform
	// dependent, so every port needs to set them manually
#ifdef __LINUX__
	// Set the application class name
	setenv("SDL_VIDEO_X11_WMCLASS", GAME_TITLE, 0);
#endif
#ifdef EMSCRIPTEN
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");
	// Only handle keyboard events when the canvas has focus
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
#endif
#ifdef __WIIU__
	//WHBProcInit();
#endif

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		Output::Error("Couldn't initialize SDL.\n{}\n", SDL_GetError());
	}

	RequestVideoMode(width, height,
			cfg.video.window_zoom.Get(),
			cfg.video.fullscreen.Get(),
			cfg.video.vsync.Get());

	SetTitle(GAME_TITLE);

#if (defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)) || (defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS))
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
		Output::Warning("Couldn't initialize joystick. {}", SDL_GetError());
	}

	SDL_JoystickEventState(SDL_ENABLE);
	sdl_joystick = SDL_JoystickOpen(0);
#endif

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	ShowCursor(true);
#else
	ShowCursor(false);
#endif

#ifdef SUPPORT_AUDIO
	if (!Player::no_audio_flag) {
		audio_ = std::make_unique<SdlAudio>(cfg.audio);
		return;
	}
#endif
}

Sdl2Ui::~Sdl2Ui() {
	if (sdl_joystick) {
		SDL_JoystickClose(sdl_joystick);
	}
	if (sdl_texture_game) {
		SDL_DestroyTexture(sdl_texture_game);
	}
	if (sdl_texture_scaled) {
		SDL_DestroyTexture(sdl_texture_scaled);
	}
	if (sdl_renderer) {
		SDL_DestroyRenderer(sdl_renderer);
	}
	if (sdl_window) {
		SDL_DestroyWindow(sdl_window);
	}
	SDL_Quit();

#ifdef __WIIU__
	//WHBProcShutdown();
#endif
}

bool Sdl2Ui::vChangeDisplaySurfaceResolution(int new_width, int new_height) {
	SDL_Texture* new_sdl_texture_game = SDL_CreateTexture(sdl_renderer,
		texture_format,
		SDL_TEXTUREACCESS_STREAMING,
		new_width, new_height);

	if (!new_sdl_texture_game) {
		Output::Warning("ChangeDisplaySurfaceResolution SDL_CreateTexture failed: {}", SDL_GetError());
		return false;
	}

	if (sdl_texture_game) {
		SDL_DestroyTexture(sdl_texture_game);
	}

	sdl_texture_game = new_sdl_texture_game;

	BitmapRef new_main_surface = Bitmap::Create(new_width, new_height, Color(0, 0, 0, 255));

	if (!new_main_surface) {
		Output::Warning("ChangeDisplaySurfaceResolution Bitmap::Create failed");
		return false;
	}

	main_surface = new_main_surface;
	window.size_changed = true;

	BeginDisplayModeChange();

	current_display_mode.width = new_width;
	current_display_mode.height = new_height;

	EndDisplayModeChange();

	return true;
}

void Sdl2Ui::RequestVideoMode(int width, int height, int zoom, bool fullscreen, bool vsync) {
	BeginDisplayModeChange();

	// SDL2 documentation says that resolution dependent code should not be used
	// anymore. The library takes care of it now.
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	current_display_mode.zoom = zoom;
	current_display_mode.vsync = vsync;

	EndDisplayModeChange();

	// Work around some SDL bugs, window properties are incorrect when started
	// as full screen, e.g. height lacks title bar size, icon is not added, etc.
	if (fullscreen)
		ToggleFullscreen();
}

void Sdl2Ui::BeginDisplayModeChange() {
	last_display_mode = current_display_mode;
	current_display_mode.effective = false;
}

void Sdl2Ui::EndDisplayModeChange() {
	// Check if the new display mode is different from last one
	if (current_display_mode.flags != last_display_mode.flags ||
		current_display_mode.zoom != last_display_mode.zoom ||
		current_display_mode.width != last_display_mode.width ||
		current_display_mode.height != last_display_mode.height) {

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
#ifdef EMSCRIPTEN
			SetIsFullscreen(true);
#else
			SetIsFullscreen((current_display_mode.flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
	}
}

bool Sdl2Ui::RefreshDisplayMode() {
	uint32_t flags = current_display_mode.flags;
	int display_width = current_display_mode.width;
	int display_height = current_display_mode.height;
	bool& vsync = current_display_mode.vsync;

#ifdef SUPPORT_ZOOM
	int display_width_zoomed = display_width * current_display_mode.zoom;
	int display_height_zoomed = display_height * current_display_mode.zoom;
#else
	int display_width_zoomed = display_width;
	int display_height_zoomed = display_height;
#endif

	if (!sdl_window) {
		#ifdef __ANDROID__
		// Workaround SDL bug: https://bugzilla.libsdl.org/show_bug.cgi?id=2291
		// Set back buffer format to 565
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		#endif

		#if defined(__APPLE__) && TARGET_OS_OSX
		// Use OpenGL on Mac only -- to work around an SDL Metal deficiency
		// where it will always use discrete GPU.
		// See SDL source code:
		// http://hg.libsdl.org/SDL/file/aa9d7c43a982/src/render/metal/SDL_render_metal.m#l1613
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
		#endif

		#if defined(EMSCRIPTEN) || defined(_WIN32)
		// FIXME: This will not DPI-scale on Windows due to SDL2 limitations.
		// Is properly fixed in SDL3. See #2764
		flags |= SDL_WINDOW_ALLOW_HIGHDPI;
		#endif

		// Create our window
		if (vcfg.window_x.Get() < 0 || vcfg.window_y.Get() < 0 || vcfg.window_height.Get() <= 0 || vcfg.window_width.Get() <= 0) {
			sdl_window = SDL_CreateWindow(GAME_TITLE,
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				display_width_zoomed, display_height_zoomed,
				SDL_WINDOW_RESIZABLE | flags);
		} else {
			sdl_window = SDL_CreateWindow(GAME_TITLE,
				vcfg.window_x.Get(),
				vcfg.window_y.Get(),
				vcfg.window_width.Get(), vcfg.window_height.Get(),
				SDL_WINDOW_RESIZABLE | flags);
		}

		if (!sdl_window) {
			Output::Debug("SDL_CreateWindow failed : {}", SDL_GetError());
			return false;
		}

		SDL_GetWindowSize(sdl_window, &window.width, &window.height);
		window.size_changed = true;

		auto window_sg = lcf::makeScopeGuard([&]() {
				SDL_DestroyWindow(sdl_window);
				sdl_window = nullptr;
		});

		SetAppIcon();

		uint32_t renderer_flags = 0;
		if (vsync) {
			renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
		}

		sdl_renderer = SDL_CreateRenderer(sdl_window, -1, renderer_flags);
		if (!sdl_renderer) {
			Output::Debug("SDL_CreateRenderer failed : {}", SDL_GetError());
			return false;
		}

		auto renderer_sg = lcf::makeScopeGuard([&]() {
				SDL_DestroyRenderer(sdl_renderer);
				sdl_renderer = nullptr;
				});

		SDL_RendererInfo rinfo = {};
		if (SDL_GetRendererInfo(sdl_renderer, &rinfo) == 0) {
			Output::Debug("SDL2: RendererInfo hw={} sw={} vsync={}",
					!!(rinfo.flags & SDL_RENDERER_ACCELERATED),
					!!(rinfo.flags & SDL_RENDERER_SOFTWARE),
					!!(rinfo.flags & SDL_RENDERER_PRESENTVSYNC)
					);
			texture_format = SelectFormat(rinfo, false);
		} else {
			Output::Debug("SDL_GetRendererInfo failed : {}", SDL_GetError());
		}

		vsync = rinfo.flags & SDL_RENDERER_PRESENTVSYNC;
		SetFrameRateSynchronized(vsync);

		if (texture_format == SDL_PIXELFORMAT_UNKNOWN) {
			texture_format = GetDefaultFormat();
			Output::Debug("SDL2: None of the ({}) detected formats were supported! Falling back to {}. This will likely cause performance degredation.",
					rinfo.num_texture_formats, SDL_GetPixelFormatName(texture_format));
			// Run again to print all the formats on this system.
			SelectFormat(rinfo, true);
		}

		Output::Debug("SDL2: Selected Pixel Format {}", SDL_GetPixelFormatName(texture_format));

		// Flush display
		SDL_RenderClear(sdl_renderer);
		SDL_RenderPresent(sdl_renderer);

		sdl_texture_game = SDL_CreateTexture(sdl_renderer,
			texture_format,
			SDL_TEXTUREACCESS_STREAMING,
			display_width, display_height);

		if (!sdl_texture_game) {
			Output::Debug("SDL_CreateTexture failed : {}", SDL_GetError());
			return false;
		}

		renderer_sg.Dismiss();
		window_sg.Dismiss();
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
				SDL_SetWindowSize(sdl_window, display_width_zoomed, display_height_zoomed);
			}
		}
#endif
	}
	// Need to set up icon again, some platforms recreate the window when
	// creating the renderer (i.e. Windows), see also comment in SetAppIcon()
	SetAppIcon();

	uint32_t sdl_pixel_fmt = GetDefaultFormat();
	int a, w, h;

	if (SDL_QueryTexture(sdl_texture_game, &sdl_pixel_fmt, &a, &w, &h) != 0) {
		Output::Debug("SDL_QueryTexture failed : {}", SDL_GetError());
		return false;
	}

	auto format = GetDynamicFormat(sdl_pixel_fmt);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	if (!main_surface) {
		// Drawing surface will be the window itself
		main_surface = Bitmap::Create(
			display_width, display_height, Color(0, 0, 0, 255));
	}

	return true;
}

void Sdl2Ui::ToggleFullscreen() {
	BeginDisplayModeChange();
	if ((current_display_mode.flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) {
		current_display_mode.flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else {
		current_display_mode.flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_GetWindowPosition(sdl_window, &window_mode_metrics.x, &window_mode_metrics.y);
		window_mode_metrics.width = window.width;
		window_mode_metrics.height = window.height;
	}
	EndDisplayModeChange();
}

void Sdl2Ui::ToggleZoom() {
#ifdef SUPPORT_ZOOM
	BeginDisplayModeChange();
	// Work around a SDL bug which doesn't demaximize the window when the size
	// is changed
	int flags = SDL_GetWindowFlags(sdl_window);
	if ((flags & SDL_WINDOW_MAXIMIZED) == SDL_WINDOW_MAXIMIZED) {
		SDL_RestoreWindow(sdl_window);
	}

	// get current window size, calculate next bigger zoom factor
	int w, h;
	SDL_GetWindowSize(sdl_window, &w, &h);
	last_display_mode.zoom = std::min(w / main_surface->width(), h / main_surface->height());
	current_display_mode.zoom = last_display_mode.zoom + 1;

	// get maximum usable window size
	int display_index = SDL_GetWindowDisplayIndex(sdl_window);
	SDL_Rect max_mode;
	// this takes account of the menu bar and dock on macOS and task bar on windows
	SDL_GetDisplayUsableBounds(display_index, &max_mode);

	// reset zoom, if it does not fit
	if ((max_mode.h < main_surface->height() * current_display_mode.zoom) ||
		(max_mode.w < main_surface->width() * current_display_mode.zoom)) {
		current_display_mode.zoom = 1;
	}
	EndDisplayModeChange();
#endif
}

void Sdl2Ui::ProcessEvents() {
	SDL_Event evnt;

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	// Reset Mouse scroll
	keys[Input::Keys::MOUSE_SCROLLUP] = false;
	keys[Input::Keys::MOUSE_SCROLLDOWN] = false;
#endif

	// Poll SDL events and process them
	while (SDL_PollEvent(&evnt)) {
		ProcessEvent(evnt);

		if (Player::exit_flag)
			break;
	}
}

void Sdl2Ui::SetScalingMode(ScalingMode mode) {
	window.size_changed = true;
	vcfg.scaling_mode.Set(mode);
}

void Sdl2Ui::ToggleStretch() {
	window.size_changed = true;
	vcfg.stretch.Toggle();
}

void Sdl2Ui::ToggleVsync() {
#if SDL_VERSION_ATLEAST(2, 0, 18)
	// Modifying vsync requires recreating the renderer
	vcfg.vsync.Toggle();

	if (SDL_RenderSetVSync(sdl_renderer, int(vcfg.vsync.Get())) == 0) {
		current_display_mode.vsync = vcfg.vsync.Get();
		SetFrameRateSynchronized(vcfg.vsync.Get());
	} else {
		Output::Warning("Unable to toggle vsync. This is likely a problem with your system configuration.");
	}
#else
	Output::Warning("Cannot toogle vsync: SDL2 version too old (must be 2.0.18)");
#endif
}

void Sdl2Ui::UpdateDisplay() {
	// SDL_UpdateTexture was found to be faster than SDL_LockTexture / SDL_UnlockTexture.
	SDL_UpdateTexture(sdl_texture_game, nullptr, main_surface->pixels(), main_surface->pitch());

	if (window.size_changed && window.width > 0 && window.height > 0) {
		// Based on SDL2 function UpdateLogicalSize
		window.size_changed = false;

		float width_float = static_cast<float>(window.width);
		float height_float = static_cast<float>(window.height);

		float want_aspect = (float)main_surface->width() / main_surface->height();
		float real_aspect = width_float / height_float;

		auto do_stretch = [this]() {
			if (vcfg.stretch.Get()) {
				viewport.x = 0;
				viewport.w = window.width;
			}
		};

		if (vcfg.scaling_mode.Get() == ScalingMode::Integer) {
			// Integer division on purpose
			if (want_aspect > real_aspect) {
				window.scale = static_cast<float>(window.width / main_surface->width());
			} else {
				window.scale = static_cast<float>(window.height / main_surface->height());
			}

			viewport.w = static_cast<int>(ceilf(main_surface->width() * window.scale));
			viewport.x = (window.width - viewport.w) / 2;
			viewport.h = static_cast<int>(ceilf(main_surface->height() * window.scale));
			viewport.y = (window.height - viewport.h) / 2;
			do_stretch();

			SDL_RenderSetViewport(sdl_renderer, &viewport);
		} else if (fabs(want_aspect - real_aspect) < 0.0001) {
			// The aspect ratios are the same, let SDL2 scale it
			window.scale = width_float / main_surface->width();
			SDL_RenderSetViewport(sdl_renderer, nullptr);

			// Only used here for the mouse coordinates
			viewport.x = 0;
			viewport.y = 0;
			viewport.w = window.width;
			viewport.h = window.height;
		} else if (want_aspect > real_aspect) {
			// Letterboxing (black bars top and bottom)
			window.scale = width_float / main_surface->width();
			viewport.x = 0;
			viewport.w = window.width;
			viewport.h = static_cast<int>(ceilf(main_surface->height() * window.scale));
			viewport.y = (window.height - viewport.h) / 2;
			do_stretch();
			SDL_RenderSetViewport(sdl_renderer, &viewport);
		} else {
			// black bars left and right
			window.scale = height_float / main_surface->height();
			viewport.y = 0;
			viewport.h = window.height;
			viewport.w = static_cast<int>(ceilf(main_surface->width() * window.scale));
			viewport.x = (window.width - viewport.w) / 2;
			do_stretch();
			SDL_RenderSetViewport(sdl_renderer, &viewport);
		}

		if (vcfg.scaling_mode.Get() == ScalingMode::Bilinear && window.scale > 0.f) {
			if (sdl_texture_scaled) {
				SDL_DestroyTexture(sdl_texture_scaled);
			}
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
			sdl_texture_scaled = SDL_CreateTexture(sdl_renderer, texture_format, SDL_TEXTUREACCESS_TARGET,
			   static_cast<int>(ceilf(window.scale)) * main_surface->width(), static_cast<int>(ceilf(window.scale)) * main_surface->height());
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
			if (!sdl_texture_scaled) {
				Output::Debug("SDL_CreateTexture failed : {}", SDL_GetError());
			}
		}
	}

	SDL_RenderClear(sdl_renderer);
	if (vcfg.scaling_mode.Get() == ScalingMode::Bilinear && window.scale > 0.f) {
		// Render game texture on the scaled texture
		SDL_SetRenderTarget(sdl_renderer, sdl_texture_scaled);
		SDL_RenderClear(sdl_renderer);
		SDL_RenderCopy(sdl_renderer, sdl_texture_game, nullptr, nullptr);

		SDL_SetRenderTarget(sdl_renderer, nullptr);
		SDL_RenderCopy(sdl_renderer, sdl_texture_scaled, nullptr, nullptr);
	} else {
		SDL_RenderCopy(sdl_renderer, sdl_texture_game, nullptr, nullptr);
	}
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
			ProcessWindowEvent(evnt);
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

		case SDL_CONTROLLERDEVICEADDED:
			ProcessControllerAdded(evnt);
			return;

		case SDL_CONTROLLERDEVICEREMOVED:
			ProcessControllerRemoved(evnt);
			return;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			ProcessControllerButtonEvent(evnt);
			return;

		case SDL_CONTROLLERAXISMOTION:
			ProcessControllerAxisEvent(evnt);
			return;

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			ProcessFingerEvent(evnt);
			return;
	}
}

void Sdl2Ui::ProcessWindowEvent(SDL_Event &evnt) {
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
#if defined(USE_MOUSE_OR_TOUCH) && defined(SUPPORT_MOUSE_OR_TOUCH)
	if (state == SDL_WINDOWEVENT_ENTER) {
		mouse_focus = true;
	} else if (state == SDL_WINDOWEVENT_LEAVE) {
		mouse_focus = false;
	}
#endif
	if (state == SDL_WINDOWEVENT_SIZE_CHANGED || state == SDL_WINDOWEVENT_RESIZED) {
		window.width = evnt.window.data1;
		window.height = evnt.window.data2;

#ifdef EMSCRIPTEN
		double display_ratio = emscripten_get_device_pixel_ratio();
		window.width = static_cast<int>(window.width * display_ratio);
		window.height = static_cast<int>(window.height * display_ratio);
#endif

		window.size_changed = true;
	}
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
#if defined(USE_MOUSE_OR_TOUCH) && defined(SUPPORT_MOUSE_OR_TOUCH)
	mouse_focus = true;

	int xw = viewport.w;
	int yh = viewport.h;

	if (xw == 0 || yh == 0) {
		// Startup. No viewport yet
		return;
	}

#ifdef EMSCRIPTEN
	double display_ratio = emscripten_get_device_pixel_ratio();
	mouse_pos.x = (evnt.motion.x * display_ratio - viewport.x) * main_surface->width() / xw;
	mouse_pos.y = (evnt.motion.y * display_ratio - viewport.y) * main_surface->height() / yh;
#else
	mouse_pos.x = (evnt.motion.x - viewport.x) * main_surface->width() / xw;
	mouse_pos.y = (evnt.motion.y - viewport.y) * main_surface->height() / yh;
#endif

#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessMouseWheelEvent(SDL_Event& evnt) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	// Ignore Finger (touch) events here
	if (evnt.wheel.which == SDL_TOUCH_MOUSEID)
		return;

	int amount = evnt.wheel.y;

	// translate direction
	if (evnt.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
		amount *= -1;

	keys[Input::Keys::MOUSE_SCROLLUP] = amount > 0;
	keys[Input::Keys::MOUSE_SCROLLDOWN] = amount < 0;
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::ProcessMouseButtonEvent(SDL_Event& evnt) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
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

void Sdl2Ui::ProcessControllerAdded(SDL_Event& evnt) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	int id = evnt.cdevice.which;
	if (SDL_IsGameController(id)) {
		SDL_GameController* controller = SDL_GameControllerOpen(id);
		if (controller) {
			Output::Debug("Controller {} ({}) added", id, SDL_GameControllerName(controller));
		}
	}
#endif
}

void Sdl2Ui::ProcessControllerRemoved(SDL_Event &evnt) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	int id = evnt.cdevice.which;
	SDL_GameController* controller = SDL_GameControllerFromInstanceID(id);
	if (controller) {
		Output::Debug("Controller {} ({}) removed", id, SDL_GameControllerName(controller));
		SDL_GameControllerClose(controller);
	}
#endif
}

void Sdl2Ui::ProcessControllerButtonEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	keys[SdlJKey2InputKey(evnt.cbutton.button)] = evnt.cbutton.state == SDL_PRESSED;
#endif
}

void Sdl2Ui::ProcessControllerAxisEvent(SDL_Event &evnt) {
#if defined(USE_JOYSTICK_AXIS)  && defined(SUPPORT_JOYSTICK_AXIS)
	int axis = evnt.caxis.axis;
	int value = evnt.caxis.value;

	auto normalize = [](int value) {
		return static_cast<float>(value) / 32768.f;
	};

	switch (axis) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			analog_input.primary.x = normalize(value);
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			analog_input.primary.y = normalize(value);
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			analog_input.secondary.x = normalize(value);
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			analog_input.secondary.y = normalize(value);
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			analog_input.trigger_left = normalize(value);
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			analog_input.trigger_right = normalize(value);
			break;
	}
#endif
}

void Sdl2Ui::ProcessFingerEvent(SDL_Event& evnt) {
#if defined(USE_TOUCH) && defined(SUPPORT_TOUCH)
	int xw = viewport.w;
	int yh = viewport.h;

	if (xw == 0 || yh == 0) {
		// Startup. No viewport yet
		return;
	}

	// We currently ignore swipe gestures
	// A finger touch is detected when the fingers go up a brief delay after going down
	if (evnt.type == SDL_FINGERDOWN) {
		int finger = evnt.tfinger.fingerId;
		if (finger < static_cast<int>(finger_input.size())) {
			auto& fi = touch_input[finger];
			fi.position.x = (evnt.tfinger.x - viewport.x) * main_surface->width() / xw;
			fi.position.y = (evnt.tfinger.y - viewport.y) * main_surface->height() / yh;

#ifdef EMSCRIPTEN
			double display_ratio = emscripten_get_device_pixel_ratio();
			fi.position.x = (evnt.tfinger.x * display_ratio - viewport.x) * main_surface->width() / xw;
			fi.position.y = (evnt.tfinger.y * display_ratio - viewport.y) * main_surface->height() / yh;
#else
			fi.position.x = (evnt.tfinger.x - viewport.x) * main_surface->width() / xw;
			fi.position.y = (evnt.tfinger.y - viewport.y) * main_surface->height() / yh;
#endif

			fi.pressed = true;
		}
	} else if (evnt.type == SDL_FINGERUP) {
		int finger = evnt.tfinger.fingerId;
		if (finger < static_cast<int>(finger_input.size())) {
			auto& fi = touch_input[finger];
			fi.pressed = false;
		}
	}
#else
	/* unused */
	(void) evnt;
#endif
}

void Sdl2Ui::SetAppIcon() {
#if defined(__WINRT__) || defined(__MORPHOS__)
	// do nothing
#elif defined(_WIN32)
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

	if (icon == NULL || icon_small == NULL)
		Output::Warning("Could not load window icon.");

	window = wminfo.info.win.window;
	SetClassLongPtr(window, GCLP_HICON, (LONG_PTR) icon);
	SetClassLongPtr(window, GCLP_HICONSM, (LONG_PTR) icon_small);
#else
	#if defined(__APPLE__) && TARGET_OS_OSX
		if (MacOSUtils::IsAppBundle()) {
			// Do nothing if running as a .app. In this case macOS uses the
			// icon packaged with the .app's resources, which is a much higher
			// resolution version than the one that would be set below.
			return;
		}
	#endif
	/* SDL handles transfering the application icon to new or recreated windows,
	   if initially set through it (see below). So no need to set again for all
	   platforms relying on it. Platforms defined above need special treatment.
	*/
	static bool icon_set = false;

	if (icon_set)
		return;

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

	if (icon == NULL)
		Output::Warning("Could not load window icon.");

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
		case SDL_SCANCODE_KP_MULTIPLY	: return Input::Keys::KP_MULTIPLY;
		case SDL_SCANCODE_KP_PLUS		: return Input::Keys::KP_ADD;
		case SDL_SCANCODE_KP_ENTER		: return Input::Keys::RETURN;
		case SDL_SCANCODE_KP_MINUS		: return Input::Keys::KP_SUBTRACT;
		case SDL_SCANCODE_KP_PERIOD		: return Input::Keys::KP_PERIOD;
		case SDL_SCANCODE_KP_DIVIDE		: return Input::Keys::KP_DIVIDE;
		case SDL_SCANCODE_COMMA			: return Input::Keys::COMMA;
		case SDL_SCANCODE_PERIOD		: return Input::Keys::PERIOD;
		case SDL_SCANCODE_SLASH			: return Input::Keys::SLASH;
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
		case SDL_SCANCODE_LEFTBRACKET	: return Input::Keys::LEFT_BRACKET;
		case SDL_SCANCODE_RIGHTBRACKET	: return Input::Keys::RIGHT_BRACKET;
		case SDL_SCANCODE_BACKSLASH		: return Input::Keys::BACKSLASH;
		case SDL_SCANCODE_SEMICOLON		: return Input::Keys::SEMICOLON;
		case SDL_SCANCODE_APOSTROPHE	: return Input::Keys::APOSTROPH;

		default							: return Input::Keys::NONE;
	}
}
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
Input::Keys::InputKey SdlJKey2InputKey(int button_index) {
	// Constants starting from 15 require newer SDL2 versions
	switch (button_index) {
		case SDL_CONTROLLER_BUTTON_A: return Input::Keys::JOY_A;
		case SDL_CONTROLLER_BUTTON_B: return Input::Keys::JOY_B;
		case SDL_CONTROLLER_BUTTON_X: return Input::Keys::JOY_X;
		case SDL_CONTROLLER_BUTTON_Y: return Input::Keys::JOY_Y;
		case SDL_CONTROLLER_BUTTON_BACK: return Input::Keys::JOY_BACK;
		case SDL_CONTROLLER_BUTTON_GUIDE: return Input::Keys::JOY_GUIDE;
		case SDL_CONTROLLER_BUTTON_START: return Input::Keys::JOY_START;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: return Input::Keys::JOY_LSTICK;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return Input::Keys::JOY_RSTICK;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER	: return Input::Keys::JOY_SHOULDER_LEFT;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return Input::Keys::JOY_SHOULDER_RIGHT;
		case SDL_CONTROLLER_BUTTON_DPAD_UP: return Input::Keys::JOY_DPAD_UP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return Input::Keys::JOY_DPAD_DOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return Input::Keys::JOY_DPAD_LEFT;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return Input::Keys::JOY_DPAD_RIGHT;
		case 15	: return Input::Keys::JOY_OTHER_1; // SDL_CONTROLLER_BUTTON_MISC1 (2.0.14)
		case 16	: return Input::Keys::JOY_REAR_RIGHT_1; // SDL_CONTROLLER_BUTTON_PADDLE1 (2.0.14)
		case 17	: return Input::Keys::JOY_REAR_RIGHT_2; // SDL_CONTROLLER_BUTTON_PADDLE2 (2.0.14)
		case 18	: return Input::Keys::JOY_REAR_LEFT_1; // SDL_CONTROLLER_BUTTON_PADDLE3 (2.0.14)
		case 19	: return Input::Keys::JOY_REAR_LEFT_2; // SDL_CONTROLLER_BUTTON_PADDLE4 (2.0.14)
		case 20	: return Input::Keys::JOY_TOUCH; // SDL_CONTROLLER_BUTTON_TOUCHPAD (2.0.14)
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

void Sdl2Ui::vGetConfig(Game_ConfigVideo& cfg) const {
#ifdef EMSCRIPTEN
	cfg.renderer.Lock("SDL2 (Software, Emscripten)");
#elif defined(__WIIU__)
	cfg.renderer.Lock("SDL2 (Software, Wii U)");
#else
	cfg.renderer.Lock("SDL2 (Software)");
#endif

#if SDL_VERSION_ATLEAST(2, 0, 18)
	cfg.vsync.SetOptionVisible(true);
#endif
	cfg.fullscreen.SetOptionVisible(true);
	cfg.fps_limit.SetOptionVisible(true);
	cfg.fps_render_window.SetOptionVisible(true);
#if defined(SUPPORT_ZOOM) && !defined(__ANDROID__)
	// An initial zoom level is needed on Android however changing it looks awful
	cfg.window_zoom.SetOptionVisible(true);
#endif
	cfg.scaling_mode.SetOptionVisible(true);
	cfg.stretch.SetOptionVisible(true);
	cfg.game_resolution.SetOptionVisible(true);

	cfg.vsync.Set(current_display_mode.vsync);
	cfg.window_zoom.Set(current_display_mode.zoom);
	cfg.fullscreen.Set(IsFullscreen());

#ifdef EMSCRIPTEN
	// Fullscreen is handled by the browser
	cfg.fullscreen.SetOptionVisible(false);
	cfg.fps_limit.SetOptionVisible(false);
	cfg.fps_render_window.SetOptionVisible(false);
	cfg.window_zoom.SetOptionVisible(false);
	// Toggling this freezes the web player
	cfg.vsync.SetOptionVisible(false);
#elif defined(__WIIU__)
	// FIXME: Some options below may crash, better disable for now
	cfg.fullscreen.SetOptionVisible(false);
	cfg.window_zoom.SetOptionVisible(false);
	cfg.vsync.SetOptionVisible(false);
#endif
}

Rect Sdl2Ui::GetWindowMetrics() const {
	if (!IsFullscreen()) {
		Rect metrics;
		SDL_GetWindowSize(sdl_window, &metrics.width, &metrics.height);
		SDL_GetWindowPosition(sdl_window, &metrics.x, &metrics.y);
		return metrics;
	} else {
		return window_mode_metrics;
	}
}
