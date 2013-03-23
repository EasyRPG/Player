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

#ifndef _WINDOW_TARGETSTATUS_H_
#define _WINDOW_TARGETSTATUS_H_

// Headers
#include "window_base.h"

/**
 * Window_TargetStatus class.
 * Shows owned (and equipped) items.
 * If needed it can also display the costs of a skill.
 */
class Window_TargetStatus : public Window_Base {

public:
	/**
	 * Constructor.
	 */
	Window_TargetStatus(int ix, int iy, int iwidth, int iheight);

	/**
	 * Renders the current item quantity/spell costs on
	 * the window.
	 */
	void Refresh();

	/**
	 * Sets the ID of the item/skill that shall be used.
	 *
	 * @param id ID of item/skill.
	 * @param is_item true if ID for an item, otherwise for a skill.
	 */
	void SetData(int id, bool is_item);

private:
	/** ID of item or skill. */
	int id;
	/** True if item, false if skill. */
	bool use_item;
};

#endif
