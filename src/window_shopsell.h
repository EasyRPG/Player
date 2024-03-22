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

#ifndef EP_WINDOW_SHOPSELL_H
#define EP_WINDOW_SHOPSELL_H

// Headers
#include <vector>
#include "window_item.h"

/**
 * Window ShopSell class.
 * Displays all items of the party and allows to sell them.
 */
class Window_ShopSell : public Window_Item {

public:
	/**
	 * Constructor.
	 */
	Window_ShopSell(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Chechs if item should be enabled.
	 *
	 * @param item_id item to check.
	 */
	bool CheckEnable(int item_id) override;
};

#endif
