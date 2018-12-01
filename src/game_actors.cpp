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

namespace {
	std::vector<std::shared_ptr<Game_Actor> > data;
}

void Game_Actors::Init() {
	data.resize(Data::actors.size());
	for (size_t i = 1; i <= data.size(); i++)
		GetActor(i)->Init();
}

void Game_Actors::Fixup() {
	// Ensure actor save data and LDB actors has correct size
	if (Main_Data::game_data.actors.size() != data.size()) {
		size_t save_actor_size = Main_Data::game_data.actors.size();

		Output::Warning("Actor array size doesn't match Savegame actor array size (%d != %d)",
						data.size(), save_actor_size);

		Main_Data::game_data.actors.resize(data.size());

		// When the save data size is smaller than the LDB size set the additional actors to nullptr.
		// GetActor will copy the actor data to the savegame data next time it is invoked.
		if (save_actor_size < data.size()) {
			std::fill(data.begin() + save_actor_size, data.end(), nullptr);
		}
	}

	for (size_t i = 1; i <= data.size(); ++i) {
		GetActor(i)->Fixup();
	}
}

void Game_Actors::Dispose() {
	data.clear();
}

Game_Actor* Game_Actors::GetActor(int actor_id) {
	if (!ActorExists(actor_id)) {
		return nullptr;
	} else if (!data[actor_id - 1]) {
		data[actor_id - 1].reset(new Game_Actor(actor_id));
	}

	return data[actor_id - 1].get();
}

bool Game_Actors::ActorExists(int actor_id) {
	return actor_id > 0 && (size_t)actor_id <= data.size();
}
