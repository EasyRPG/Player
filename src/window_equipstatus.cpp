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
#include "window_equipstatus.h"

////////////////////////////////////////////////////////////
Window_EquipStatus::Window_EquipStatus(int actor_id) :
	Window_Base(0, 32, 124, 96),
	actor_id(actor_id),
	draw_params(false) {
	contents = new Bitmap(width - 16, height - 16);
	Refresh();
}

////////////////////////////////////////////////////////////
Window_EquipStatus::~Window_EquipStatus() {
}

////////////////////////////////////////////////////////////
void Window_EquipStatus::Refresh() {
	// ToDo
}

////////////////////////////////////////////////////////////
void Window_EquipStatus::SetNewParameters(
	int new_atk, int new_def, int new_spi, int new_agi) {
	draw_params = true;
	atk = new_atk;
	def = new_def;
	spi = new_spi;
	agi = new_agi;
}

////////////////////////////////////////////////////////////
void Window_EquipStatus::ClearParameters() {
	draw_params = false;
}
