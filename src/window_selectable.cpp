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

constexpr int arrow_animation_frames = 20;

// Constructor
Window_Selectable::Window_Selectable(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) { }

void Window_Selectable::CreateContents() {
	int w = std::max(0, width - border_x * 2);
	int h = std::max(0, std::max(height - border_y * 2, GetRowMax() * menu_item_height));

	SetContents(Bitmap::Create(w, h));
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
int Window_Selectable::GetColumnMax() const {
	return column_max;
}
void Window_Selectable::SetColumnMax(int ncolmax) {
	column_max = ncolmax;
}
int Window_Selectable::GetRowMax() const {
	return (item_max + column_max - 1) / column_max;
}
int Window_Selectable::GetTopRow() const {
	return oy / menu_item_height;
}
void Window_Selectable::SetTopRow(int row) {
	if (row < 0) row = 0;
	if (row > GetRowMax() - 1) row = GetRowMax() - 1;
	SetOy(row * menu_item_height);
}
int Window_Selectable::GetPageRowMax() const {
	return (height - border_y * 2) / menu_item_height;
}
int Window_Selectable::GetPageItemMax() {
	return GetPageRowMax() * column_max;
}

Rect Window_Selectable::GetItemRect(int index) {
	Rect rect = Rect();
	rect.width = (width / column_max - 16);
	rect.x = (index % column_max * (rect.width + 16));
	rect.height = menu_item_height - 4;
	rect.y = index / column_max * menu_item_height + menu_item_height / 8;
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
	if (UpdateHelpFn && help_window != nullptr) {
		UpdateHelpFn(*help_window, index);
	}
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

	cursor_width = (width / column_max - 16) + 8;
	x = (index % column_max * (cursor_width + 8)) - 4;

	int y = index / column_max * menu_item_height - oy;
	SetCursorRect(Rect(x, y, cursor_width, menu_item_height));
}

void Window_Selectable::UpdateArrows() {
	bool show_up_arrow = (GetTopRow() > 0);
	bool show_down_arrow = (GetTopRow() < (GetRowMax() - GetPageRowMax()));

	if (show_up_arrow || show_down_arrow) {
		arrow_frame = (arrow_frame + 1) % (arrow_animation_frames * 2);
	}
	bool arrow_visible = (arrow_frame < arrow_animation_frames);
	SetUpArrow(show_up_arrow && arrow_visible);
	SetDownArrow(show_down_arrow && arrow_visible);
}

// Update
void Window_Selectable::Update() {
	Window_Base::Update();
	if (active && item_max > 0 && index >= 0) {
		if (scroll_dir != 0) {
			scroll_progress++;
			SetOy(GetOy() + (menu_item_height * scroll_progress / 4 - menu_item_height * (scroll_progress - 1) / 4) * scroll_dir);
			UpdateArrows();
			if (scroll_progress < 4) {
				return;
			} else {
				scroll_dir = 0;
				scroll_progress = 0;
				if (active && help_window != NULL) {
					UpdateHelp();
				}
				UpdateCursorRect();
			}
		}

		int old_index = index;

		auto move_down = [&]() {
			if (index < item_max - column_max || column_max == 1 ) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				index = (index + column_max) % item_max;
			}
		};
		if (Input::IsTriggered(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
			move_down();
		} else if (Input::IsRepeated(Input::DOWN)) {
			if (endless_scrolling || (index + column_max) % item_max > index) {
				move_down();
			}
		}

		auto move_up = [&]() {
			if (index >= column_max || column_max == 1) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				index = (index - column_max + item_max) % item_max;
			}
		};
		if (Input::IsTriggered(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
			move_up();
		} else if (Input::IsRepeated(Input::UP)) {
			if (endless_scrolling || (index - column_max + item_max) % item_max < index) {
				move_up();
			}
		}

		// page up/down is limited to selectables with one column
		if (column_max == 1) {
			if (Input::IsRepeated(Input::PAGE_DOWN) && index < item_max - 1) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				int new_pos = index + GetPageRowMax();
				index = (new_pos <= item_max - 1) ? new_pos : item_max - 1;
			}
			if (Input::IsRepeated(Input::PAGE_UP) && index > 0) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				int new_pos = index - GetPageRowMax();
				index = (new_pos >= 0) ? new_pos : 0;
			}
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			if (column_max >= 2 && index < item_max - 1) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				index += 1;
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			if (column_max >= 2 && index > 0) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				index -= 1;
			}
		}

		if (std::abs(index - old_index) <= column_max) {
			int row = index / column_max;
			if (row < GetTopRow() && old_index < item_max - 1) {
				scroll_dir = -1;
				return;
			} else if (row > GetTopRow() + (GetPageRowMax() - 1) && old_index > 0) {
				scroll_dir = 1;
				return;
			}
		}
	}
	if (active && help_window != NULL) {
		UpdateHelp();
	}
	UpdateCursorRect();
	UpdateArrows();
}

// Set endless scrolling state
void Window_Selectable::SetEndlessScrolling(bool state) {
	endless_scrolling = state;
}

// Set menu item height
void Window_Selectable::SetMenuItemHeight(int height) {
	menu_item_height = height;
}
