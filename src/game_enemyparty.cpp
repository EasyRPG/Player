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
#include <cassert>
#include "game_interpreter.h"
#include "game_enemyparty.h"
#include "main_data.h"
#include "utils.h"

Game_EnemyParty::Game_EnemyParty() {
}

Game_Enemy& Game_EnemyParty::operator[] (const int index) {
	if (index < 0 || index >= (int)enemies.size()) {
		assert(false && "Subscript out of range");
	}

	return *enemies[index];
}

int Game_EnemyParty::GetBattlerCount() const {
	return (int)enemies.size();
}

void Game_EnemyParty::Setup(int battle_troop_id) {
	enemies.clear();
	troop = &Data::troops[battle_troop_id - 1];

	int non_hidden = 0;
	for (const RPG::TroopMember& mem : troop->members) {
		non_hidden += (!mem.invisible ? 1 : 0);
	}

	for (const RPG::TroopMember& mem : troop->members) {
		std::shared_ptr<Game_Enemy> enemy = std::make_shared<Game_Enemy>(mem.enemy_id);
		enemy->SetBattleX(mem.x);
		enemy->SetBattleY(mem.y);

		if (!mem.invisible) {
			if (troop->appear_randomly) {
				// At least one party member must be visible
				if (non_hidden > 1) {
					bool hide = Utils::ChanceOf(1, 2);
					enemy->SetHidden(hide);
					non_hidden -= (hide ? 1 : 0);
				}
			} else {
				enemy->SetHidden(false);
			}
		} else {
			enemy->SetHidden(true);
		}

		enemies.push_back(enemy);
	}
}

std::vector<std::shared_ptr<Game_Enemy> >& Game_EnemyParty::GetEnemies() {
	return enemies;
}

int Game_EnemyParty::GetExp() const {
	std::vector<std::shared_ptr<Game_Enemy> >::const_iterator it;
	int sum = 0;
	for (it = enemies.begin(); it != enemies.end(); ++it) {
		if ((*it)->IsDead()) {
			sum += (*it)->GetExp();
		}
	}
	return sum;
}

int Game_EnemyParty::GetMoney() const {
	std::vector<std::shared_ptr<Game_Enemy> >::const_iterator it;
	int sum = 0;
	for (it = enemies.begin(); it != enemies.end(); ++it) {
		if ((*it)->IsDead()) {
			sum += (*it)->GetMoney();
		}
	}
	return sum;
}

void Game_EnemyParty::GenerateDrops(std::vector<int>& out) const {
	std::vector<std::shared_ptr<Game_Enemy> >::const_iterator it;
	for (it = enemies.begin(); it != enemies.end(); ++it) {
		if ((*it)->IsDead()) {
			// Only roll if the enemy has something to drop
			if ((*it)->GetDropId() != 0) {
				if (Utils::ChanceOf((*it)->GetDropProbability(), 100)) {
					out.push_back((*it)->GetDropId());
				}
			}
		}
	}
}
