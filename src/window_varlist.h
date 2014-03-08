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

#ifndef _WINDOW_VARLIST_H_
#define _WINDOW_VARLIST_H_

// Headers
#include "window_command.h"
#include "font.h"

class Window_VarList : public Window_Command
{
public:
	/**
	 * Constructor.
	 *
	 * @param commands commands to display.
	 * @param width window width, if no width is passed
	 *              the width is autocalculated.
	 * @param max_item forces a window height for max_item
	 *                 items, if no height is passed
	 *                 the height is autocalculated.
	 */
	Window_VarList(std::vector<std::string> commands);
	~Window_VarList();

	/**
	* UpdateList.
	* 
	* @param starting value.
	*/
	void UpdateList(int first_value);

	/**
	 * Refreshes the window contents.
	 */
	void  Refresh();

	/**
	 * Indicate if item value displayed on the window correspond to switches or variables.
	 *
	 * @param true to display switches, false to display variables.
	 */
	void SetShowSwitch(bool _switch);

	/**
	 * Overwrite SetActive to hide/show selection rect when window is disabled.
	 *
	 * @param bool indicating if window will be enabled or disabled.
	 */
	void SetActive(bool nactive);

	/**
	 * Overwrite GetIndex to return the hidden index when the window is disabled.
	 */
	int GetIndex();

private:

	/**
	 * Draws the value of a variable standing on a row.
	 *
	 * @param index row with the var
	 */
	void DrawItemValue(int index);

	bool show_switch;
	int first_var;
	int hidden_index;

};

#endif
