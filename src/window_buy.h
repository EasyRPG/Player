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

#ifndef _WINDOW_BUY_H_
#define _WINDOW_BUY_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "window_base.h"
#include "window_help.h"
#include "window_total.h"

////////////////////////////////////////////////////////////
/// Window Shop Class
/// The shop item list window
////////////////////////////////////////////////////////////
// Window_Selectable is too badly broken to use as a base class
class Window_Buy : public Window_Base {
public:
	////////////////////////////////////////////////////////
	/// Constructor
	/// @param idigits_max : The maximum number of digits 
	///	allowed
	////////////////////////////////////////////////////////
	Window_Buy(int ix, int iy, int iwidth = 320, int iheight = 80);
	
	////////////////////////////////////////////////////////
	/// Destructor
	////////////////////////////////////////////////////////
	~Window_Buy();

	void CreateContents();

	virtual void UpdateCursorRect();
	Rect GetItemRect(int index);
	void Update();
	void Refresh();
	int GetSelected(void);
	void SetHelpWindow(Window_Help* w);
	void SetTotalWindow(Window_Total* w);
	
protected:
	static const int border_x = 8;
	static const int border_y = 4;
	static const int row_spacing = 16;
	Window_Help* help_window;
	Window_Total* total_window;
	int row_max;
	int top_index;
	int index;
};

#endif
