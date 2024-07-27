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
#include "output.h"

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

	bool cursor_is_inline = !inline_rects.empty() && index < static_cast<int>(inline_rects.size());
	if (cursor_is_inline) {
		InlineUpdateCursorRect();
		return;
	}

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
	y += ( menu_item_line_spacing * index ) - (4 * index); // calculate spacing between lines

	int item_height = menu_item_height;
	if (border_x == 0) { // When margin doesn't exist
		x += 4;
		cursor_width += 8;
		if (index == 0) item_height += 3;
	}

	SetCursorRect(Rect(x, y, cursor_width, item_height));
}

void Window_Selectable::InlineUpdateCursorRect() {
	if (index < 0 || index >= static_cast<int>(inline_rects.size())) {
		SetCursorRect(Rect());
		return;
	}

	const auto& rect = inline_rects[index];
	int cursor_width = rect.width + 9;
	int x = rect.x - 4;
	int y = rect.y;

	int height = this->height - border_y * 2;
	int max_y = std::max_element(inline_rects.begin(), inline_rects.end(),
		[](const auto& a, const auto& b) { return a.y + a.height < b.y + b.height; })->y + menu_item_height;
	int max_oy = std::max(0, max_y - height);

	int target_oy = oy;
	if (y < oy) {
		target_oy = y;
		scroll_dir = scroll_dir == 0 ? -1 : scroll_dir;
	}
	else if (y + menu_item_height > oy + height) {
		target_oy = std::min(y + menu_item_height - height, max_oy);
		scroll_dir = scroll_dir == 0 ? 1 : scroll_dir;
	}
	else {
		scroll_dir = 0;
	}

	if (scroll_dir != 0) {
		scroll_progress = std::min(scroll_progress + 1, 4);
		int scroll_amount = (menu_item_height * scroll_progress / 4 - menu_item_height * (scroll_progress - 1) / 4) * scroll_dir;
		oy = std::clamp(oy + scroll_amount, 0, max_oy);
		if (scroll_progress == 4) {
			scroll_dir = 0;
			scroll_progress = 0;
			oy = target_oy;
		}
	}
	else {
		oy = target_oy;
	}

	y -= oy;
	if (border_x == 0) {
		x += border_x;
		cursor_width -= border_x * 2;
	}

	//workaround to deal with the cursor appearing over the borders while scrolling
	int output_y = scroll_dir == -1 ? y / 2 : y;
	int output_height = scroll_dir == 1 ? menu_item_height / 2 : menu_item_height;

	SetCursorRect(Rect(x, output_y, cursor_width, output_height));

	UpdateArrows();
	bool latest_rect_is_oob = inline_rects.back().y + menu_item_height != oy + height;
	bool arrow_visible = (arrow_frame < arrow_animation_frames);
	SetDownArrow(latest_rect_is_oob && arrow_visible);
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

	bool cursor_is_inline = !inline_rects.empty() && index < static_cast<int>(inline_rects.size());
	if (cursor_is_inline) {
		InlineUpdate();
		return;
	}

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
			if (column_max >= wrap_limit && index < item_max - 1) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				index += 1;
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			if (column_max >= wrap_limit && index > 0) {
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

void Window_Selectable::InlineUpdate() {
	if (active && !inline_rects.empty()) {
		int old_index = index;
		if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
			MoveIndexVertical(1);
		}
		if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
			MoveIndexVertical(-1);
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			MoveIndexHorizontal(1);
		}
		if (Input::IsRepeated(Input::LEFT)) {
			MoveIndexHorizontal(-1);
		}
		if (index != old_index) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			if (active && help_window != NULL) {
				UpdateHelp();
			}
		}
	}
	InlineUpdateCursorRect();

}

void Window_Selectable::MoveIndexHorizontal(int direction) {
	int current_x = inline_rects[index].x;
	int current_y = inline_rects[index].y;
	int nearest_index = -1;
	int min_distance = std::numeric_limits<int>::max();

	for (size_t i = 0; i < inline_rects.size(); ++i) {
		if (i != index && inline_rects[i].y == current_y) {
			if ((direction > 0 && inline_rects[i].x > current_x) ||
				(direction < 0 && inline_rects[i].x < current_x)) {
				int distance = std::abs(inline_rects[i].x - current_x);
				if (distance < min_distance) {
					min_distance = distance;
					nearest_index = i;
				}
			}
		}
	}

	if (nearest_index != -1) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		index = nearest_index;
	}
}

void Window_Selectable::MoveIndexVertical(int direction) {
	int current_x = inline_rects[index].x;
	int current_y = inline_rects[index].y;
	int nearest_index = -1;
	int min_y_distance = std::numeric_limits<int>::max();
	int min_x = std::numeric_limits<int>::max();

	// First pass: find the minimum y distance in the correct direction
	for (size_t i = 0; i < inline_rects.size(); ++i) {
		if (i != index) {
			if ((direction > 0 && inline_rects[i].y > current_y) ||
				(direction < 0 && inline_rects[i].y < current_y)) {
				int y_distance = std::abs(inline_rects[i].y - current_y);
				if (y_distance < min_y_distance) {
					min_y_distance = y_distance;
				}
			}
		}
	}

	// Second pass: among buttons with minimum y distance, find the one with smallest x value
	for (size_t i = 0; i < inline_rects.size(); ++i) {
		if (i != index) {
			if ((direction > 0 && inline_rects[i].y > current_y) ||
				(direction < 0 && inline_rects[i].y < current_y)) {
				int y_distance = std::abs(inline_rects[i].y - current_y);
				if (y_distance == min_y_distance && inline_rects[i].x < min_x) {
					min_x = inline_rects[i].x;
					nearest_index = i;
				}
			}
		}
	}

	if (nearest_index != -1) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		index = nearest_index;
	}
}

// Set endless scrolling state
void Window_Selectable::SetEndlessScrolling(bool state) {
	endless_scrolling = state;
}

// Set menu item height
void Window_Selectable::SetMenuItemHeight(int height) {
	menu_item_height = height;
}

void Window_Selectable::SetMenuItemLineSpacing(int spacing) {
	menu_item_line_spacing = spacing;
}

void Window_Selectable::SetSingleColumnWrapping(bool wrap) {
	wrap_limit = wrap ? 1 : 2;
}
