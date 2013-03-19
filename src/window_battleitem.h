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

#ifndef _WINDOW_BATTLEITEM_H_
#define _WINDOW_BATTLEITEM_H_

// Headers
#include <vector>
#include "window_help.h"
#include "window_item.h"

/**
 * Window BattleItem class.
 * Displays all items of the party for use in battle.
 */
class Window_BattleItem : public Window_Item {

public:
	/**
	 * Constructor.
	 */
	Window_BattleItem(int ix, int iy, int iwidth, int iheight);

	/**
	 * Checks if item should be enabled.
	 *
	 * @param item_id item to check.
	 */
	virtual bool CheckEnable(int item_id);

	/**
	 * Sets the actor used for enable checks.
	 *
	 * @param actor_id the actor.
	 */
	void SetActor(int actor_id);

protected:
	int actor_id;

	bool CanUseItem(const RPG::Item& item);
	bool CanUseSkill(int skill_id);
};

#endif
