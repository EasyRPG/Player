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

#ifndef _WINDOW_INTEGEREDITOR_H_
#define _WINDOW_INTEGEREDITOR_H_

// Headers
#include "window_selectable.h"
#include "font.h"

class Window_IntegerEditor : public Window_Selectable {
public:
	/**
	 * Constructor.
	 *
	 * @param int with the initial value of the integer.
	 * @param number of columns on the editor (7 for RM2K or 8 for RM2K3).
	 */
	Window_IntegerEditor(int icolumns);

	/**
	 * Refreshes the window contents.
	 */
	void Refresh();

	/**
	 * Updates the window.
	 */
	void Update();

	/**
	 * Gets the value of the variable.
	 */
	int GetValue();

	/**
	 * Sets the current value.
	 */
	void SetValue(int ivalue);

	/**
	 * Overwrites Window_Selectable::UpdateCursorRect() to fit to a 7/8 columns window.
	 */
	void UpdateCursorRect();

private:

	int digits[8];

	void DrawDigit(int index, Font::SystemColor color);
	void RiseDigit(int index);
	void ReduceDigit(int index);
};

#endif