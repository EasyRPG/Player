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

#ifndef EP_SDL2_UI_H
#define EP_SDL2_UI_H

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

#include <array>
#include <SDL.h>

extern "C" {
	union SDL_Event;
	struct SDL_Texture;
	struct SDL_Window;
	struct SDL_Renderer;
}

struct AudioInterface;

/**
 * Sdl2Ui class.
 */
class Sdl2Ui final : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param cfg config options
	 */
	Sdl2Ui(long width, long height, const Game_Config& cfg);

	/**
	 * Destructor.
	 */
	~Sdl2Ui() override;

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */
	bool vChangeDisplaySurfaceResolution(int new_width, int new_height) override;
	void ToggleFullscreen() override;
	void ToggleZoom() override;
	void UpdateDisplay() override;
	void SetTitle(const std::string &title) override;
	bool ShowCursor(bool flag) override;
	void ProcessEvents() override;
	void SetScalingMode(ScalingMode) override;
	void ToggleStretch() override;
	void ToggleVsync() override;
	void vGetConfig(Game_ConfigVideo& cfg) const override;
	Rect GetWindowMetrics() const override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio() override;
#endif

	/** @} */

private:
	/**
	 * Refreshes the display mode after it was changed.
	 *
	 * @return whether the change was successful.
	 */
	bool RefreshDisplayMode();

	void BeginDisplayModeChange();
	void EndDisplayModeChange();

	/**
	 * Processes a SDL Event.
	 */
	/** @{ */

	void ProcessEvent(SDL_Event &sdl_event);

	void ProcessWindowEvent(SDL_Event &evnt);
	void ProcessKeyDownEvent(SDL_Event &evnt);
	void ProcessKeyUpEvent(SDL_Event &evnt);
	void ProcessMouseMotionEvent(SDL_Event &evnt);
	void ProcessMouseButtonEvent(SDL_Event &evnt);
	void ProcessMouseWheelEvent(SDL_Event &evnt);
	void ProcessControllerAdded(SDL_Event &evnt);
	void ProcessControllerRemoved(SDL_Event &evnt);
	void ProcessControllerButtonEvent(SDL_Event &evnt);
	void ProcessControllerAxisEvent(SDL_Event &evnt);
	void ProcessFingerEvent(SDL_Event & evnt);

	/** @} */

	/**
	 * Sets app icon.
	 */
	void SetAppIcon();

	/**
	 * Resets keys states.
	 */
	void ResetKeys();

	void RequestVideoMode(int width, int height, int zoom, bool fullscreen, bool vsync);

	/** Last display mode. */
	DisplayMode last_display_mode;

	/** Main SDL window. */
	SDL_Texture* sdl_texture_game = nullptr;
	SDL_Texture* sdl_texture_scaled = nullptr;
	SDL_Window* sdl_window = nullptr;
	SDL_Renderer* sdl_renderer = nullptr;
	SDL_Joystick *sdl_joystick = nullptr;

	Rect window_mode_metrics;
	SDL_Rect viewport = {};
	struct {
		int width = 0;
		int height = 0;
		bool size_changed = true;
		float scale = 0.f;
	} window = {};

	uint32_t texture_format = SDL_PIXELFORMAT_UNKNOWN;

	std::unique_ptr<AudioInterface> audio_;
};

#endif
