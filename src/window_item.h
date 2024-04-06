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

#ifndef EP_WINDOW_ITEM_H
#define EP_WINDOW_ITEM_H

// Headers
#include <vector>
#include "window_help.h"
#include "window_selectable.h"

/**
 * Window_Item class.
 */
class Window_Item : public Window_Selectable {

public:
	/**
	 * Constructor.
	 */
	Window_Item(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Gets item.
	 *
	 * @return current selected item.
	 */
	const lcf::rpg::Item* GetItem() const;

	/**
	 * Checks if the item should be in the list.
	 *
	 * @param item_id item to check.
	 */
	virtual bool CheckInclude(int item_id);

	/**
	 * Checks if item should be enabled.
	 *
	 * @param item_id item to check.
	 */
	virtual bool CheckEnable(int item_id);

	/**
	 * Refreshes the item list.
	 */
	void Refresh();

	/**
	 * Draws an item together with the quantity.
	 *
	 * @param index index of item to draw.
	 */
	void DrawItem(int index);

	/**
	 * Updates the help window.
	 */
	void UpdateHelp() override;

	/**
	 * Assigns an actor to the item list.
	 * All equipped skill items will be added.
	 */
	void SetActor(Game_Actor* actor);

private:
	std::vector<int> data;

	Game_Actor* actor = nullptr;
};

#endif
