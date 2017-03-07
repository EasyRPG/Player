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

// Headers
#include "window_selectable.h"
#include "game_system.h"
#include "input.h"
#include "util_macro.h"
#include "bitmap.h"

// Constructor
Window_Selectable::Window_Selectable(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight),
	item_max(1),
	column_max(1),
	index(-1),
	help_window(NULL) {
}

void Window_Selectable::CreateContents() {
	SetContents(Bitmap::Create(width - 16, max(height - 16, GetRowMax() * 16)));
}

// Properties

int Window_Selectable::GetIndex() const {
	return index;
}
void Window_Selectable::SetIndex(int nindex) {
	index = min(nindex, item_max - 1);
	if (active && help_window != NULL) {
		UpdateHelp();
	}
	UpdateCursorRect();
}
int Window_Selectable::GetRowMax() const {
	return (item_max + column_max - 1) / column_max;
}
int Window_Selectable::GetTopRow() const {
	return oy / 16;
}
void Window_Selectable::SetTopRow(int row) {
	if (row < 0) row = 0;
	if (row > GetRowMax() - 1) row = GetRowMax() - 1;
	SetOy(row * 16);
}
int Window_Selectable::GetPageRowMax() const {
	return (height - 16) / 16;
}
int Window_Selectable::GetPageItemMax() {
	return GetPageRowMax() * column_max;
}

Rect Window_Selectable::GetItemRect(int index) {
	Rect rect = Rect();
	rect.width = (contents->GetWidth()) / column_max - 4;
	rect.height = 12;
	rect.x = index % column_max * rect.width;
	if (rect.x > 0){
		rect.x += 8;
	}
	rect.y = index / column_max * 16 + 2;
	return rect;
}

Window_Help* Window_Selectable::GetHelpWindow() {
	return help_window;
}

void Window_Selectable::SetHelpWindow(Window_Help* nhelp_window) {
	help_window = nhelp_window;
	if (active && help_window != NULL) {
		UpdateHelp();
	}
}

void Window_Selectable::UpdateHelp() {
}

// Update Cursor Rect
void Window_Selectable::UpdateCursorRect() {
	int cursor_width = 0;
	int x = 0;
	if (index < 0) {
		SetCursorRect(Rect());
		return;
	}
	int row = index / column_max;
	if (row < GetTopRow()) {
		SetTopRow(row);
	} else if (row > GetTopRow() + (GetPageRowMax() - 1)) {
		SetTopRow(row - (GetPageRowMax() - 1));
	}

	if (column_max > 1){
		cursor_width = (width / column_max - 16) + 12;
		x = (index % column_max * cursor_width) - 4 ;
	}
	else{
		cursor_width = (width / column_max - 16) + 8;
		x = (index % column_max * (cursor_width + 16)) - 4;
	}

	int y = index / column_max * 16 - oy;
	SetCursorRect(Rect(x, y, cursor_width, 16));

}

// Update
void Window_Selectable::Update() {
	Window_Base::Update();
	if (active && item_max > 0 && index >= 0) {
		if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
			if (index < item_max - column_max || (column_max == 1 &&
				(Input::IsTriggered(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)))) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				index = (index + column_max) % item_max;
			}
		}
		if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
			if (index >= column_max || (column_max == 1 &&
				(Input::IsTriggered(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)))) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				index = (index - column_max + item_max) % item_max;
			}
		}
		// page up/down is limited to selectables with one column
		if (column_max == 1) {
			if (Input::IsRepeated(Input::PAGE_DOWN) && index < item_max - 1) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				int new_pos = index + GetPageRowMax();
				index = (new_pos <= item_max - 1) ? new_pos : item_max - 1;
			}
			if (Input::IsRepeated(Input::PAGE_UP) && index > 0) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				int new_pos = index - GetPageRowMax();
				index = (new_pos >= 0) ? new_pos : 0;
			}
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			if (column_max >= 2 && index < item_max - 1) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				index += 1;
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			if (column_max >= 2 && index > 0) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				index -= 1;
			}
		}
	}
	if (active && help_window != NULL) {
		UpdateHelp();
	}
	UpdateCursorRect();
}
