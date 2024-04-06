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
#include <functional>
#include "window_base.h"
#include "window_help.h"

/**
 * Window Selectable class.
 */
class Window_Selectable: public Window_Base {
public:
	Window_Selectable(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Creates the contents based on how many items
	 * are currently in the window.
	 */
	void CreateContents();

	int GetItemMax() const;
	int GetIndex() const;
	void SetIndex(int nindex);
	int GetColumnMax() const;
	void SetColumnMax(int ncolmax);
	int GetRowMax() const;
	int GetTopRow() const;
	void SetTopRow(int row);
	int GetPageRowMax() const;
	int GetPageItemMax();

	/** Change the max item */
	void SetItemMax(int value);

	/**
	 * Returns the Item Rect used for item drawing.
	 *
	 * @param index index of item.
	 * @return Rect where the item is drawn.
	 */
	virtual Rect GetItemRect(int index);

	/**
	 * Function called by the base UpdateHelp() implementation.
	 * Passes in the Help Window and the current selected index
	 * Will not be called if the help_window is null
	 */
	std::function<void(Window_Help&, int)> UpdateHelpFn;

	Window_Help* GetHelpWindow();

	/**
	 * Assigns a help window that displays a description
	 * about the selected item.
	 *
	 * @param nhelp_window the help window.
	 */
	void SetHelpWindow(Window_Help* nhelp_window);


	/**
	 * Returns a rectangle indicating the cursor location for the passed index.
	 *
	 * @param index cursor index
	 * @return cursor rectangle
	 */
	virtual Rect GetCursorRect(int index) const;
	virtual void UpdateCursorRect();
	void Update() override;

	virtual void UpdateHelp();

	/**
	 * Returns the index of the item that is at the passed position or -1 if there is nothing.
	 *
	 * @param position Position to check. Relative to the content.
	 * @return index of the item or -1 if nothing was found.
	 */
	virtual int CursorHitTest(Point position) const;

	/**
	 * Sets if endless scrolling is enabled.
	 *
	 * @param state true to enable (default), false to disable.
	 */
	void SetEndlessScrolling(bool state);

	/**
	 * Sets the menu item height.
	 *
	 * @param height the menu item height.
	 */
	void SetMenuItemHeight(int height);

	/**
	 * Allow left/right input to move cursor up/down when the selectable has only one column.
	 * By default this behaviour is only enabled for two and more columns.
	 *
	 * @param wrap enable/disable single column wrap
	 */
	void SetSingleColumnWrapping(bool wrap);
	void SetMouseOldIndex(int i);
	int GetMouseOldIndex();

protected:
	void UpdateArrows();

	Window_Help* help_window = nullptr;
	int item_max = 1;
	int column_max = 1;
	int index = -1;
	int arrow_frame = 0;

	bool endless_scrolling = true;

	int menu_item_height = 16;

	int scroll_dir = 0;
	int scroll_progress = 0;

	int wrap_limit = 2;
	int mouseTimeArrow;
	int startCursorY = 0;
	int mouseOldIndex = 0;
};

inline void	Window_Selectable::SetItemMax(int value) {
	item_max = value;
}

#endif
