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
#include "game_targets.h"
#include <algorithm>

template <typename T>
static auto FindTarget(T&& targets, int map_id) {
	return std::find_if(targets.begin(), targets.end(),
			[map_id](auto& tgt) { return tgt.map_id == map_id; }
			);
}

void Game_Targets::AddTeleportTarget(int map_id, int x, int y, bool switch_on, int switch_id) {
	auto iter = FindTarget(teleports, map_id);
	if (iter == teleports.end()) {
		lcf::rpg::SaveTarget tgt;
		// RPG_RT duplicates the map_id into the save object's id.
		tgt.ID = map_id;
		iter = teleports.insert(iter, std::move(tgt));
	}

	iter->map_id = map_id;
	iter->map_x = x;
	iter->map_y = y;
	iter->switch_on = switch_on;
	iter->switch_id = switch_id;

	// Teleports must be sorted by map id.
	std::sort(teleports.begin(), teleports.end(), [](auto& l, auto& r) { return l.map_id < r.map_id; });
}

void Game_Targets::RemoveTeleportTarget(int map_id) {
	auto iter = FindTarget(teleports, map_id);
	if (iter != teleports.end()) {
		teleports.erase(iter);
	}
}

const lcf::rpg::SaveTarget* Game_Targets::GetTeleportTarget(int map_id) const {
	auto iter = FindTarget(teleports, map_id);
	return (iter != teleports.end()) ? &*iter : nullptr;
}

void Game_Targets::SetEscapeTarget(int map_id, int x, int y, bool switch_on, int switch_id) {
	escape.map_id = map_id;
	escape.map_x = x;
	escape.map_y = y;
	escape.switch_on = switch_on;
	escape.switch_id = switch_id;
}

void Game_Targets::SetSaveData(std::vector<lcf::rpg::SaveTarget> save) {
	for (auto& data: save) {
		if (data.ID == 0) {
			escape = std::move(data);
		} else {
			teleports.push_back(data);
			// Protect against bad save data. Teleports must be sorted by map id.
			std::sort(teleports.begin(), teleports.end(), [](auto& l, auto& r) { return l.map_id < r.map_id; });
		}
	}
}

std::vector<lcf::rpg::SaveTarget> Game_Targets::GetSaveData() const {
	std::vector<lcf::rpg::SaveTarget> save;
	save.push_back(escape);
	save.insert(save.end(), teleports.begin(), teleports.end());
	return save;
}

