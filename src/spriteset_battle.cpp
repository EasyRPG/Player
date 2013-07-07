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
#include "spriteset_battle.h"
#include "cache.h"
#include "game_battler.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_temp.h"
#include "main_data.h"
#include "sprite_battler.h"

Spriteset_Battle::Spriteset_Battle() {
	if (!Game_Temp::battle_background.empty()) {
		background.reset(new Background(Game_Temp::battle_background));
	} else {
		background.reset(new Background(Data::terrains[Game_Temp::battle_terrain_id - 1].background_name));
	}

	// Create the enemy sprites
	boost::ptr_vector<Game_Enemy>::iterator it;
	boost::ptr_vector<Game_Enemy>& enemies = Main_Data::game_enemyparty->GetEnemies();
	for (it = enemies.begin(); it != enemies.end(); it++) {
		enemy_sprites.push_back(new Sprite_Battler(static_cast<Game_Battler*>(&*it)));
	}

	Update();
}

void Spriteset_Battle::Update() {
	boost::ptr_vector<Sprite_Battler>::iterator it;
	for (it = enemy_sprites.begin(); it != enemy_sprites.end(); it++) {
		it->Update();
	}
}

// Finds the sprite for a specific character
Sprite_Battler* Spriteset_Battle::FindBattler(const Game_Battler* battler)
{
	boost::ptr_vector<Sprite_Battler>::iterator it;
	for (it = enemy_sprites.begin(); it != enemy_sprites.end(); it++) {
		if (it->GetBattler() == battler)
			return &*it;
	}
	return NULL;
}
