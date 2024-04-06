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
#include "window_equipitem.h"
#include "game_actors.h"
#include "game_party.h"
#include <lcf/reader_util.h>
#include "output.h"

Window_EquipItem::Window_EquipItem(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id, int equip_type) :
	Window_Item(parent, ix, iy, iwidth, iheight),
	actor_id(actor_id) {
	this->equip_type = equip_type;
	if (equip_type > 4 || equip_type < 0) {
		this->equip_type = Window_EquipItem::other;
	}

	if (this->equip_type == Window_EquipItem::shield &&
		Main_Data::game_actors->GetActor(actor_id)->HasTwoWeapons()) {

		this->equip_type = Window_EquipItem::weapon;
	}
}

bool Window_EquipItem::CheckInclude(int item_id) {
	// Do not show equippable items if the actor has its equipment fixed
	if (Main_Data::game_actors->GetActor(actor_id)->IsEquipmentFixed(false)) {
		return false;
	}

	// Add the empty element
	if (item_id == 0) {
		return true;
	}

	bool result = false;

	// Equipment and items are guaranteed to be valid
	lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);

	switch (equip_type) {
	case Window_EquipItem::weapon:
		result = item->type == lcf::rpg::Item::Type_weapon;
		break;
	case Window_EquipItem::shield:
		result = item->type == lcf::rpg::Item::Type_shield;
		break;
	case Window_EquipItem::armor:
		result = item->type == lcf::rpg::Item::Type_armor;
		break;
	case Window_EquipItem::helmet:
		result = item->type == lcf::rpg::Item::Type_helmet;
		break;
	case Window_EquipItem::other:
		result = item->type == lcf::rpg::Item::Type_accessory;
		break;
	default:
		return false;
	}

	if (result) {
		// Check if the party has the item at least once
		if (Main_Data::game_party->GetItemCount(item_id) == 0) {
			return false;
		} else {
			return Main_Data::game_actors->GetActor(actor_id)->IsEquippable(item_id);
		}
	} else {
		return false;
	}
}

bool Window_EquipItem::CheckEnable(int item_id) {
	(void)item_id;
	return true;
}
