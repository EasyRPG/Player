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

#ifndef _BASEUI_H_
#define _BASEUI_H_

// Headers
#include <string>
#include <bitset>

#include "system.h"
#include "color.h"
#include "font.h"
#include "rect.h"
#include "keys.h"

struct AudioInterface;

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
	 * @param title display title.
	 * @param fullscreen start in fullscreen flag.
	 * @param zoom start with zoom flag.
	 */
	static EASYRPG_SHARED_PTR<BaseUi> CreateUi(long width, long height, const std::string& title, bool fullscreen, bool zoom);

	/**
	 * Begins a display mode change.
	 */
	virtual void BeginDisplayModeChange() = 0;

	/**
	 * Ends a display mode change.
	 */
	virtual void EndDisplayModeChange() = 0;

	/**
	 * Resizes display.
	 *
	 * @param width display client width.
	 * @param height display client height.
	 */
	virtual void Resize(long width, long height) = 0;

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
	 * Begins screen capture. While this mode is active
	 * all drawing to screen will be applied to an internal
	 * Bitmap, that is returned by the EndScreenCapture
	 * method.
	 */
	virtual void BeginScreenCapture() = 0;

	/**
	 * Ends screen capture and get the drawn contents.
	 *
	 * @return bitmap with drawn contents.
	 */
	virtual BitmapRef EndScreenCapture() = 0;

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
	virtual bool IsFullscreen() = 0;

	/**
	 * Gets ticks in ms for time measurement.
	 *
	 * @return time in ms.
	 */
	virtual uint32_t GetTicks() const = 0;

	/**
	 * Sleeps some time.
	 *
	 * @param time_milli ms to sleep.
	 */
	virtual void Sleep(uint32_t time_milli) = 0;

	/**
	 * Returns audio instance.
	 *
	 * @return audio implementation.
	 */
	virtual AudioInterface& GetAudio() = 0;

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

	/**
	 * Gets background color.
	 *
	 * @return background color.
	 */
	Color const& GetBackcolor() const;

	/**
	 * Sets background color.
	 *
	 * @param color new background color.
	 */
	void SetBackcolor(const Color &color);

	BitmapRef const& GetDisplaySurface() const;
	BitmapRef& GetDisplaySurface();

	typedef std::bitset<Input::Keys::KEYS_COUNT> KeyStatus;

	/**
	 * Gets vector with the all keys pressed states.
	 *
	 * @returns vector with the all keys pressed states.
	 */
	KeyStatus& GetKeyStates();

protected:
	/**
	 * Protected Constructor. Use CreateBaseUi instead.
	 */
	BaseUi();

	/**
	 * Display mode data struct.
	 */
	struct DisplayMode {
		DisplayMode() : effective(false), zoom(false), width(0), height(0), bpp(0), flags(0) {}
		bool effective;
		bool zoom;
		int width;
		int height;
		uint8_t bpp;
		uint32_t flags;
	};

	/** Current display mode. */
	DisplayMode current_display_mode;

	KeyStatus keys;

	/** Surface used for zoom. */
	BitmapRef main_surface;

	/** Mouse hovering the window flag. */
	bool mouse_focus;

	/** Mouse x coordinate on screen relative to the window. */
	int mouse_x;

	/** Mouse y coordinate on screen relative to the window. */
	int mouse_y;

	/** Cursor visibility flag. */
	bool cursor_visible;

	/** Color for display background. */
	Color back_color;
};

/** Global DisplayUi variable. */
extern EASYRPG_SHARED_PTR<BaseUi> DisplayUi;

#endif
