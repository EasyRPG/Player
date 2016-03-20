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
#include "game_battler.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_temp.h"
#include "main_data.h"
#include "player.h"
#include "sprite_battler.h"

Spriteset_Battle::Spriteset_Battle() {
	background.reset(new Background(Game_Temp::battle_background));

	// Create the sprites
	std::vector<Game_Battler*> battler;
	Main_Data::game_enemyparty->GetBattlers(battler);
	if (Player::IsRPG2k3()) {
		for (int i = 0; i < Data::actors.size(); ++i) {
			battler.push_back(Game_Actors::GetActor(i + 1));
		}
	}

	for (Game_Battler* b : battler) {
		sprites.push_back(EASYRPG_MAKE_SHARED<Sprite_Battler>(b));
		if (b->GetType() == Game_Battler::Type_Ally) {
			sprites.back()->SetVisible(false);
		}
	}

	timer1.reset(new Sprite_Timer(0));
	timer2.reset(new Sprite_Timer(1));

	Update();
}

void Spriteset_Battle::Update() {
	for (auto sprite : sprites) {
		Game_Battler* battler = sprite->GetBattler();
		if (battler->GetType() == Game_Battler::Type_Ally) {
			sprite->SetVisible(Main_Data::game_party->IsActorInParty(battler->GetId()));
		}

		sprite->Update();
	}

	timer1->Update();
	timer2->Update();
}

Sprite_Battler* Spriteset_Battle::FindBattler(const Game_Battler* battler)
{
	std::vector<EASYRPG_SHARED_PTR<Sprite_Battler> >::iterator it;
	for (it = sprites.begin(); it != sprites.end(); ++it) {
		if ((*it)->GetBattler() == battler)
			return it->get();
	}
	return NULL;
}
