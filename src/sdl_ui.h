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
	SdlUi(long width, long height, const std::string title,	bool fullscreen, bool zoom);

	///////////////////////////////////////////////////////
	/// Destructor.
	///////////////////////////////////////////////////////
	~SdlUi();
	
	///////////////////////////////////////////////////////
	/// Inherited from BaseUi.
	///////////////////////////////////////////////////////
	//@{

	void StartDisplayModeChange();
	void EndDisplayModeChange();
	void Resize(long width, long height);
	void ToggleFullscreen();
	void ToggleZoom();
	void CleanDisplay();
	void UpdateDisplay();
	void SetTitle(const std::string title);
	void DrawScreenText(const std::string& text);
	bool ShowCursor(bool flag);
	void SetBackcolor(const Color& color);

	void ProcessEvents();

	bool IsFullscreen();
	long GetWidth();
	long GetHeight();
	std::vector<bool> &GetKeyStates();

	bool GetMouseFocus();
	int GetMousePosX();
	int GetMousePosY();

	//@}

	/// Get display surface.
	SDL_Surface* GetDisplaySurface();

protected:
	///////////////////////////////////////////////////////
	/// Refresh the display mode after it was changed.
	/// @returns whether the change was successful
	///////////////////////////////////////////////////////
	bool RefreshDisplayMode();

	///////////////////////////////////////////////////////
	/// Process a single SDL Event.
	///////////////////////////////////////////////////////
	void ProcessEvent(SDL_Event &sdl_event);

	///////////////////////////////////////////////////////
	/// Blit a surface scaled x2 to another surface.
	/// @param src : source surface
	/// @param dst : destination surface
	///////////////////////////////////////////////////////
	void Blit2X(SDL_Surface* src, SDL_Surface* dst);

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
		DisplayMode() : effective(false), fullscreen(false), zoom(false), width(0), height(0) {}
		bool effective;
		bool fullscreen;
		bool zoom;
		int width;
		int height;
	};

	/// Current display mode
	DisplayMode current_display_mode;

	/// Last display mode
	DisplayMode last_display_mode;

	/// Mode is being changing flag
	bool mode_changing;

	/// Main SDL window.
	SDL_Surface* main_window;

	/// Surface used for zoom.
	SDL_Surface* main_surface;

	/// Color for display background
	uint32 back_color;

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
extern SdlUi* DisplaySdlUi;

#endif
