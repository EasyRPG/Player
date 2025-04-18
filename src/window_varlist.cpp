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
#include <iomanip>
#include "window_varlist.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_strings.h"
#include "bitmap.h"
#include <lcf/data.h>
#include <lcf/reader_util.h>
#include "input.h"
#include "output.h"
#include "game_party.h"
#include "game_map.h"
#include "game_system.h"

constexpr int LINE_COUNT = 10;

Window_VarList::Window_VarList() :
	Window_Selectable(0, 32, 224, 176) {

	item_max = LINE_COUNT;
	items.reserve(LINE_COUNT);
	for (int i = 0; i < LINE_COUNT; i++)
		items.push_back("");

	index = 0;
	SetContents(Bitmap::Create(this->width - 16, item_max * menu_item_height));
}

Window_VarList::~Window_VarList() {

}

void Window_VarList::Refresh() {
	contents->Clear();

	for (int i = 0; i < LINE_COUNT; i++) {
		DrawItem(i, Font::ColorDefault);
	}
	const int item_count = GetItemCount();
	for (int i = 0; i < item_count; i++) {
		DrawItemValue(i);
	}
}

int Window_VarList::GetItemIndex() const {
	return GetIndex() * GetItemCount() / LINE_COUNT;
}

void Window_VarList::SetItemIndex(int index) {
	SetIndex(index * LINE_COUNT / GetItemCount());
}

void Window_VarList::DrawItem(int index, Font::SystemColor color) {
	const int y = menu_item_height * index + 2;
	contents->ClearRect(Rect(0, y, contents->GetWidth() - 0, menu_item_height));
	contents->TextDraw(0, y + menu_item_height / 8, color, items[index]);
}

void Window_VarList::DrawItemValue(int index){
	if (!DataIsValid(first_var+index)) {
		return;
	}
	const int y = menu_item_height * index * LINE_COUNT / GetItemCount() + 2;
	switch (mode) {
		case eSwitch:
			{
				auto value = Main_Data::game_switches->Get(first_var + index);
				auto font = (!value) ? Font::ColorCritical : Font::ColorDefault;
				contents->TextDraw(GetWidth() - 16, y, font, value ? "[ON]" : "[OFF]", Text::AlignRight);
			}
			break;
		case eVariable:
			{
				auto value = Main_Data::game_variables->Get(first_var + index);
				auto font = (value < 0) ? Font::ColorCritical : Font::ColorDefault;
				contents->TextDraw(GetWidth() - 16, y, font, std::to_string(value), Text::AlignRight);
			}
			break;
		case eItem:
			{
				auto value = Main_Data::game_party->GetItemCount(first_var + index);
				auto font = (value == 0) ? Font::ColorCritical : Font::ColorDefault;
				contents->TextDraw(GetWidth() - 16, y, font, std::to_string(value), Text::AlignRight);
			}
			break;
		case eTroop:
		case eMap:
		case eHeal:
		case eCommonEvent:
		case eMapEvent:
			{
				contents->TextDraw(GetWidth() - 16, y, Font::ColorDefault, "", Text::AlignRight);
			}
			break;
		case eLevel:
			{
				auto value = Main_Data::game_party->GetActors()[first_var + index - 1]->GetLevel();
				contents->TextDraw(GetWidth() - 16, y, Font::ColorDefault, std::to_string(value), Text::AlignRight);
			}
			break;
		case eString:
			DrawStringVarItem(index, y);
		break;
		case eNone:
			break;
	}
}

void Window_VarList::UpdateList(int first_value){
	static std::stringstream ss;
	first_var = first_value;
	int map_idx = 0;
	if (mode == eMap) {
		auto iter = std::lower_bound(lcf::Data::treemap.maps.begin(), lcf::Data::treemap.maps.end(), first_value,
				[](const lcf::rpg::MapInfo& l, int r) { return l.ID < r; });
		map_idx = iter - lcf::Data::treemap.maps.begin();
	}

	for (int i = 0; i < LINE_COUNT; i++) {
		this->SetItemText(i, "");
	}

	const int item_count = GetItemCount();
	for (int i = 0; i < item_count; i++){
		if (!DataIsValid(first_var+i)) {
			continue;
		}
		ss.str("");
		ss << std::setfill('0') << std::setw(GetDigitCount()) << (first_value + i) << ": ";
		switch (mode) {
			case eSwitch:
				ss << Main_Data::game_switches->GetName(first_value + i);
				break;
			case eVariable:
				ss << Main_Data::game_variables->GetName(first_value + i);
				break;
			case eItem:
				ss << lcf::ReaderUtil::GetElement(lcf::Data::items, first_value+i)->name;
				break;
			case eTroop:
				ss << lcf::ReaderUtil::GetElement(lcf::Data::troops, first_value+i)->name;
				break;
			case eMap:
				if (map_idx < static_cast<int>(lcf::Data::treemap.maps.size())) {
					auto& map = lcf::Data::treemap.maps[map_idx];
					if (map.ID == first_value + i) {
						ss << map.name;
						++map_idx;
					}
				}
				break;
			case eHeal:
				if (first_value + i == 1) {
					ss << "Party";
				} else {
					auto* actor = Main_Data::game_party->GetActors()[first_value + i-2];
					ss << actor->GetName() << " " << actor->GetHp() << " / " << actor->GetMaxHp();
				}
				break;
			case eLevel:
				if (first_value + i >= 1) {
					auto* actor = Main_Data::game_party->GetActors()[first_value + i-1];
					ss << actor->GetName();
				}
				break;
			case eCommonEvent:
				ss << lcf::ReaderUtil::GetElement(lcf::Data::commonevents, first_value+i)->name;
				break;
			case eMapEvent:
				ss << Game_Map::GetEvent(first_value + i)->GetName();
				break;
			case eString:
				if (show_detail) {
					std::string strvar_name = ToString(Main_Data::game_strings->GetName(first_value + i));
					if (strvar_name.empty()) {
						strvar_name = "---";
					}
					ss << strvar_name;
				}
				break;
			default:
				break;
		}
		int item_idx = i * LINE_COUNT / item_count;
		this->SetItemText(item_idx, ss.str());
	}
}

void Window_VarList::SetItemText(unsigned index, std::string_view text) {
	if (index < item_max) {
		items[index] = ToString(text);
	}
}

void Window_VarList::SetMode(Mode mode) {
	this->mode = mode;
	SetVisible((mode != eNone));
	Refresh();
}

bool Window_VarList::DataIsValid(int range_index) {
	switch (mode) {
		case eSwitch:
			return Main_Data::game_switches->IsValid(range_index);
		case eVariable:
			return Main_Data::game_variables->IsValid(range_index);
		case eItem:
			return range_index > 0 && range_index <= static_cast<int>(lcf::Data::items.size());
		case eTroop:
			return range_index > 0 && range_index <= static_cast<int>(lcf::Data::troops.size());
		case eMap:
			return range_index > 0 && range_index <= (lcf::Data::treemap.maps.size() > 0 ? lcf::Data::treemap.maps.back().ID : 0);
		case eHeal:
			return range_index > 0 && range_index <= static_cast<int>(Main_Data::game_party->GetActors().size()) + 1;
		case eLevel:
			return range_index > 0 && range_index <= static_cast<int>(Main_Data::game_party->GetActors().size());
		case eCommonEvent:
			return range_index > 0 && range_index <= static_cast<int>(lcf::Data::commonevents.size());
		case eMapEvent:
			return Game_Map::GetEvent(range_index) != nullptr;
		case eString:
			return range_index > 0 && range_index <= Main_Data::game_strings->GetSizeWithLimit();
		default:
			break;
	}
	return false;
}

int Window_VarList::GetNumElements(Mode mode) {
	switch (mode) {
		case Window_VarList::eSwitch:
			return Main_Data::game_switches->GetSizeWithLimit();
		case Window_VarList::eVariable:
			return Main_Data::game_variables->GetSizeWithLimit();
		case eItem:
			return static_cast<int>(lcf::Data::items.size());
		case eTroop:
			return static_cast<int>(lcf::Data::troops.size());
		case eMap:
			return lcf::Data::treemap.maps.size() > 0 ? lcf::Data::treemap.maps.back().ID : 0;
		case eCommonEvent:
			return static_cast<int>(lcf::Data::commonevents.size());
		case eMapEvent:
			return Game_Map::GetHighestEventId();
		case eString:
			return Main_Data::game_strings->GetSizeWithLimit();
		default:
			return -1;
	}
}

void Window_VarList::Update() {
	int index_prev = this->index;

	if (show_detail) {
		suspend_cursor_refresh = true;
	}

	Window_Selectable::Update();

	if (show_detail) {
		int div = LINE_COUNT / GetItemCount();
		if (int offs = index % div; offs > 0) {
			if (index < index_prev) {
				index -= offs;
				if (index < 0) {
					index = 0;
				}
			} else if (index > index_prev) {
				index -= offs;
				if (index + div < item_max) {
					index += div;
				} else if (index_prev != 0) {
					index = 0;
				}
			}
		}
		suspend_cursor_refresh = false;
		UpdateCursorRect();
		SetCursorRect({ cursor_rect.x, cursor_rect.y, cursor_rect.width, cursor_rect.height * div });
	}
}

void Window_VarList::UpdateCursorRect() {
	if (suspend_cursor_refresh) {
		return;
	}
	Window_Selectable::UpdateCursorRect();
}

void Window_VarList::DrawStringVarItem(int index, int y) {
	auto value = ToString(Main_Data::game_strings->Get(first_var + index));
	const int space_reserved = (GetDigitCount() + 2);
	int x = space_reserved * 6, max_len_vals = 32 - space_reserved;

	bool is_json = false;
#ifdef HAVE_NLOHMANN_JSON
	auto& json_cache = Main_Data::game_strings->_json_cache;
	is_json = json_cache.find(first_var + index) != json_cache.end();
#endif

	if (is_json) {
		if (show_detail) {
			contents->TextDraw(GetWidth() - 16, y, Font::ColorHeal, "[JSON]", Text::AlignRight);
		} else {
			contents->TextDraw(GetWidth() - 16, y, Font::ColorHeal, "[J]", Text::AlignRight);
		}
	}

	if (show_detail) {
		x = 6;
		y += menu_item_height;
		max_len_vals += space_reserved;
	}
	if (value.empty()) {
		contents->TextDraw(x, y, Font::ColorDisabled, "undefined", Text::AlignLeft);
	} else {
		size_t pos = 0;
		while ((pos = value.find("\n", pos)) != std::string::npos) {
			value.replace(pos, 1, "\\n");
			pos += 3;
		}
		if (value.length() > max_len_vals) {
			value = value.substr(0, max_len_vals - 3) + "...";
		}
		if (show_detail) {
			contents->TextDraw(x, y, Font::ColorDisabled, "\"", Text::AlignLeft);
			contents->TextDraw(x + 6, y, Font::ColorCritical, value, Text::AlignLeft);
			contents->TextDraw(x + value.length() * 6 + 6, y, Font::ColorDisabled, "\"", Text::AlignLeft);
		} else {
			contents->TextDraw(x, y, Font::ColorCritical, value, Text::AlignLeft);
		}
	}
}
