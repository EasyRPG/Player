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
#include "window_equip.h"
#include "game_actors.h"
#include "bitmap.h"

Window_Equip::Window_Equip(int ix, int iy, int iwidth, int iheight, int actor_id) :
	Window_Selectable(ix, iy, iwidth, iheight),
	actor_id(actor_id) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	index = 0;

	Refresh();
}

int Window_Equip::GetItemId() {
	return index < 0 ? 0 : data[index];
}

void Window_Equip::Refresh() {
	contents->Clear();

	// Add the equipment of the actor to data
	data.clear();
	Game_Actor* actor = Game_Actors::GetActor(actor_id);
	for (int i = 1; i <= 5; ++i) {
		const RPG::Item* item = actor->GetEquipment(i);
		data.push_back(item ? item->ID : 0);
	}
	item_max = data.size();

	// Draw equipment text
	for (int i = 0; i < 5; ++i) {
		DrawEquipmentType(actor, 0, (12 + 4) * i + 2, i);
		if (data[i] > 0) {
			DrawItemName(&Data::items[data[i] - 1], 60, (12 + 4) * i + 2);
		}
	}
}

void Window_Equip::UpdateHelp() {
	help_window->SetText(GetItemId() == 0 ? "" : Data::items[GetItemId() - 1].description);
}
