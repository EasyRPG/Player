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

#ifndef __game_troop__
#define __game_troop__

#include <vector>
#include "game_unit.h"

class Game_Enemy;
class Game_Interpreter;

typedef std::vector<Game_Battler*> tEnemyArray;

class Game_Troop : public Game_Unit {

public:
	Game_Troop();

	void Clear();

	tEnemyArray GetMembers();

private:
	tEnemyArray enemies;
	Game_Interpreter* interpreter;
	int turn_count;
	bool can_escape, can_lose, preemptive, surprise, turn_ending;
	Game_Enemy* forcing_battler;
};
#endif // __game_troop__
