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

#ifndef EP_BASEUI_H
#define EP_BASEUI_H

// Headers
#include <string>
#include <bitset>

#include "system.h"
#include "color.h"
#include "font.h"
#include "point.h"
#include "rect.h"
#include "keys.h"
#include "game_config.h"
#include "game_clock.h"

#ifdef SUPPORT_AUDIO
	struct AudioInterface;
#endif

/**
 * BaseUi base abstract class.
 */
class BaseUi {
public:
	/**
	 * Virtual Destructor.
	 */
	virtual ~BaseUi() {}

	/**
	 * Creates a new BaseUi.
	 *
	 * @param width display client width.
	 * @param height display client height.
	 * @param cfg video config options
	 */
	static std::shared_ptr<BaseUi> CreateUi(long width, long height, const Game_ConfigVideo& cfg);

	/**
	 * Toggles fullscreen.
	 */
	virtual void ToggleFullscreen() = 0;

	/**
	 * Toggles zoom.
	 */
	virtual void ToggleZoom() = 0;

	/**
	 * Processes events queue.
	 */
	virtual void ProcessEvents() = 0;

	/**
	 * Cleans video buffer.
	 */
	void CleanDisplay();

	/**
	 * Updates video buffer.
	 */
	virtual void UpdateDisplay() = 0;

	/**
	 * Gets a copy of the display surface.
	 *
	 * @return bitmap a copy of the display surface.
	 */
	BitmapRef CaptureScreen();

	/**
	 * Sets display title.
	 *
	 * @param title title string.
	 */
	virtual void SetTitle(const std::string &title) = 0;

	/**
	 * Sets if the cursor should be shown.
	 *
	 * @param flag cursor visibility flag.
	 * @return previous state.
	 */
	virtual bool ShowCursor(bool flag) = 0;

	/**
	 * Gets if fullscreen mode is active.
	 *
	 * @return whether fullscreen mode is active.
	 */
	bool IsFullscreen() const;

#ifdef SUPPORT_AUDIO
	/**
	 * Returns audio instance.
	 *
	 * @return audio implementation.
	 */
	virtual AudioInterface& GetAudio() = 0;
#endif

	/**
	 * Gets client width size.
	 *
	 * @return client width size.
	 */
	long GetWidth() const;

	/**
	 * Gets client height size.
	 *
	 * @return client height size.
	 */
	long GetHeight() const;

	/**
	 * Gets whether mouse is hovering the display.
	 *
	 * @return whether mouse is hovering the display.
	 */
	bool GetMouseFocus() const;

	/**
	 * @return mouse position.
	 */
	Point GetMousePosition() const;

	BitmapRef const& GetDisplaySurface() const;
	BitmapRef& GetDisplaySurface();

	typedef std::bitset<Input::Keys::KEYS_COUNT> KeyStatus;

	/**
	 * Gets vector with the all keys pressed states.
	 *
	 * @returns vector with the all keys pressed states.
	 */
	KeyStatus& GetKeyStates();

	/** @return true if the display manages the framerate */
	bool IsFrameRateSynchronized() const;

	/** @return true if we should render the fps counter to the screen */
	bool RenderFps() const;

	/** @return true if we should render the fps counter to the title bar */
	bool ShowFpsOnTitle() const;

	/** Toggle whether we should show fps */
	void ToggleShowFps();

	/**
	 * @return the minimum amount of time each physical frame should take.
	 * If the UI manages time (i.e.) vsync, will return a 0 duration.
	 */
	Game_Clock::duration GetFrameLimit() const;

protected:
	/**
	 * Protected Constructor. Use CreateBaseUi instead.
	 */
	explicit BaseUi(const Game_ConfigVideo& cfg);

	void SetFrameRateSynchronized(bool value);
	void SetIsFullscreen(bool value);

	/**
	 * Display mode data struct.
	 */
	struct DisplayMode {
		int zoom = 0;
		int width = 0;
		int height = 0;
		uint32_t flags = 0;
		uint8_t bpp = 0;
		bool effective = false;
		bool vsync = false;
	};

	/** Current display mode. */
	DisplayMode current_display_mode;

	KeyStatus keys;

	/** Surface used for zoom. */
	BitmapRef main_surface;

	/** Mouse position on screen relative to the window. */
	Point mouse_pos;

	/** Color for display background. */
	Color back_color = Color{ 0, 0, 0, 255 };

	/** Mouse hovering the window flag. */
	bool mouse_focus = false;

	/** The frames per second limit */
	int fps_limit = Game_Clock::GetTargetGameFps();

	/** The amount of time each frame should take, based on fps limit */
	Game_Clock::duration frame_limit = Game_Clock::GetTargetGameTimeStep();

	/** Cursor visibility flag. */
	bool cursor_visible = false;

	/** Ui manages frame rate externally */
	bool external_frame_rate = false;

	/** Whether UI is currently fullscreen */
	bool is_fullscreen = false;

	/** Whether we will show fps counter the screen */
	bool show_fps = false;

	/** If we will render fps on the screen even in windowed mode */
	bool fps_render_window = false;

	/** How to scale the viewport when larger than 320x240 */
	ScalingMode scaling_mode = ScalingMode::Bilinear;
};

/** Global DisplayUi variable. */
extern std::shared_ptr<BaseUi> DisplayUi;

inline bool BaseUi::IsFrameRateSynchronized() const {
	return external_frame_rate;
}

inline void BaseUi::SetFrameRateSynchronized(bool value) {
	external_frame_rate = value;
}

inline bool BaseUi::IsFullscreen() const {
	return is_fullscreen;
}

inline void BaseUi::SetIsFullscreen(bool fs) {
	is_fullscreen = fs;
}

inline BaseUi::KeyStatus& BaseUi::GetKeyStates() {
	return keys;
}

inline BitmapRef const& BaseUi::GetDisplaySurface() const {
	return main_surface;
}

inline BitmapRef& BaseUi::GetDisplaySurface() {
	return main_surface;
}

inline long BaseUi::GetWidth() const {
	return current_display_mode.width;
}

inline long BaseUi::GetHeight() const {
	return current_display_mode.height;
}

inline bool BaseUi::GetMouseFocus() const {
	return mouse_focus;
}

inline Point BaseUi::GetMousePosition() const {
	return mouse_pos;
}

inline bool BaseUi::RenderFps() const {
	return show_fps && (IsFullscreen() || fps_render_window);
}

inline bool BaseUi::ShowFpsOnTitle() const {
	return show_fps;
}

inline void BaseUi::ToggleShowFps() {
	show_fps = !show_fps;
}

inline Game_Clock::duration BaseUi::GetFrameLimit() const {
	return IsFrameRateSynchronized() ? Game_Clock::duration(0) : frame_limit;
}

#endif
