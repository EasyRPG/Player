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
#include "game_interpreter.h"
#include "game_enemyparty.h"
#include "game_enemy.h"
#include "main_data.h"
#include "rpg_enemy.h"

Game_EnemyParty_Class::Game_EnemyParty_Class() {
	//interpreter = NULL;
}

void Game_EnemyParty_Class::Setup(int battle_troop_id) {
	troop = &Data::troops[battle_troop_id - 1];
	std::vector<RPG::TroopMember>::const_iterator ei;
	for (ei = troop->members.begin(); ei != troop->members.end(); ei++)	{
		Game_Enemy* enemy = new Game_Enemy(ei->enemy_id);
		enemy->SetBattleX(ei->x);
		enemy->SetBattleY(ei->y);
		enemy->SetHidden(ei->invisible);
		enemies.push_back(enemy);
	}
}

boost::ptr_vector<Game_Enemy>& Game_EnemyParty_Class::GetEnemies() {
	return enemies;
}

std::vector<Game_Enemy*> Game_EnemyParty_Class::GetAliveEnemies() {
	std::vector<Game_Enemy*> alive;
	boost::ptr_vector<Game_Enemy>::iterator it;
	for (it = enemies.begin(); it != enemies.end(); ++it) {
		if (!it->IsDead()) {
			alive.push_back(&*it);
		}
	}
	return alive;
}

void Game_EnemyParty_Class::Clear() {
	interpreter->Clear();

	enemies.clear();
}

Game_EnemyParty_Class& Game_EnemyParty() {
	static bool init = false;
	static boost::scoped_ptr<Game_EnemyParty_Class> instance;
	if (!init) {
		instance.reset(new Game_EnemyParty_Class());
		init = true;
	}
	return *instance;
}
