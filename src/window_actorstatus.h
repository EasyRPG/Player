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

#ifndef EP_WINDOW_ACTORSTATUS_H
#define EP_WINDOW_ACTORSTATUS_H

// Headers
#include "window_base.h"
#include "font.h"

/**
 * Window ActorStatus Class.
 * Displays the right hand information window in the status
 * scene (HP, MP, EXP).
 */
class Window_ActorStatus : public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_ActorStatus(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id);

	/**
	 * Renders the stats on the window.
	 */
	void Refresh();

	/**
	 * Draws the actor status
	 */
	void DrawStatus();

	/**
	 * Draws min and max separated by a "/" in cx, cy
	 */
	void DrawMinMax(int cx, int cy, int min, int max, int color = Font::ColorDefault);

private:
	int actor_id;
};

#endif
