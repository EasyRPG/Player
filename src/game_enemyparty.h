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

#ifndef EP_GAME_ENEMYPARTY_H
#define EP_GAME_ENEMYPARTY_H

#include <vector>
#include <lcf/rpg/troop.h>
#include "game_enemy.h"
#include "game_party_base.h"

/**
 * Manages the enemy party during battles.
 */
class Game_EnemyParty : public Game_Party_Base {
public:
	/**
	 * Initializes Game_Enemy_Party.
	 */
	Game_EnemyParty();

	Game_Enemy& operator[] (const int index) override;

	int GetBattlerCount() const override;
	int GetVisibleBattlerCount() const override;

	/**
	 * Reset the party for a new battle.
	 * 
	 * @param battle_troop_id ID of the enemy party, or 0 for empty.
	 */
	void ResetBattle(int battle_troop_id);

	/**
	 * Gets a list with all party members
	 *
	 * @return list of party members
	 */
	std::vector<Game_Enemy*> GetEnemies();

	/**
	 * Sums up the experience points of all enemy party members.
	 *
	 * @return All experience points
	 */
	int GetExp() const;

	/**
	 * Sums up the money of all enemy party members.
	 *
	 * @return All money
	 */
	int GetMoney() const;

	/**
	 * Rolls once for each enemy's drops.
	 *
	 * @param out List of the dropped items' IDs
	 */
	void GenerateDrops(std::vector<int>& out) const;

	/**
	 * Get the enemy by party index.
	 *
	 * @param idx the index of the enemy.
	 * @return enemy from party
	 */
	Game_Enemy* GetEnemy(int idx);

	/** 
	 * Get the position index of the enemy in the party.
	 *
	 * @param enemy the enemy to query.
	 *
	 * @return 0 based index, or -1 if not found
	 */
	int GetEnemyPositionInParty(const Game_Enemy* enemy) const;

private:
	std::vector<Game_Enemy> enemies;
};

#endif
