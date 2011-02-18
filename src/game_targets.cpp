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
#include <vector>
#include "main_data.h"
#include "game_targets.h"

////////////////////////////////////////////////////////////

static std::vector<RPG::SaveTarget>& data = Main_Data::game_data.targets;

////////////////////////////////////////////////////////////
namespace Game_Targets {
	RPG::SaveTarget* FindTarget(int id, bool create);
}

////////////////////////////////////////////////////////////
RPG::SaveTarget* Game_Targets::FindTarget(int id, bool create) {
	std::vector<RPG::SaveTarget>::iterator it;
	for (it = data.begin(); it != data.end(); it++)
		if (it->ID == id)
			return &*it;
	if (!create)
		return NULL;
	data.resize(data.size() + 1);
	data.back().ID = id;
	return &data.back();
}

////////////////////////////////////////////////////////////
void Game_Targets::AddTeleportTarget(int map_id, int x, int y, int switch_id) {
	RPG::SaveTarget* target = FindTarget(map_id, true);

	target->map_id = map_id;
	target->map_x = x;
	target->map_y = y;
	target->switch_on = switch_id > 0;
	target->switch_id = switch_id;
}

////////////////////////////////////////////////////////////
void Game_Targets::RemoveTeleportTarget(int map_id) {
	RPG::SaveTarget* target = FindTarget(map_id, false);
	if (target == NULL)
		return;
	data.erase(std::vector<RPG::SaveTarget>::iterator(target));
}

////////////////////////////////////////////////////////////
RPG::SaveTarget* Game_Targets::GetTeleportTarget(int map_id) {
	return FindTarget(map_id, true);
}

////////////////////////////////////////////////////////////
void Game_Targets::SetEscapeTarget(int map_id, int x, int y, int switch_id) {
	RPG::SaveTarget* target = FindTarget(0, true);

	target->map_id = map_id;
	target->map_x = x;
	target->map_y = y;
	target->switch_on = switch_id > 0;
	target->switch_id = switch_id;
}

////////////////////////////////////////////////////////////
RPG::SaveTarget* Game_Targets::GetEscapeTarget() {
	return FindTarget(0, false);
}

