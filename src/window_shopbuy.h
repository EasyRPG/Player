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

#ifndef EP_WINDOW_SHOPBUY_H
#define EP_WINDOW_SHOPBUY_H

// Headers
#include <vector>
#include "window_selectable.h"
#include <player.h>

/**
 * Window Shop Buy Class
 * The shop item list window.
 */
class Window_ShopBuy : public Window_Selectable {
public:
	/**
	 * Constructor.
	 */
	Window_ShopBuy(Scene* parent, const std::vector<int>& goods,
			int ix, int iy, int iwidth = Player::screen_width, int iheight = 80);

	/**
	 * Gets item ID of the selected item.
	 *
	 * @return current selected item ID.
	 */
	int GetItemId();

	/**
	 * Refreshes the item list.
	 */
	void Refresh();

	/**
	 * Draws an item together with the price.
	 *
	 * @param index index of item to draw.
	 */
	void DrawItem(int index);

	/**
	 * Updates the help window.
	 */
	void UpdateHelp() override;

	/**
	 * Checks if the item should be enabled.
	 *
	 * @param item_id item id to check.
	 * @return true if it is enabled.
	 */
	bool CheckEnable(int item_id);

protected:
	const std::vector<int>& data;
};

#endif
