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
#include "bitmap.h"
#include "data.h"
#include "output.h"
#include "reader_util.h"
#include "game_party.h"

Window_VarList::Window_VarList(std::vector<std::string> commands) :
Window_Command(commands, 224, 10) {
	SetX(0);
	SetY(32);
	SetHeight(176);
	SetWidth(224);
}

Window_VarList::~Window_VarList() {

}

void Window_VarList::Refresh() {
	contents->Clear();
	for (int i = 0; i < 10; i++) {
		DrawItemValue(i);
	}
}

void Window_VarList::DrawItemValue(int index){
	if (!DataIsValid(first_var+index)) {
		return;
	}
	switch (mode) {
		case eSwitch:
			{
				auto value = Main_Data::game_switches->Get(first_var + index);
				auto font = (!value) ? Font::ColorCritical : Font::ColorDefault;
				DrawItem(index, Font::ColorDefault);
				contents->TextDraw(GetWidth() - 16, 16 * index + 2, font, value ? "[ON]" : "[OFF]", Text::AlignRight);
			}
			break;
		case eVariable:
			{
				auto value = Main_Data::game_variables->Get(first_var + index);
				auto font = (value < 0) ? Font::ColorCritical : Font::ColorDefault;
				DrawItem(index, Font::ColorDefault);
				contents->TextDraw(GetWidth() - 16, 16 * index + 2, font, std::to_string(value), Text::AlignRight);
			}
			break;
		case eItem:
			{
				auto value = Main_Data::game_party->GetItemCount(first_var + index);
				auto font = (value == 0) ? Font::ColorCritical : Font::ColorDefault;
				DrawItem(index, Font::ColorDefault);
				contents->TextDraw(GetWidth() - 16, 16 * index + 2, font, std::to_string(value), Text::AlignRight);
			}
			break;
		case eTroop:
		case eMap:
		case eHeal:
		case eCommonEvent:
			{
				DrawItem(index, Font::ColorDefault);
				contents->TextDraw(GetWidth() - 16, 16 * index + 2, Font::ColorDefault, "", Text::AlignRight);
			}
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
		auto iter = std::lower_bound(Data::treemap.maps.begin(), Data::treemap.maps.end(), first_value,
				[](const RPG::MapInfo& l, int r) { return l.ID < r; });
		map_idx = iter - Data::treemap.maps.begin();
	}
	for (int i = 0; i < 10; i++){
		if (!DataIsValid(first_var+i)) {
			continue;
		}
		ss.str("");
		ss << std::setfill('0') << std::setw(4) << (first_value + i) << ": ";
		switch (mode) {
			case eSwitch:
				ss << Main_Data::game_switches->GetName(first_value + i);
				break;
			case eVariable:
				ss << Main_Data::game_variables->GetName(first_value + i);
				break;
			case eItem:
				ss << ReaderUtil::GetElement(Data::items, first_value+i)->name;
				break;
			case eTroop:
				ss << ReaderUtil::GetElement(Data::troops, first_value+i)->name;
				break;
			case eMap:
				if (map_idx < (int)Data::treemap.maps.size()) {
					auto& map = Data::treemap.maps[map_idx];
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
			case eCommonEvent:
				ss << ReaderUtil::GetElement(Data::commonevents, first_value+i)->name;
				break;
			default:
				break;
		}
		this->SetItemText(i, ss.str());
	}
}

void Window_VarList::SetMode(Mode mode) {
	this->mode = mode;
	SetVisible((mode != eNone));
	Refresh();
}

void Window_VarList::SetActive(bool nactive) {
	Window::SetActive(nactive);
	if (nactive)
		index = hidden_index;
	else {
		hidden_index = index;
		index = -1;
	}
	Refresh();
}

int Window_VarList::GetIndex() {
	return GetActive() ? index : hidden_index;
}


bool Window_VarList::DataIsValid(int range_index) {
	switch (mode) {
		case eSwitch:
			return Main_Data::game_switches->IsValid(range_index);
		case eVariable:
			return Main_Data::game_variables->IsValid(range_index);
		case eItem:
			return range_index > 0 && range_index <= Data::items.size();
		case eTroop:
			return range_index > 0 && range_index <= Data::troops.size();
		case eMap:
			return range_index > 0 && range_index <= (Data::treemap.maps.size() > 0 ? Data::treemap.maps.back().ID : 0);
		case eHeal:
			return range_index > 0 && range_index <= Main_Data::game_party->GetActors().size() + 1;
		case eCommonEvent:
			return range_index > 0 && range_index <= Data::commonevents.size();
		default:
			break;
	}
	return false;
}

