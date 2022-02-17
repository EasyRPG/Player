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
#include "system.h"
#include <vector>
#include "game_actors.h"
#include "main_data.h"
#include "output.h"

Game_Actors::Game_Actors() {
	data.reserve(lcf::Data::actors.size());
	for (size_t i = 0; i < lcf::Data::actors.size(); i++) {
		data.emplace_back(Game_Actor(i + 1));
	}
}

void Game_Actors::SetSaveData(std::vector<lcf::rpg::SaveActor> save) {
	// Ensure actor save data and LDB actors has correct size
	if (save.size() > data.size()) {
		Output::Warning("Game_Actors: Save game array size {} is larger than number of LDB actors {} : Dropping extras ...", save.size(), data.size());
	}

	for (size_t i = 0; i < std::min(save.size(), data.size()); ++i) {
		data[i].SetSaveData(std::move(save[i]));
	}
}

std::vector<lcf::rpg::SaveActor> Game_Actors::GetSaveData() const {
	std::vector<lcf::rpg::SaveActor> save;
	save.reserve(data.size());
	for (auto& actor: data) {
		save.push_back(actor.GetSaveData());
	}
	return save;
}

Game_Actor* Game_Actors::GetActor(int actor_id) {
	if (!ActorExists(actor_id)) {
		return nullptr;
	}

	return &data[actor_id - 1];
}

bool Game_Actors::ActorExists(int actor_id) {
	return actor_id > 0 && actor_id <= static_cast<int>(data.size());
}

void Game_Actors::ResetBattle() {
	for (auto& actor: data) {
		actor.ResetBattle();
	}
}

void Game_Actors::ReloadActors() {
	for (auto& actor: data) {
		actor.ReloadDbActor();
	}
}
