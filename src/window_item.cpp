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
#include "window_item.h"

////////////////////////////////////////////////////////////
Window_Item::Window_Item(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
}

////////////////////////////////////////////////////////////
Window_Item::~Window_Item() {
}

////////////////////////////////////////////////////////////
int Window_Item::GetItem() {
	return data[index];
}

////////////////////////////////////////////////////////////
bool Window_Item::CheckInclude(int item_id) {
	return true;
}

////////////////////////////////////////////////////////////
bool Window_Item::CheckEnable(int item_id) {
	return true;
}

////////////////////////////////////////////////////////////
void Window_Item::Refresh() {
	data.clear();

}

////////////////////////////////////////////////////////////
void Window_Item::DrawItem(int index) {
}

////////////////////////////////////////////////////////////
void Window_Item::UpdateHelp() {
}

////////////////////////////////////////////////////////////
void Window_Item::SetHelpWindow(Window_Help* help_window) {
	this->help_window = help_window;
}
