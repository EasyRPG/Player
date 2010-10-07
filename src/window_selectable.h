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

#ifndef _WINDOW_SELECTABLE_H_
#define _WINDOW_SELECTABLE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "window_base.h"
//#include "window_help.h"

////////////////////////////////////////////////////////////
/// Window Selectable class
////////////////////////////////////////////////////////////
class Window_Selectable: public Window_Base {
public:
	Window_Selectable(int ix, int iy, int iwidth, int iheight);
	~Window_Selectable();

	int GetIndex() const;
	void SetIndex(int nindex);
	int GetRowMax() const;
	int GetTopRow() const;
	void SetTopRow(int row);
	int GetPageRowMax() const;
	int GetPageItemMax();
	//Window_Help* GetHelpWindow();
	//void SetHelpWindow(Window_Help* nhelp_window);
	void UpdateCursorRect();
	void Update();

	//virtual void UpdateHelp() = 0;

protected:
	int item_max;
	int column_max;
	int index;
	//Window_Help* window_help;
};

#endif
