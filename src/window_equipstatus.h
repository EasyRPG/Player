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

#ifndef EP_WINDOW_EQUIPSTATUS_H
#define EP_WINDOW_EQUIPSTATUS_H

// Headers
#include "window_base.h"

/**
 * Window_EquipLeft class.
 * Displays stats of the hero/item.
 */
class Window_EquipStatus : public Window_Base {

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
	Window_EquipStatus(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id);

	/**
	 * Refreshes screen.
	 */
	void Refresh();

	/**
	 * Sets the parameter drawn after the ->.
	 *
	 * @param new_atk new attack.
	 * @param new_def new defense.
	 * @param new_spi new spirit.
	 * @param new_agi new agility.
	 */
	void SetNewParameters(int new_atk, int new_def, int new_spi, int new_agi);

	/**
	 * Removes the parameters drawn after the ->.
	 */
	void ClearParameters();

	/**
	 * Returns the draw color based on the passed values.
	 *
	 * @param old_value old value.
	 * @param new_value new value,
	 * @return 0 if equal, 4 if new > old, 5 if old > new,
	 */
	int GetNewParameterColor(int old_value, int new_value);

	/**
	 * Draws the actor parameters followed by the new ones.
	 *
	 * @param cx x coordinate.
	 * @param cy y coordinate.
	 * @param type parameter type to draw.
	 */
	void DrawParameter(int cx, int cy, int type);

private:
	int actor_id;
	/** Draws the params if true. */
	bool draw_params;

	int atk;
	int def;
	int spi;
	int agi;

	/** Needs redraw. */
	bool dirty;
};

#endif
