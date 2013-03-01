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

#ifndef _WINDOW_KEYBOARD_H_
#define _WINDOW_KEYBOARD_H_

// Headers
#include "window_base.h"
#include "window_selectable.h"

/**
 * Window Input Number Class.
 * The number input window.
 */
class Window_Keyboard : public Window_Base {
public:
	/**
	 * Constructor.
	 *
	 * @param idigits_max the maximum number of digits
	 *                    allowed.
	 */
	Window_Keyboard(int ix, int iy, int iwidth = 320, int iheight = 80);

	enum Mode {
		Hiragana = 0,
		Katakana = 1,
		Letter = 2,
		Symbol = 3,
		MODE_END = 4,
	};

	void UpdateCursorRect();
	Rect GetItemRect(int row, int col) const;
	void Update();
	void Refresh();
	void SetMode(Mode nmode);
	std::string const& GetSelected() const;

	static const char* const TO_SYMBOL;
	static const char* const TO_LETTER;
	static const char* const DONE;
	static const char* const SPACE;

	static std::string TO_KATAKANA;
	static std::string TO_HIRAGANA;
	static std::string DONE_JP;

protected:
	static const int border_x = 8;
	static const int border_y = 4;
	static const int row_max = 9;
	static const int col_max = 10;
	static const int min_width = 2;
	static std::string items[MODE_END][row_max][col_max];
	int row_spacing;
	int col_spacing;
	Mode mode;
	int row;
	int col;

	bool play_cursor;
};

#endif
