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

#ifndef EP_WINDOW_SHOPSTATUS_H
#define EP_WINDOW_SHOPSTATUS_H

// Headers
#include <string>
#include "window_base.h"

/**
 * Window ShopStatus Class.
 * Displays possessed and equipped items.
 */
class Window_ShopStatus : public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_ShopStatus(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Renders the current total on the window.
	 */
	void Refresh();

	/**
	 * Sets the item to display.
	 *
	 * @param item_id ID of item to use.
	 */
	void SetItemId(int item_id);

protected:
	int item_id;
};

#endif
