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

#ifndef __GAME_ENEMY_PARTY__
#define __GAME_ENEMY_PARTY__

// Headers
#include <vector>
#include "rpg_troop.h"
#include <boost/ptr_container/ptr_vector.hpp>

class Game_Enemy;
class Game_Interpreter;

/**
 * Manages the enemy party during battles.
 */
namespace Game_EnemyParty {
	/**
	 * Initializes Game_Enemy_Party.
	 */
	void Init();

	/**
	 * Setups initial enemy party.
	 * 
	 * @param battle_troop_id ID of the enemy party
	 */
	void Setup(int battle_troop_id);

	/**
	 * Clears the party
	 */
	void Clear();

	/**
	 * Gets a list with all party members
	 *
	 * @return list of party members
	 */
	boost::ptr_vector<Game_Battler>& GetEnemies();
}

#endif
