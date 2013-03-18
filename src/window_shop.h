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

#ifndef _WINDOW_SHOP_H_
#define _WINDOW_SHOP_H_

// Headers
#include <string>
#include "window_base.h"

/**
 * Window Shop Class.
 */
class Window_Shop :	public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_Shop(int ix, int iy, int iwidth, int iheight);

	/**
	 * Renders the current shop on the window.
	 */
	void Refresh();

	/**
	 * Updates the window.
	 */
	void Update();

	void SetMode(int nmode);
	int GetChoice() const;
	void SetChoice(int nchoice);

protected:
	void UpdateCursorRect();

	std::string greeting;
	std::string regreeting;
	std::string buy_msg;
	std::string sell_msg;
	std::string leave_msg;
	std::string buy_select;
	std::string buy_number;
	std::string purchased;
	std::string sell_select;
	std::string sell_number;
	std::string sold_msg;
	int index;
	int mode;
	int buy_index;
	int sell_index;
	int leave_index;
	int choice;
};

#endif
