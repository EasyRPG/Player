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

#ifndef EP_WINDOW_EQUIP_H
#define EP_WINDOW_EQUIP_H

// Headers
#include "window_selectable.h"

/**
 * Window_Equip class.
 * Displays currently equipped items.
 */
class Window_Equip : public Window_Selectable {
public:
	/**
	 * Constructor.
	 *
	 * @param ix window x position.
	 * @param iy window y position.
	 * @param iwidth window width.
	 * @param iheight window height.
	 * @param actor actor whose inventory is displayed.
	 */
	Window_Equip(int ix, int iy, int iwidth, int iheight, const Game_Actor& actor);

	/**
	 * Refreshes.
	 */
	void Refresh();

	/**
	 * Returns the item ID of the selected item.
	 *
	 * @return item ID.
	 */
	int GetItemId();

	/**
	 * Updates the help text.
	 */
	void UpdateHelp() override;

private:
	const Game_Actor& actor;
	std::vector<int> data;

};

#endif
