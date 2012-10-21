/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include <string>
#include "window_keyboard.h"
#include "game_system.h"
#include "input.h"
#include "bitmap.h"
#include "font.h"

////////////////////////////////////////////////////////////
const char * const Window_Keyboard::items[2][9][10] = {
	{
	{"A","B","C","D","E","a","b","c","d","e"},
	{"F","G","H","I","J","f","g","h","i","j"},
	{"K","L","M","N","O","k","l","m","n","o"},
	{"P","Q","R","S","T","p","q","r","s","t"},
	{"U","V","W","X","Y","u","v","w","x","y"},
	{"Z"," "," "," "," ","z"," "," "," "," "},
	{"0","1","2","3","4","5","6","7","8","9"},
	{" "," "," "," "," "," "," ","Symbol"," ","Done"},
	{" "," "," "," "," "," "," "," "," "," "},
	},
	{
	{"$A","$B","$C","$D","$E","$a","$b","$c","$d","$e"},
	{"$F","$G","$H","$I","$J","$f","$g","$h","$i","$j"},
	{"$K","$L","$M","$N","$O","$k","$l","$m","$n","$o"},
	{"$P","$Q","$R","$S","$T","$p","$q","$r","$s","$t"},
	{"$U","$V","$W","$X","$Y","$u","$v","$w","$x","$y"},
	{"$Z"," "," "," "," ","$z"," "," "," "," "},
	{" "," "," "," "," "," "," "," "," "," "},
	{" "," "," "," "," "," "," ","Letter"," ","Done"},
	{" "," "," "," "," "," "," "," "," "," "},
	}
};

////////////////////////////////////////////////////////////
Window_Keyboard::Window_Keyboard(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {
	row = 0;
	col = 0;

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
	SetZ(9999);

	row_spacing = 16;
	col_spacing = (contents->GetWidth() - 2 * border_x) / col_max;

	mode = 0;

	Refresh();
	UpdateCursorRect();
}

Window_Keyboard::~Window_Keyboard() {
}

void Window_Keyboard::SetMode(int nmode) {
	mode = nmode;
	Refresh();
	UpdateCursorRect();
}

const char *Window_Keyboard::GetSelected(void) {
	return items[mode][row][col];
}

Rect Window_Keyboard::GetItemRect(int row, int col) {
	Rect rect = Rect();
	int width = col_spacing;
	int height = row_spacing;
	rect.width = width;
	rect.height = height;
	rect.x = col * width + border_x;
	rect.y = row * height + border_y;

	const std::string s(items[mode][row][col]);
	int mw = min_width;
	int tw = std::max(mw, contents->GetTextSize(s).width) + 8;
	int dx = (rect.width - tw) / 2;
	rect.x += dx;
	rect.width -= 2 * dx;

	return rect;
}

void Window_Keyboard::UpdateCursorRect() {
	SetCursorRect(GetItemRect(row, col));
}

void Window_Keyboard::Refresh() {
	contents->Clear();

	for (int j = 0; j < row_max; j++) {
		for (int i = 0; i < col_max; i++) {
			const std::string s(items[mode][j][i]);
			Rect r = GetItemRect(j, i);
			contents->TextDraw(r.x + 4, r.y + 2, Font::ColorDefault, s);
		}
	}
}

void Window_Keyboard::Update() {
	Window_Base::Update();
	if (active) {
		if (Input::IsRepeated(Input::DOWN)) {
			Game_System::SePlay(Data::system.cursor_se);
			row = (row + 1) % row_max;
		}
		if (Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Data::system.cursor_se);
			row = (row + row_max - 1) % row_max;
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			Game_System::SePlay(Data::system.cursor_se);
			col += 1;
			if (col >= col_max) {
				col = 0;
				row = (row + 1) % row_max;
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			Game_System::SePlay(Data::system.cursor_se);
			col -= 1;
			if (col < 0) {
				col = col_max - 1;
				row = (row + row_max - 1) % row_max;
			}
		}

	}

	UpdateCursorRect();
}
