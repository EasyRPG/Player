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

#ifndef EP_GAME_ACTORS_H
#define EP_GAME_ACTORS_H

// Headers
#include <vector>
#include "game_actor.h"
#include <lcf/rpg/saveactor.h>

/**
 * Game_Actors namespace.
 */
class Game_Actors {
public:
	/**
	 * Initializes Game Actors.
	 */
	Game_Actors();

	Game_Actors(const Game_Actors&) = delete;
	Game_Actors& operator=(const Game_Actors&) = delete;

	void SetSaveData(std::vector<lcf::rpg::SaveActor> save);
	std::vector<lcf::rpg::SaveActor> GetSaveData() const;

	/**
	 * Gets an actor by its ID.
	 *
	 * @param actor_id the actor ID in the database.
	 * @return the actor object.
	 */
	Game_Actor* GetActor(int actor_id);

	/** @return the number of actors */
	int GetNumActors() const;

	/**
	 * Gets if an actor ID is valid.
	 *
	 * @param actor_id the actor ID in the database.
	 * @return whether the actor exists.
	 */
	bool ActorExists(int actor_id);

	/**
	 * Resets battle modifiers of all actors.
	 */
	void ResetBattle();

	void ReloadActors();

private:
	std::vector<Game_Actor> data;
};

inline int Game_Actors::GetNumActors() const {
	return static_cast<int>(data.size());
}

#endif
