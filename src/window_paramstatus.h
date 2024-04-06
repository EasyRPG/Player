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

#ifndef EP_WINDOW_PARAMSTATUS_H
#define EP_WINDOW_PARAMSTATUS_H

// Headers
#include "window_base.h"

/**
 * Window_ParamStatus class.
 * Displays stats of the hero.
 */
class Window_ParamStatus : public Window_Base {

public:
	/**
	 * Constructor.
	 *
	 * @param ix window x position.
	 * @param iy window y position.
	 * @param iwidth window width.
	 * @param iheight window height.
	 * @param actor_id actor whose stats are displayed.
	 */
	Window_ParamStatus(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id);

	/**
	 * Refreshes screen.
	 */
	void Refresh();

private:
	int actor_id = 0;
};

#endif
