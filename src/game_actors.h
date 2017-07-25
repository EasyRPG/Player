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
#include "rpg_saveactor.h"

/**
 * Game_Actors namespace.
 */
namespace Game_Actors {
	/**
	 * Initializes Game Actors.
	 */
	void Init();

	/**
	 * Used after savegame loading to replace savegame default values with
	 * database ones.
	 */
	void Fixup();

	/**
	 * Disposes Game Actors.
	 */
	void Dispose();

	/**
	 * Gets an actor by its ID.
	 *
	 * @param actor_id the actor ID in the database.
	 * @return the actor object.
	 */
	Game_Actor* GetActor(int actor_id);

	/**
	 * Gets if an actor ID is valid.
	 *
	 * @param actor_id the actor ID in the database.
	 * @return whether the actor exists.
	 */
	bool ActorExists(int actor_id);
}

#endif
