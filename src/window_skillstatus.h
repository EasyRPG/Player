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

#ifndef _WINDOW_SKILLSTATUS_H_
#define _WINDOW_SKILLSTATUS_H_

// Headers
#include "window_base.h"

/**
 * Window_SkillStatus class.
 */
class Window_SkillStatus : public Window_Base {

public:
	/**
	 * Constructor.
	 */
	Window_SkillStatus(int ix, int iy, int iwidth, int iheight);

	/**
	 * Sets the actor whose stats are displayed.
	 * @param actor_id ID of the actor.
	 */
	void SetActor(int actor_id);

	/**
	 * Renders the stats of the actor.
	 */
	void Refresh();

private:
	int actor_id;
};

#endif
