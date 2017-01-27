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
#include <iomanip>
#include <sstream>
#include "window_item.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"
#include "game_temp.h"

Window_Item::Window_Item(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 2;
}

const RPG::Item* Window_Item::GetItem() const {
	if (index < 0 || index >= (int)Data::items.size() || data[index] == 0) {
		return NULL;
	}

	return &Data::items[data[index] - 1];
}

bool Window_Item::CheckInclude(int item_id) {
	if (data.size() == 0 && item_id == 0) {
		return true;
	} else {
		return (item_id > 0);
	}
}

bool Window_Item::CheckEnable(int item_id) {
	return Main_Data::game_party->IsItemUsable(item_id);
}

void Window_Item::Refresh() {
	std::vector<int> party_items;

	data.clear();
	Main_Data::game_party->GetItems(party_items);

	for (size_t i = 0; i < party_items.size(); ++i) {
		if (this->CheckInclude(party_items[i])) {
			data.push_back(party_items[i]);
		}
	}

	if (Game_Temp::battle_running) {
		// Include equipped accessories that invoke skills
		if (actor) {
			for (int i = 1; i <= 5; ++i) {
				const RPG::Item* item = actor->GetEquipment(i);
				if (item && item->use_skill && item->skill_id > 0 &&
						std::find(data.begin(), data.end(), item->ID) == data.end()) {
					data.push_back(item->ID);
				}
			}
		}
	}

	if (CheckInclude(0)) {
		data.push_back(0);
	}

	item_max = data.size();

	CreateContents();
	
	if (index > 0 && index >= item_max) {
		--index;
	} 

	contents->Clear();

	for (int i = 0; i < item_max; ++i) {
		DrawItem(i);
	}
}

void Window_Item::DrawItem(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	int item_id = data[index];

	if (item_id > 0) {
		int number = Main_Data::game_party->GetItemCount(item_id);

		if (actor) {
			const RPG::Item &item = Data::items[item_id - 1];
			if (item.use_skill) {
				number += actor->GetItemCount(item_id);
			}
		}

		bool enabled = CheckEnable(item_id);
		DrawItemName(&Data::items[item_id - 1], rect.x, rect.y, enabled);

		std::stringstream ss;
		ss << number;
		Font::SystemColor color = enabled ? Font::ColorDefault : Font::ColorDisabled;
		contents->TextDraw(rect.x + rect.width - 28, rect.y, color, "x");
		contents->TextDraw(rect.x + rect.width - 6, rect.y, color, ss.str(), Text::AlignRight);
	}
}

void Window_Item::UpdateHelp() {
	help_window->SetText(GetItem() == NULL ? "" :
		Data::items[GetItem()->ID - 1].description);
}

void Window_Item::SetActor(Game_Actor * actor) {
	this->actor = actor;
}
