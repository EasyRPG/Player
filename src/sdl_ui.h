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

#ifndef _SDL_UI_H_
#define _SDL_UI_H_

///////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////
#include "SDL.h"
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

///////////////////////////////////////////////////////////
/// SdlUi class.
///////////////////////////////////////////////////////////
class SdlUi : public BaseUi {
public:
	///////////////////////////////////////////////////////
	/// Constructor.
	/// @param width : window client width
	/// @param height : window client height
	/// @param title : window title
	/// @param fullscreen : start in fullscreen flag
	///////////////////////////////////////////////////////
	SdlUi(long width, long height, const std::string& title,	bool fullscreen);

	///////////////////////////////////////////////////////
	/// Destructor.
	///////////////////////////////////////////////////////
	~SdlUi();

	///////////////////////////////////////////////////////
	/// Inherited from BaseUi.
	///////////////////////////////////////////////////////
	//@{

	void BeginDisplayModeChange();
	void EndDisplayModeChange();
	void Resize(long width, long height);
	void ToggleFullscreen();
	void ToggleZoom();
	void CleanDisplay();
	void UpdateDisplay();
	void BeginScreenCapture();
	BitmapRef EndScreenCapture();
	void SetTitle(const std::string &title);
	void DrawScreenText(const std::string &text);
	void DrawScreenText(const std::string &text, int x, int y, Color const& color = Color(255, 255, 255, 255));
	void DrawScreenText(const std::string &text, Rect const& dst_rect, Color const& color = Color(255, 255, 255, 255));
	bool ShowCursor(bool flag);

	void ProcessEvents();

	bool IsFullscreen();
	long GetWidth();
	long GetHeight();
	std::vector<bool> &GetKeyStates();

	bool GetMouseFocus();
	int GetMousePosX();
	int GetMousePosY();

	Color GetBackcolor();
	void SetBackcolor(const Color &color);

	//@}

	/// Get display surface.
	BitmapRef GetDisplaySurface();

protected:
	///////////////////////////////////////////////////////
	/// Refresh the display mode after it was changed.
	/// @returns whether the change was successful
	///////////////////////////////////////////////////////
	bool RefreshDisplayMode();

	///////////////////////////////////////////////////////
	/// Process a SDL Event.
	///////////////////////////////////////////////////////
	//@{

	void ProcessEvent(SDL_Event &sdl_event);

	void ProcessActiveEvent(SDL_Event &evnt);
	void ProcessKeyDownEvent(SDL_Event &evnt);
	void ProcessKeyUpEvent(SDL_Event &evnt);
	void ProcessMouseMotionEvent(SDL_Event &evnt);
	void ProcessMouseButtonEvent(SDL_Event &evnt);
	void ProcessJoystickButtonEvent(SDL_Event &evnt);
	void ProcessJoystickHatEvent(SDL_Event &evnt);
	void ProcessJoystickAxisEvent(SDL_Event &evnt);

	//@}

	///////////////////////////////////////////////////////
	/// Blit a bitmap scaled x2 to an SDL surface.
	/// @param src : source bitmap
	/// @param dst : destination surface
	///////////////////////////////////////////////////////
	void Blit2X(Bitmap const& src, SDL_Surface* dst);

	///////////////////////////////////////////////////////
	/// Set app icon.
	///////////////////////////////////////////////////////
	void SetAppIcon();

	///////////////////////////////////////////////////////
	/// Reset keys states.
	///////////////////////////////////////////////////////
	void ResetKeys();

	/// Display mode data struct
	struct DisplayMode {
		DisplayMode() : effective(false), zoom(false), width(0), height(0), bpp(0), flags(0) {}
		bool effective;
		bool zoom;
		int width;
		int height;
		uint8_t bpp;
		uint32_t flags;
	};

	bool zoom_available;
	bool toggle_fs_available;

	bool RequestVideoMode(int width, int height, bool fullscreen);

	/// Current display mode
	DisplayMode current_display_mode;

	/// Last display mode
	DisplayMode last_display_mode;

	/// Mode is being changing flag
	bool mode_changing;

	/// Main SDL window.
	SDL_Surface* main_window;

	/// Surface used for zoom.
	BitmapRef main_surface;

	/// Color for display background
	uint32_t back_color;

	/// Keys states flags.
	std::vector<bool> keys;

	/// Mouse hovering the window flag.
	bool mouse_focus;

	/// Mouse x coordinate on screen relative to the window.
	int mouse_x;

	/// Mouse y coordinate on screen relative to the window.
	int mouse_y;

	/// Cursor visibility flag
	bool cursor_visible;
};

/// Global SdlUi variable.
extern EASYRPG_SHARED_PTR<SdlUi> DisplaySdlUi;

#endif
