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
#include "game_party.h"
#include "bitmap.h"
#include "font.h"
#include "output.h"
#include <lcf/reader_util.h>

Window_ShopBuy::Window_ShopBuy(const std::vector<int>& goods,
		int ix, int iy, int iwidth, int iheight)
	: Window_Selectable(ix, iy, iwidth, iheight)
	  , data(goods)
{
	index = 0;
	item_max = data.size();
}

int Window_ShopBuy::GetItemId() {
	if (index < 0 || index >= (int)data.size()) {
		return 0;
	} else {
		return data[index];
	}
}

void Window_ShopBuy::Refresh() {
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
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);

	int price = 0;
	bool enabled = false;

	if (!item) {
		Output::Warning("Window ShopBuy: Invalid item ID {}", item_id);
	} else {
		enabled = item->price <= Main_Data::game_party->GetGold() && Main_Data::game_party->GetItemCount(item_id) < Main_Data::game_party->GetMaxItemCount(item_id);
		price = item->price;
	}

	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);
	DrawItemName(*item, rect.x, rect.y, enabled);

	std::string str = std::to_string(price);
	contents->TextDraw(rect.width, rect.y, enabled ? Font::ColorDefault : Font::ColorDisabled, str, Text::AlignRight);
}

void Window_ShopBuy::UpdateHelp() {
	std::string help_text = "";
	if (!data.empty() && index < data.size()) {
		const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, data[index]);
		if (item) {
			help_text = ToString(item->description);
		} else {
			help_text = "??? BAD ITEM ???";
		}
	}

	help_window->SetText(std::move(help_text));
}

bool Window_ShopBuy::CheckEnable(int item_id) {
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		return false;
	}

	return (item->price <= Main_Data::game_party->GetGold() &&
		Main_Data::game_party->GetItemCount(item_id) < Main_Data::game_party->GetMaxItemCount(item_id));
}
