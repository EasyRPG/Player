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

#ifndef EP_WINDOW_KEYBOARD_H
#define EP_WINDOW_KEYBOARD_H

#include "window_base.h"

struct Keyboard_Layout;

/**
 * Window Input Number Class.
 * The number input window.
 */
class Window_Keyboard : public Window_Base {
public:
	/**
	 * Constructor.
	 *
	 * @param ix window x position.
	 * @param iy window y position.
	 * @param iwidth window width.
	 * @param iheight window height.
	 * @param ndone_text text for the "Done" button.
	 */
	Window_Keyboard(int ix, int iy, int iwidth = 320, int iheight = 80, const char* ndone_text = DONE);

	enum Mode {
		Hiragana,
		Katakana,
		Hangul1,
		Hangul2,
		ZhCn1,
		ZhCn2,
		ZhTw1,
		ZhTw2,
		RuCyrl,
		Letter,
		Symbol,
		MODE_END
	};

	void UpdateCursorRect();
	Rect GetItemRect(int row, int col) const;
	void Update() override;
	void Refresh();
	void SetMode(Mode nmode, Mode nnext_mode);
	std::string const& GetKey(int row, int col) const;
	std::string const& GetSelected() const;

	static const char* const DONE;
	static const char* const SPACE;
	static const char* const NEXT_PAGE;
	static const char* const DONE_JP;
	static const char* const DONE_KO;
	static const char* const DONE_ZH_CN;
	static const char* const DONE_ZH_TW;
	static const char* const DONE_RU;

	static const int row_max = 9;
	static const int col_max = 10;
	static Keyboard_Layout layouts[MODE_END];

protected:
	static const int border_x = 8;
	static const int border_y = 4;
	static const int min_width = 2;

	std::string done_text;
	int row_spacing;
	int col_spacing;
	Mode mode;
	Mode next_mode;
	int row;
	int col;

	bool play_cursor;
};


/**
 * Keyboard layout struct.
 * Defines which keys are available and how the key to switch here is named
 */
struct Keyboard_Layout {
	const std::string key_text;
	const std::string items[Window_Keyboard::row_max][Window_Keyboard::col_max];
};

#endif
