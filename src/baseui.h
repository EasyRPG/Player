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
#include "rect.h"
#include "keys.h"

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
	 * @param fullscreen start in fullscreen flag.
	 * @param zoom initial magnification factor.
	 */
	static std::shared_ptr<BaseUi> CreateUi(long width, long height, bool fullscreen, int zoom);

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
	 * Gets mouse x coordinate.
	 *
	 * @return mouse x coordinate.
	 */
	int GetMousePosX() const;

	/**
	 * Gets mouse y coordinate.
	 *
	 * @return mouse y coordinate.
	 */
	int GetMousePosY() const;

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

protected:
	/**
	 * Protected Constructor. Use CreateBaseUi instead.
	 */
	BaseUi();

	void SetFrameRateSynchronized(bool value);
	void SetIsFullscreen(bool value);

	/**
	 * Display mode data struct.
	 */
	struct DisplayMode {
		bool effective = false;
		int zoom = 0;
		int width = 0;
		int height = 0;
		uint8_t bpp = 0;
		uint32_t flags = 0;
	};

	/** Current display mode. */
	DisplayMode current_display_mode;

	KeyStatus keys;

	/** Surface used for zoom. */
	BitmapRef main_surface;

	/** Mouse x coordinate on screen relative to the window. */
	int mouse_x = 0;

	/** Mouse y coordinate on screen relative to the window. */
	int mouse_y = 0;

	/** Color for display background. */
	Color back_color = Color{ 0, 0, 0, 255 };

	/** Mouse hovering the window flag. */
	bool mouse_focus = false;

	/** Cursor visibility flag. */
	bool cursor_visible = false;

	/** Ui manages frame rate externally */
	bool external_frame_rate = false;

	/** Whether UI is currently fullscreen */
	bool is_fullscreen = false;
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

inline int BaseUi::GetMousePosX() const {
	return mouse_x;
}

inline int BaseUi::GetMousePosY() const {
	return mouse_y;
}

#endif
