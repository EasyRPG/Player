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
			DrawItem(index, Font::ColorDefault);
			contents->TextDraw(GetWidth() - 16, 16 * index + 2, (!Game_Switches[first_var+index]) ? Font::ColorCritical : Font::ColorDefault, Game_Switches[first_var+index] ? "[ON]" : "[OFF]", Text::AlignRight);
			break;
		case eVariable:
			DrawItem(index, Font::ColorDefault);
			contents->TextDraw(GetWidth() - 16, 16 * index + 2, (Game_Variables[first_var+index] < 0) ? Font::ColorCritical : Font::ColorDefault, std::to_string(Game_Variables[first_var+index]), Text::AlignRight);
			break;
	}
}

void Window_VarList::UpdateList(int first_value){
	static std::stringstream ss;
	first_var = first_value;
	for (int i = 0; i < 10; i++){
		if (!DataIsValid(first_var+i)) {
			continue;
		}
		ss.str("");
		ss << std::setfill('0') << std::setw(4) << (first_value + i) << ": ";
		switch (mode) {
			case eSwitch:
				ss << Game_Switches.GetName(first_value + i);
				break;
			case eVariable:
				ss << Game_Variables.GetName(first_value + i);
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
			return Game_Switches.IsValid(range_index);
		case eVariable:
			return Game_Variables.IsValid(range_index);
		default:
			break;
	}
	return false;
}

