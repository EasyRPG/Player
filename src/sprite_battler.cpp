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
#include "sprite_battler.h"
#include "game_battler.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "output.h"

Sprite_Battler::Sprite_Battler(Game_Battler* battler, int index) :
	battler(battler), battle_index(index) {
}

Sprite_Battler::~Sprite_Battler() {
}

void Sprite_Battler::ResetZ() {
	static_assert(Game_Battler::Type_Ally < Game_Battler::Type_Enemy, "Game_Battler enums re-ordered! Fix Z order logic here!");

	constexpr int id_limit = 128;

	const auto& graphic = GetBitmap();
	int y = battler->GetBattlePosition().y;
	if (battler->GetType() == Game_Battler::Type_Enemy && graphic) {
		y += graphic->GetHeight() / 2;
	} else if (battler->GetType() == Game_Battler::Type_Ally) {
		y += 24;
	}

	Drawable::Z_t z = battler->GetType();
	z += y;
	z *= id_limit;
	z += id_limit - battle_index;
	z += Priority_Battler;

	SetZ(z);
}

