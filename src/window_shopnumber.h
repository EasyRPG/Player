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

#ifndef EP_WINDOW_SHOPNUMBER_H
#define EP_WINDOW_SHOPNUMBER_H

// Headers
#include <string>
#include "window_base.h"

/**
 * Window Shop Number Class.
 * The number input window for the shop.
 */
class Window_ShopNumber : public Window_Base {
public:
	/**
	 * Constructor.
	 *
	 * @param ix window x position.
	 * @param iy window y position.
	 * @param iwidth window width.
	 * @param iheight window height.
	 */
	Window_ShopNumber(int ix, int iy, int iwidth, int iheight);
	
	/**
	 * Updates the Windows contents.
	 */
	void Refresh();

	/**
	 * Updates number value according to user input.
	 */
	void Update() override;
	
	/**
	 * Returns the number value.
	 *
	 * @return the currently input number.
	 */
	int GetNumber() const;

	/**
	 * Sets all data needed for the window.
	 *
	 * @param item_id item to buy.
	 * @param item_max item maximum quantity.
	 * @param price Price of the item.
	 * @return the currently input number.
	 */
	void SetData(int item_id, int item_max, int price);

	/**
	 * Returns the total costs.
	 *
	 * @return total costs to buy the item.
	 */
	int GetTotal() const;

protected:
	int item_max;
	int price;
	int number;
	int item_id;
};

#endif
