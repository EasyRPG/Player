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

#ifndef EP_WINDOW_NUMBERINPUT_H
#define EP_WINDOW_NUMBERINPUT_H

// Headers
#include <cstdint>
#include "window_selectable.h"

/**
 * Window Input Number Class.
 * The number input window.
 */
class Window_NumberInput : public Window_Selectable {
public:
	/**
	 * Constructor.
	 *
	 * @param ix window x position.
	 * @param iy window y position.
	 * @param iwidth window width.
	 * @param iheight window height.
	 */
	Window_NumberInput(int ix, int iy, int iwidth = 320, int iheight = 80);

	/**
	 * Updates the Window's contents.
	 */
	void Refresh();

	/**
	 * Returns the number value.
	 *
	 * @return the currently input number.
	 */
	int GetNumber() const;

	/**
	 * Sets a new number value.
	 *
	 * @param inumber the new number value.
	 */
	void SetNumber(int inumber);

	/**
	 * Returns the number of displayed digits.
	 *
	 * @return number of displayed digits.
	 */
	int GetMaxDigits() const;

	/**
	 * Sets the maximal displayed digits.
	 *
	 * @param idigits_max maximal displayed digits
	 *                    must be a value from 1-6.
	 */
	void SetMaxDigits(int idigits_max);

	/**
	 * Gets whether the +- operator is displayed before the numbers.
	 *
	 * @return the current operator state
	 */
	bool GetShowOperator() const;

	/**
	 * Enables or Disables the +- operator before the numbers.
	 * By default no negative values are possible.
	 *
	 * @param show Show operators
	 */
	void SetShowOperator(bool show);

	/**
	 * Updates the position of the cursor rectangle.
	 */
	void UpdateCursorRect() override;

	/**
	 * Updates number value according to user input.
	 */
	void Update() override;

protected:
	int64_t number;
	int digits_max;
	int cursor_width;
	int index;
	bool show_operator;
	bool plus;

	void ResetIndex();
};

#endif
