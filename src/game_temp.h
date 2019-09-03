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

#ifndef EP_GAME_TEMP_H
#define EP_GAME_TEMP_H

// Headers
#include <string>
#include "game_battler.h"
#include "transition.h"

/**
 * Game Temp static class.
 */
class Game_Temp {
public:
	/**
	 * Initializes Game Temp.
	 */
	static void Init();

	static bool inn_calling;

	static bool shop_buys;
	static bool shop_sells;
	static int shop_type;		// message set A, B, or C
	static std::vector<int> shop_goods;
	static bool shop_transaction;
	static int inn_price;

	static std::string hero_name;
	static int hero_name_id;
	static int hero_name_charset;

	static bool battle_running;
	static int battle_troop_id;
	static std::string battle_background;
	static int battle_formation;
	static int battle_escape_mode;
	static int battle_defeat_mode;
	static bool battle_first_strike;
	static int battle_result;
	static bool battle_random_encounter;

	enum BattleResult {
		BattleVictory,
		BattleEscape,
		BattleDefeat,
		BattleAbort
	};

private:
	Game_Temp();
};

#endif
