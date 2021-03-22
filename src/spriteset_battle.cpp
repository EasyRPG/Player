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
#include "game_actors.h"
#include "game_battle.h"
#include "game_battler.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_screen.h"
#include "main_data.h"
#include "player.h"
#include "sprite_battler.h"
#include "sprite_actor.h"
#include "sprite_enemy.h"

Spriteset_Battle::Spriteset_Battle(const std::string bg_name, int terrain_id)
{
	background_name = std::move(bg_name);

	// Create background
	if (!background_name.empty()) {
		background.reset(new Background(background_name));
	} else {
		// Background verifies that the Terrain ID is valid
		background.reset(new Background(terrain_id));
	}
	Game_Battle::ChangeBackground(background_name);

	// Create the sprites
	std::vector<Game_Battler*> battler;
	Main_Data::game_enemyparty->GetBattlers(battler);
	if (Player::IsRPG2k3()) {
		for (unsigned int i = 0; i < lcf::Data::actors.size(); ++i) {
			battler.push_back(Main_Data::game_actors->GetActor(i + 1));
		}
	}

	timer1.reset(new Sprite_Timer(0));
	timer2.reset(new Sprite_Timer(1));

	screen.reset(new Screen());
}

void Spriteset_Battle::Update() {
	Tone new_tone = Main_Data::game_screen->GetTone();

	// Handle background change
	const auto& current_bg = Game_Battle::GetBackground();
	if (background_name != current_bg) {
		background_name = current_bg;
		if (!background_name.empty()) {
			background.reset(new Background(background_name));
		} else {
			background.reset();
		}
	}
	background->SetTone(new_tone);
	background->Update();
}
