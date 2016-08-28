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

#ifndef _SPRITESET_BATTLE_H_
#define _SPRITESET_BATTLE_H_

// Headers
#include "background.h"
#include "screen.h"
#include "sprite_battler.h"
#include "sprite_character.h"
#include "sprite_timer.h"

class Game_Battler;
/**
 * Spriteset_Battle class.
 */
class Spriteset_Battle {
public:
	Spriteset_Battle();

	void Update();
	Sprite_Battler* FindBattler(const Game_Battler* battler);

protected:
	std::unique_ptr<Background> background;
	std::vector<std::shared_ptr<Sprite_Battler>> sprites;
	std::string background_name;
	std::unique_ptr<Screen> screen;

	std::unique_ptr<Sprite_Timer> timer1;
	std::unique_ptr<Sprite_Timer> timer2;
};

#endif
