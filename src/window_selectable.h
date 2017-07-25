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

#ifndef EP_WINDOW_SELECTABLE_H
#define EP_WINDOW_SELECTABLE_H

// Headers
#include "window_base.h"
#include "window_help.h"

/**
 * Window Selectable class.
 */
class Window_Selectable: public Window_Base {
public:
	Window_Selectable(int ix, int iy, int iwidth, int iheight);

	/**
	 * Creates the contents based on how many items
	 * are currently in the window.
	 */
	void CreateContents();

	int GetIndex() const;
	void SetIndex(int nindex);
	int GetRowMax() const;
	int GetTopRow() const;
	void SetTopRow(int row);
	int GetPageRowMax() const;
	int GetPageItemMax();

	/**
	 * Returns the Item Rect used for item drawing.
	 *
	 * @param index index of item.
	 * @return Rect where the item is drawn.
	 */
	Rect GetItemRect(int index);

	Window_Help* GetHelpWindow();

	/**
	 * Assigns a help window that displays a description
	 * about the selected item.
	 *
	 * @param nhelp_window the help window.
	 */
	void SetHelpWindow(Window_Help* nhelp_window);
	virtual void UpdateCursorRect();
	void Update() override;

	virtual void UpdateHelp();

protected:
	int item_max;
	int column_max;
	int index;
	Window_Help* help_window;
};

#endif
