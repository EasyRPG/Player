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
#include <sstream>
#include <string>
#include "window_base.h"
#include "window_shopbuy.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"
#include "output.h"
#include "reader_util.h"

Window_ShopBuy::Window_ShopBuy(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	index = 0;
}

int Window_ShopBuy::GetItemId() {
	if (index < 0) {
		return 0;
	} else {
		return data[index];
	}
}

void Window_ShopBuy::Refresh() {
	data = Game_Temp::shop_goods;
	item_max = data.size();

	CreateContents();

	contents->Clear();
	Rect rect(0, 0, contents->GetWidth(), contents->GetHeight());
	contents->Clear();

	for (size_t i = 0; i < data.size(); ++i) {
		DrawItem(i);
	}
}

void Window_ShopBuy::DrawItem(int index) {
	int item_id = data[index];

	// (Shop) items are guaranteed to be valid
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);

	int price = 0;
	bool enabled = false;

	if (!item) {
		Output::Warning("Window ShopBuy: Invalid item ID %d", item_id);
	} else {
		enabled = item->price <= Main_Data::game_party->GetGold();
		price = item->price;
	}

	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);
	DrawItemName(*item, rect.x, rect.y, enabled);

	std::string str = Utils::ToString(price);
	contents->TextDraw(rect.width + 4, rect.y, enabled ? Font::ColorDefault : Font::ColorDisabled, str, Text::AlignRight);
}

void Window_ShopBuy::UpdateHelp() {
	std::string help_text = "??? BAD ITEM ???";
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, data[index]);
	if (item) {
		help_text = item->description;
	}

	help_window->SetText(help_text);
}

bool Window_ShopBuy::CheckEnable(int item_id) {
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		return false;
	}

	return (item->price <= Main_Data::game_party->GetGold() &&
		Main_Data::game_party->GetItemCount(item_id) < 99);
}
