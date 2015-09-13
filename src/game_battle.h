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

class Game_Battler;
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

	/**
	 * Plays a battle animation against the given target.
	 *
	 * @param animation_id the animation ID
	 * @param target pointer to the battler to play against
	 * @param flash whether or not the screen should flash during the animation
	 */
	void ShowBattleAnimation(int animation_id, Game_Battler* target, bool flash = true);

	/**
	 * Plays a battle animation against several targets simultaneously.
	 *
	 * @param animation_id the animation ID
	 * @param targets a vector of pointer to the battlers to play against
	 * @param flash whether or not the screen should flash during the animation
	 */
	void ShowBattleAnimation(int animation_id, const std::vector<Game_Battler*>& targets, bool flash = true);

	/**
	 * Whether or not a battle animation is currently playing.
	 */
	bool IsBattleAnimationWaiting();

	void NextTurn();

	/**
	 * Updates the gauge of all battlers based on the highest agi of all.
	 */
	void UpdateGauges();

	void ChangeBackground(const std::string& name);

	int GetTurn();
	bool CheckTurns(int turns, int base, int multiple);

	bool AreConditionsMet(const RPG::TroopPageCondition& condition);
	bool UpdateEvents();

	bool IsEscapeAllowed();
	bool IsTerminating();

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter& GetInterpreter();

	void SetTerrainId(int terrain_id_);
	int GetTerrainId();

	enum BattleMode {
		BattleNormal,
		BattleInitiative,
		BattleSurround,
		BattleBackAttack,
		BattlePincer
	};

	void SetBattleMode(int battle_mode_);
	int GetBattleMode();

	extern int escape_fail_count;
	extern std::string background_name;
}
