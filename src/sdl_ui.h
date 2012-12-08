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
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

#include <boost/scoped_ptr.hpp>

extern "C" {
	union SDL_Event;
	struct SDL_Surface;
}

struct AudioInterface;

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

	uint32_t GetTicks() const;
	void Sleep(uint32_t time_milli);

	AudioInterface& GetAudio();

	//@}

	/// Get display surface.
	BitmapRef GetDisplaySurface();

private:
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

	bool zoom_available;
	bool toggle_fs_available;

	bool RequestVideoMode(int width, int height, bool fullscreen);

	/// Last display mode
	DisplayMode last_display_mode;

	/// Mode is being changing flag
	bool mode_changing;

	/// Main SDL window.
	SDL_Surface* main_window;

	boost::scoped_ptr<AudioInterface> audio_;
};

#endif
