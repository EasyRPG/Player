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

#include "rpg_troop.h"

class Game_Interpreter;
class Spriteset_Battle;

namespace Game_Battle {
	/**
	 * Initialize Game_Battle.
	 */
	void Init();

	/**
	 * Quits (frees) Game_Battle.
	 */
	void Quit();

	/**
	 * Updates the battle state.
	 */
	void Update();
	void Terminate();

	Spriteset_Battle& GetSpriteset();

	void NextTurn();

	/**
	 * Updates the gauge of all battlers based on the highest agi of all.
	 */
	void UpdateGauges();

	void ChangeBackground(const std::string& name);

	int GetTurn();
	bool CheckTurns(int turns, int base, int multiple);

	bool AreConditionsMet(const RPG::TroopPageCondition& condition);
	void UpdateEvents();

	bool IsEscapeAllowed();

	static int turn;
	static bool message_is_fixed;
	static int message_position;
	extern bool terminate;
	extern std::string background_name;

	extern int escape_fail_count;
}
