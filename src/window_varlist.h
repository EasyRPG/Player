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

#ifndef EP_WINDOW_VARLIST_H
#define EP_WINDOW_VARLIST_H

// Headers
#include "window_command.h"

class Window_VarList : public Window_Command
{
public:
	enum Mode {
		eNone,
		eSwitch,
		eVariable,
		eItem,
		eTroop,
		eMap,
		eHeal,
		eLevel,
		eCommonEvent,
		eMapEvent,
	};

	/**
	 * Constructor.
	 *
	 * @param commands commands to display.
	 */
	Window_VarList(Scene* parent, std::vector<std::string> commands);
	~Window_VarList() override;

	/**
	 * UpdateList.
	 *
	 * @param first_value starting value.
	 */
	void UpdateList(int first_value);

	/**
	 * Refreshes the window contents.
	 */
	void  Refresh();

	/**
	 * Indicate what to display.
	 *
	 * @param mode the mode to set.
	 */
	void SetMode(Mode mode);

	/**
	 * Returns the current mode.
	 */
	Mode GetMode() const;

private:

	/**
	 * Draws the value of a variable standing on a row.
	 *
	 * @param index row with the var
	 */
	void DrawItemValue(int index);

	Mode mode = eNone;
	int first_var = 0;

	bool DataIsValid(int range_index);

};

inline Window_VarList::Mode Window_VarList::GetMode() const {
	return mode;
}

#endif
