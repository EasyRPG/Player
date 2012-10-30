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

#ifndef _BASEUI_H_
#define _BASEUI_H_

///////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "color.h"
#include "rect.h"

///////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////
class Color;
class Bitmap;
class Surface;

///////////////////////////////////////////////////////////
/// BaseUi base abstract class.
///////////////////////////////////////////////////////////
class BaseUi {
public:
	///////////////////////////////////////////////////////
	/// Virtual Destructor.
	///////////////////////////////////////////////////////
	virtual ~BaseUi() {}

	///////////////////////////////////////////////////////
	/// Creates a new BaseUi .
	/// @param width : display client width
	/// @param height : display client height
	/// @param title : display title
	/// @param fullscreen : start in fullscreen flag
	///////////////////////////////////////////////////////
	static BaseUi* CreateBaseUi(long width, long height, const std::string& title, bool fullscreen, bool zoom);

	///////////////////////////////////////////////////////
	/// Begins a display mode change.
	///////////////////////////////////////////////////////
	virtual void BeginDisplayModeChange() = 0;

	///////////////////////////////////////////////////////
	/// Ends a display mode change.
	///////////////////////////////////////////////////////
	virtual void EndDisplayModeChange() = 0;

	///////////////////////////////////////////////////////
	/// Resize display.
	/// @param width : display client width
	/// @param height : display client height
	///////////////////////////////////////////////////////
	virtual void Resize(long width, long height) = 0;

	///////////////////////////////////////////////////////
	/// Toggle fullscreen.
	///////////////////////////////////////////////////////
	virtual void ToggleFullscreen() = 0;

	///////////////////////////////////////////////////////
	/// Toggle zoom.
	///////////////////////////////////////////////////////
	virtual void ToggleZoom() = 0;

	///////////////////////////////////////////////////////
	/// Process events queue.
	///////////////////////////////////////////////////////
	virtual void ProcessEvents() = 0;

	///////////////////////////////////////////////////////
	/// Clean video buffer.
	///////////////////////////////////////////////////////
	virtual void CleanDisplay() = 0;

	///////////////////////////////////////////////////////
	/// Update video buffer.
	///////////////////////////////////////////////////////
	virtual void UpdateDisplay() = 0;

	///////////////////////////////////////////////////////
	/// Begins screen capture. While this mode is active
	/// all drawing to screen will be applied to an internal
	/// Bitmap, that is returned by the EndScreenCapture
	/// method.
	///////////////////////////////////////////////////////
	virtual void BeginScreenCapture() = 0;

	///////////////////////////////////////////////////////
	/// Ends screen capture and get the drawn contents.
	/// @return bitmap with drawn contents
	///////////////////////////////////////////////////////
	virtual Bitmap* EndScreenCapture() = 0;

	///////////////////////////////////////////////////////
	/// Set display title.
	/// @param title : title string
	///////////////////////////////////////////////////////
	virtual void SetTitle(const std::string &title) = 0;

	///////////////////////////////////////////////////////
	/// Display white text in the top left corner of the screen.
	/// Used by the FPS-Display.
	/// @param text : text to display
	///////////////////////////////////////////////////////
	virtual void DrawScreenText(const std::string &text) = 0;

	///////////////////////////////////////////////////////
	/// Display text on the screen.
	/// @param text : text to display
	/// @param x : X-coordinate where text is displayed
	/// @param y : Y-coordinate where text is displayed
	/// @param color : Text color
	///////////////////////////////////////////////////////
	virtual void DrawScreenText(const std::string &text, int x, int y, Color color = Color(255, 255, 255, 255)) = 0;

	///////////////////////////////////////////////////////
	/// Display text on the screen.
	/// @param text : text to display
	/// @param dst_rect : Rect where text is displayed
	/// @param color : Text color
	///////////////////////////////////////////////////////
	virtual void DrawScreenText(const std::string &text, Rect dst_rect, Color color = Color(255, 255, 255, 255)) = 0;

	///////////////////////////////////////////////////////
	/// Set if the cursor should be showed.
	/// @param flag: cursor visibility flag
	/// @return previous state
	///////////////////////////////////////////////////////
	virtual bool ShowCursor(bool flag) = 0;

	///////////////////////////////////////////////////////
	/// Get if fullscreen mode is active.
	/// @returns whether fullscreen mode is active.
	///////////////////////////////////////////////////////
	virtual bool IsFullscreen() = 0;

	/// @returns client width size
	virtual long GetWidth() = 0;

	/// @returns client height size
	virtual long GetHeight() = 0;

	/// @returns vector with the all keys pressed states
	virtual std::vector<bool> &GetKeyStates() = 0;

	/// @returns whether mouse is hovering the display
	virtual bool GetMouseFocus() = 0;

	/// @returns mouse x coordinate
	virtual int GetMousePosX() = 0;

	/// @returns mouse y coordinate
	virtual int GetMousePosY() = 0;

	/// @return background color
	virtual Color GetBackcolor() = 0;

	/// @param color : new background color
	virtual void SetBackcolor(const Color &color) = 0;

	virtual Surface* GetDisplaySurface() = 0;

protected:
	///////////////////////////////////////////////////////
	/// Protected Constructor. Use CreateBaseUi instead.
	///////////////////////////////////////////////////////
	BaseUi() {}
};

/// Global DisplayUi variable.
extern BaseUi* DisplayUi;

#endif
