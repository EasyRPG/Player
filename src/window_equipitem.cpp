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
#include "window_equipitem.h"
#include "game_actors.h"

////////////////////////////////////////////////////////////
Window_EquipItem::Window_EquipItem(int actor_id, int equip_type) :
	Window_Item(0, 128, 320, 112),
	actor_id(actor_id) {
	this->equip_type = (EquipType)equip_type;
	if (equip_type > 4 || equip_type < 0) {
		this->equip_type = Window_EquipItem::other;
	}

	if (this->equip_type == Window_EquipItem::shield &&
		Game_Actors::GetActor(actor_id)->GetTwoSwordsStyle()) {
		
		this->equip_type = Window_EquipItem::weapon;
	}
}

////////////////////////////////////////////////////////////
Window_EquipItem::~Window_EquipItem() {
}

////////////////////////////////////////////////////////////
bool Window_EquipItem::CheckInclude(int item_id) {
	return true;
}

////////////////////////////////////////////////////////////
bool Window_EquipItem::CheckEnable(int item_id) {
	return true;
}
