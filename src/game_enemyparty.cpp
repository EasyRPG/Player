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
#include <algorithm>
#include "game_interpreter.h"
#include "game_enemyparty.h"
#include "main_data.h"
#include <lcf/reader_util.h>
#include "utils.h"
#include "output.h"
#include "rand.h"

Game_EnemyParty::Game_EnemyParty() {
}

Game_Enemy* Game_EnemyParty::GetEnemy(int idx) {
	if (idx >= 0 && idx < static_cast<int>(enemies.size())) {
		return &enemies[idx];
	}
	return nullptr;
}

Game_Enemy& Game_EnemyParty::operator[] (const int index) {
	if (index < 0 || index >= (int)enemies.size()) {
		assert(false && "Subscript out of range");
	}

	return enemies[index];
}

int Game_EnemyParty::GetBattlerCount() const {
	return static_cast<int>(enemies.size());
}

int Game_EnemyParty::GetVisibleBattlerCount() const {
	int visible = 0;
	for (const auto& enemy: enemies) {
		visible += !enemy.IsHidden();
	}
	return visible;
}

void Game_EnemyParty::ResetBattle(int battle_troop_id) {
	enemies.clear();
	const auto* troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, battle_troop_id);
	if (!troop) {
		// Valid case when battle quits
		return;
	}

	int non_hidden = static_cast<int>(troop->members.size());
	for (const auto& mem : troop->members) {
		enemies.emplace_back(&mem);
		non_hidden -= enemies.back().IsHidden();
	}

	if (troop->appear_randomly) {
		for (auto& enemy: enemies) {
			if (non_hidden <= 1) {
				// At least one party member must be visible
				break;
			}
			if (enemy.IsHidden()) {
				continue;
			}

			if (Rand::PercentChance(40)) {
				enemy.SetHidden(true);
				--non_hidden;
			}
		}
	}
}

std::vector<Game_Enemy*> Game_EnemyParty::GetEnemies() {
	std::vector<Game_Enemy*> party;
	party.reserve(enemies.size());
	for (auto& e: enemies) {
		party.push_back(&e);
	}
	return party;
}


int Game_EnemyParty::GetExp() const {
	int sum = 0;
	for (auto& enemy: enemies) {
		if (enemy.IsDead()) {
			sum += enemy.GetExp();
		}
	}
	return sum;
}

int Game_EnemyParty::GetMoney() const {
	int sum = 0;
	for (auto& enemy: enemies) {
		if (enemy.IsDead()) {
			sum += enemy.GetMoney();
		}
	}
	return sum;
}

void Game_EnemyParty::GenerateDrops(std::vector<int>& out) const {
	for (auto& enemy: enemies) {
		if (enemy.IsDead()) {
			// Only roll if the enemy has something to drop
			if (enemy.GetDropId() != 0) {
				if (Rand::ChanceOf(enemy.GetDropProbability(), 100)) {
					out.push_back(enemy.GetDropId());
				}
			}
		}
	}
}

int Game_EnemyParty::GetEnemyPositionInParty(const Game_Enemy* enemy) const {
	if (enemy >= enemies.data() && enemy < enemies.data() + enemies.size()) {
		return enemy - enemies.data();
	}
	return -1;
}
