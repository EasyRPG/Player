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
#include "window_buy.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "input.h"

////////////////////////////////////////////////////////////
Window_Buy::Window_Buy(int ix, int iy, int iwidth, int iheight) : 
	Window_Base(ix, iy, iwidth, iheight),
	help_window(NULL),
	total_window(NULL) {

	index = 0;
	top_index = 0;

	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
	SetZ(9999);

	row_max = (contents->GetHeight() - 2 * border_y) / row_spacing;

	Refresh();
	UpdateCursorRect();
}

Window_Buy::~Window_Buy() {
}

int Window_Buy::GetSelected(void) {
	return Game_Temp::shop_goods[index];
}

Rect Window_Buy::GetItemRect(int index) {
	Rect rect = Rect();
	int width = contents->GetWidth() - 2 * border_x;
	int height = row_spacing;
	rect.width = width;
	rect.height = height;
	rect.x = border_x;
	rect.y = (index - top_index) * height + border_y;

	return rect;
}

void Window_Buy::UpdateCursorRect() {
	SetCursorRect(GetItemRect(index));
}

void Window_Buy::Refresh() {
	contents->Clear();

	contents->GetFont()->color = Font::ColorDefault;

	for (int i = 0; i < row_max; i++) {
		int idx = top_index + i;
		if ((size_t) idx >= Game_Temp::shop_goods.size())
			break;
		int item_id = Game_Temp::shop_goods[idx];
		const std::string& s = Data::items[item_id - 1].name;
		contents->TextDraw(border_x + 4, border_y + 2 + i * row_spacing, s);
	}
}

void Window_Buy::Update() {
	Window_Base::Update();
	if (active) {
		if (Input::IsRepeated(Input::DOWN)) {
			Game_System::SePlay(Data::system.cursor_se);
			index++;
			if ((size_t) index >= Game_Temp::shop_goods.size())
				index--;
			top_index = std::max(top_index, index - row_max + 1);
		}
		if (Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Data::system.cursor_se);
			index--;
			if (index < 0)
				index++;
			top_index = std::min(top_index, index);
		}
		Refresh();

		int item_id = GetSelected();

		if (help_window)
			help_window->SetText(Data::items[item_id - 1].description);

		if (total_window) {
			int possessed = Game_Party::ItemNumber(item_id);
			int equipped = 0;
			const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
			for (size_t i = 0; i < actors.size(); i++) {
				const Game_Actor* actor = actors[i];
				if (actor->GetWeaponId() == item_id)
					equipped++;
				if (actor->GetShieldId() == item_id)
					equipped++;
				if (actor->GetArmorId() == item_id)
					equipped++;
				if (actor->GetHelmetId() == item_id)
					equipped++;
				if (actor->GetAccessoryId() == item_id)
					equipped++;
			}
			total_window->SetPossessed(possessed);
			total_window->SetEquipped(equipped);
		}
	}

	UpdateCursorRect();
}

void Window_Buy::SetHelpWindow(Window_Help* w) {
	help_window = w;
}

void Window_Buy::SetTotalWindow(Window_Total* w) {
	total_window = w;
}
