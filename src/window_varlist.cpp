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
Window_Command(commands, 224, 10), show_switch(true), first_var(0) {
	SetX(0);
	SetY(32);
	SetHeight(176);
	SetWidth(224);
	hidden_index = 0;
}


Window_VarList::~Window_VarList() {

}

void Window_VarList::Refresh() {
	contents->Clear();
	for (int i = 0; i < 10; i++) {
		if (!show_switch && Game_Variables.IsValid(first_var+i)) {
			DrawItem(i, Font::ColorDefault);
		}
		DrawItemValue(i);
	}
}

void Window_VarList::DrawItemValue(int index){
	if (show_switch){
		if (!Game_Switches.IsValid(first_var+index))
			return;
		DrawItem(index, Font::ColorDefault);
		contents->TextDraw(GetWidth() - 16, 16 * index + 2, (!Game_Switches[first_var+index]) ? Font::ColorCritical : Font::ColorDefault, Game_Switches[first_var+index] ? "[ON]" : "[OFF]", Text::AlignRight);
	}
	else {
		if (!Game_Variables.IsValid(first_var+index))
			return;
		DrawItem(index, Font::ColorDefault);
		std::stringstream ss;
		ss  << Game_Variables[first_var+index];
		contents->TextDraw(GetWidth() - 16, 16 * index + 2, (Game_Variables[first_var+index] < 0) ? Font::ColorCritical : Font::ColorDefault, ss.str(), Text::AlignRight);
	}
}

void Window_VarList::UpdateList(int first_value){
	static std::stringstream ss;
	first_var = first_value;
	for (int i = 0; i < 10; i++){
		ss.str("");
		if ((show_switch && Game_Switches.IsValid(first_var+i)) || ((!show_switch && Game_Variables.IsValid(first_var+i))))
			ss << std::setfill('0') << std::setw(4) << (first_value + i) << ": " << (show_switch ? Game_Switches.GetName(first_value + i) : Game_Variables.GetName(first_value + i));
		this->SetItemText(i, ss.str());
	}
}

void Window_VarList::SetShowSwitch(bool _switch) {
	if (show_switch != _switch) {
		show_switch = _switch;
		Refresh();
	}
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
