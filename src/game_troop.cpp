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

#include "game_interpreter.h"
#include "game_troop.h"
#include "rpg_enemy.h"

Game_Troop::Game_Troop():
	interpreter(NULL) {
}

Game_Troop::~Game_Troop()
{
}

tEnemyArray Game_Troop::GetMembers() {
	return enemies;
}

void Game_Troop::Clear() {
	interpreter->Clear();

	enemies.clear();

	turn_count = 0;
	can_escape = false;
	can_lose = false;
	preemptive = false;
	surprise = false;
	turn_ending = false;
	forcing_battler = NULL;
}
