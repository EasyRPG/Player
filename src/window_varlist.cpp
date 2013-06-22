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
Window_Command(commands, 224, 10), show_switch(true), range(0) {
	SetX(0);
	SetY(32);
	SetHeight(176);
	SetWidth(224);
}


Window_VarList::~Window_VarList() {

}

void Window_VarList::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawItem(i, Font::ColorDefault);
		DrawItemValue(i);
	}
}

void Window_VarList::DrawItemValue(int index){
	if (show_switch){
		contents->TextDraw(GetWidth() - 16, 16 * index + 2, Font::ColorDefault, Game_Switches[range*10+index+1] ? "[ON]" : "[OFF]", Text::AlignRight);
	}
	else {
		std::stringstream ss;
		ss  << Game_Variables[range*10+index];
		contents->TextDraw(GetWidth() - 16, 16 * index + 2, Font::ColorDefault, ss.str(), Text::AlignRight);
	}
}

void Window_VarList::UpdateList(int first_value){
	std::stringstream ss;
	range = first_value;
	for (int i = 0; i < 11; i++){
		ss.str("");
		ss << std::setw(4) << (first_value * 10 + i + 1);
		this->SetItemText(i, ss.str());
	}
}

void Window_VarList::SetShowSwitch(bool _switch) {
	show_switch = _switch;
	Refresh();
}