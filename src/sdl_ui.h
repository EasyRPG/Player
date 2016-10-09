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

#ifndef _SDL_UI_H_
#define _SDL_UI_H_

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

#include <SDL.h>

extern "C" {
	union SDL_Event;
	struct SDL_Surface;
#if SDL_MAJOR_VERSION > 1
	struct SDL_Texture;
	struct SDL_Window;
	struct SDL_Renderer;
#endif
}

struct AudioInterface;

/**
 * SdlUi class.
 */
class SdlUi : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param fullscreen start in fullscreen flag.
	 */
	SdlUi(long width, long height, bool fullscreen);

	/**
	 * Destructor.
	 */
	~SdlUi() override;

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */

	void BeginDisplayModeChange() override;
	void EndDisplayModeChange() override;
	void Resize(long width, long height) override;
	void ToggleFullscreen() override;
	void ToggleZoom() override;
	void UpdateDisplay() override;
	void SetTitle(const std::string &title) override;
	bool ShowCursor(bool flag) override;

	void ProcessEvents() override;

	bool IsFullscreen() override;

	uint32_t GetTicks() const override;
	void Sleep(uint32_t time_milli) override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio() override;
#endif

	/** @} */

	/** Get display surface. */
	BitmapRef GetDisplaySurface();

private:
	/**
	 * Refreshes the display mode after it was changed.
	 *
	 * @return whether the change was successful.
	 */
	bool RefreshDisplayMode();

	/**
	 * Processes a SDL Event.
	 */
	/** @{ */

	void ProcessEvent(SDL_Event &sdl_event);

	void ProcessActiveEvent(SDL_Event &evnt);
	void ProcessKeyDownEvent(SDL_Event &evnt);
	void ProcessKeyUpEvent(SDL_Event &evnt);
	void ProcessMouseMotionEvent(SDL_Event &evnt);
	void ProcessMouseButtonEvent(SDL_Event &evnt);
	void ProcessJoystickButtonEvent(SDL_Event &evnt);
	void ProcessJoystickHatEvent(SDL_Event &evnt);
	void ProcessJoystickAxisEvent(SDL_Event &evnt);
#if SDL_MAJOR_VERSION>1
	void ProcessMouseWheelEvent(SDL_Event &evnt);
	void ProcessFingerEvent(SDL_Event & evnt);

	/** @} */

	/**
	 * Sets app icon.
	 */
	void SetAppIcon();
#endif

	/**
	 * Blits a bitmap scaled x2 to an SDL surface.
	 *
	 * @param src source bitmap.
	 * @param dst destination surface.
	 */
	void Blit2X(Bitmap const& src, SDL_Surface* dst);

	/**
	 * Resets keys states.
	 */
	void ResetKeys();

	bool zoom_available;
	bool toggle_fs_available;

	bool RequestVideoMode(int width, int height, bool fullscreen);

	/** Last display mode. */
	DisplayMode last_display_mode;

	/** Mode is being changing flag */
	bool mode_changing;

	/** Main SDL window. */
#if SDL_MAJOR_VERSION==1
	SDL_Surface* sdl_surface;
#else
	SDL_Texture* sdl_texture;
	SDL_Window* sdl_window;
	SDL_Renderer* sdl_renderer;
#endif

	std::unique_ptr<AudioInterface> audio_;
};

#endif
