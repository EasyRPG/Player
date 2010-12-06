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
#include "window_equip.h"

////////////////////////////////////////////////////////////
Window_Equip::Window_Equip(int actor_id) :
	Window_Selectable(124, 32, 196, 96),
	actor_id(actor_id),
	help_window(NULL) {

}

////////////////////////////////////////////////////////////
Window_Equip::~Window_Equip() {
}

////////////////////////////////////////////////////////////
void Window_Equip::Refresh() {

}

////////////////////////////////////////////////////////////
void Window_Equip::SetHelpWindow(Window_Help* help_window) {
	this->help_window = help_window;
}

////////////////////////////////////////////////////////////
void Window_Equip::UpdateHelp() {
}
