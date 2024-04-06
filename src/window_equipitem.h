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

#ifndef EP_WINDOW_EQUIPITEM_H
#define EP_WINDOW_EQUIPITEM_H

// Headers
#include "window_item.h"

/**
 * Window_EquipItem class.
 * Displays the available equipment of a specific type.
 */
class Window_EquipItem : public Window_Item {

public:
	/** Enum containing the different equipment types. */
	enum EquipType {
		weapon = 0,
		shield,
		armor,
		helmet,
		other
	};

	/**
	 * Constructor.
	 *
	 * @param actor_id actor whos equipment is displayed.
	 * @param equip_type type of equipment to show.
	 */
	Window_EquipItem(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id, int equip_type);

	/**
	 * Checks if the item should be in the list based on
	 * the type.
	 *
	 * @param item_id item to check.
	 */
	bool CheckInclude(int item_id) override;

	/**
	 * Chechs if item should be enabled. Always true.
	 *
	 * @param item_id item to check.
	 */
	bool CheckEnable(int item_id) override;

private:
	int actor_id;
	int equip_type;
};

#endif
