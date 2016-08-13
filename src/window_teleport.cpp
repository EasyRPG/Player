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
#include <vector>
#include "window_teleport.h"
#include "bitmap.h"
#include "font.h"
#include "game_map.h"
#include "game_targets.h"

Window_Teleport::Window_Teleport(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 2;

	Refresh();
}

const RPG::SaveTarget& Window_Teleport::GetTarget() const {
	std::vector<RPG::SaveTarget*> targets = Game_Targets::GetTeleportTargets();

	return *targets[GetIndex()];
}

void Window_Teleport::Refresh() {
	std::vector<RPG::SaveTarget*> targets = Game_Targets::GetTeleportTargets();
	item_max = (int)targets.size();
	CreateContents();

	for (size_t i = 0; i < targets.size(); ++i) {
		Rect rect = GetItemRect(i);
		contents->ClearRect(rect);

		contents->TextDraw(rect, Font::ColorDefault, Game_Map::GetMapName(targets[i]->map_id));
	}
}
