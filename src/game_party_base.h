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

#ifndef _GAME_PARTY_BASE_H_
#define _GAME_PARTY_BASE_H_

// Headers
#include <vector>
#include <map>
#include "game_actor.h"
#include "main_data.h"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

/**
 * Base class of the two Parties (Allied and Enemy)
 */
class Game_Party_Base {
public:
	/**
	 * Gets a battler from the party by position in the party
	 *
	 * @param int Index of member to return
	 * @return Party battler
	 */
	virtual Game_Battler& operator[] (const int index) = 0;

	/**
	 * Returns how many members are in the party
	 *
	 * @return Number of members in the party
	 */
	virtual int GetBattlerCount() const = 0;

	/**
	 * Returns a list of all battlers in the party
	 *
	 * @param out List of all battlers
	 */
	virtual void GetBattlers(std::vector<Game_Battler*>& out);

	/**
	 * Returns a list of all alive battlers in the party
	 *
	 * @param out List of all dead battlers
	 */
	virtual void GetAliveBattlers(std::vector<Game_Battler*>& out);

	/**
	 * Returns a list of all dead battlers in the party
	 *
	 * @param out List of all dead battlers
	 */
	virtual void GetDeadBattlers(std::vector<Game_Battler*>& out);

	/**
	 * Return the next alive battler in the party based on the passed battler.
	 *
	 * @param battler Battler
	 * @return Battler after the provided one, NULL if battler isn't in party at all.
	 */
	virtual Game_Battler* GetNextAliveBattler(Game_Battler* battler);

	/**
	 * Gets a random alive battler from the party
	 * @return Random alive battler
	 */
	virtual Game_Battler* GetRandomAliveBattler();

	/**
	 * Gets a random dead battler from the party
	 *
	 * @return Random dead battler
	 */
	virtual Game_Battler* GetRandomDeadBattler();

	/**
	 * Tests if all party members are dead.
	 *
	 * @return Whether all are dead.
	 */
	virtual bool IsAnyAlive();

	/**
	 * Gets average agility of the party (for battle)
	 *
	 * @return average agility
	 */
	virtual int GetAverageAgility();

private:

};

#endif
