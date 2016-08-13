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
#include "main_data.h"
#include "game_targets.h"

static std::vector<RPG::SaveTarget>& data = Main_Data::game_data.targets;

namespace Game_Targets {
	std::vector<RPG::SaveTarget>::iterator FindTarget(int id, bool create);
}

std::vector<RPG::SaveTarget>::iterator Game_Targets::FindTarget(int id, bool create) {
	std::vector<RPG::SaveTarget>::iterator it;
	for (it = data.begin(); it != data.end(); ++it)
		if (it->ID == id)
			return it;
	if (!create)
		return data.end();
	data.resize(data.size() + 1);
	data.back().ID = id;
	return data.end() - 1;
}

void Game_Targets::AddTeleportTarget(int map_id, int x, int y, int switch_id) {
	std::vector<RPG::SaveTarget>::iterator target = FindTarget(map_id, true);

	target->map_id = map_id;
	target->map_x = x;
	target->map_y = y;
	target->switch_on = switch_id > 0;
	target->switch_id = switch_id;
}

void Game_Targets::RemoveTeleportTarget(int map_id) {
	std::vector<RPG::SaveTarget>::iterator target = FindTarget(map_id, false);
	if (target == data.end())
		return;
	data.erase(target);
}

bool Game_Targets::HasTeleportTarget() {
	// Escape target has ID 0

	if (data.empty()) {
		return false;
	}

	if (data.size() > 1) {
		return true;
	}

	return data[0].ID != 0;
}

RPG::SaveTarget* Game_Targets::GetTeleportTarget(int map_id) {
	std::vector<RPG::SaveTarget>::iterator target = FindTarget(map_id, false);
	return target == data.end() ? NULL : &*target;
}

std::vector<RPG::SaveTarget*> Game_Targets::GetTeleportTargets() {
	std::vector<RPG::SaveTarget*> targets;

	for (auto& target : data) {
		if (target.ID != 0) {
			targets.push_back(&target);
		}
	}

	return targets;
}

void Game_Targets::SetEscapeTarget(int map_id, int x, int y, int switch_id) {
	std::vector<RPG::SaveTarget>::iterator target = FindTarget(0, true);

	target->map_id = map_id;
	target->map_x = x;
	target->map_y = y;
	target->switch_on = switch_id > 0;
	target->switch_id = switch_id;
}

bool Game_Targets::HasEscapeTarget() {
	return GetEscapeTarget() != nullptr;
}

RPG::SaveTarget* Game_Targets::GetEscapeTarget() {
	std::vector<RPG::SaveTarget>::iterator target = FindTarget(0, false);
	return target == data.end() ? NULL : &*target;
}

