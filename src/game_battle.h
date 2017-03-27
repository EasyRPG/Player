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

#include <functional>
#include "rpg_troop.h"

class Game_Battler;
class Game_Interpreter;
class Spriteset_Battle;
namespace RPG {
	class EventPage;
}

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

	/**
	 * Checks if a victory condition for the player party (enemy dead) is fulfilled.
	 *
	 * @return True on victory
	 */
	bool CheckWin();

	/**
	 * Check if a lose condition for the player party (party dead) is fulfilled.
	 *
	 * @return True on lose
	 */
	bool CheckLose();

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

	/**
	 * Starts a new battle turn.
	 * Event pages are reset depending on the actor.
	 *
	 * @param battler Battler who did the turn.
	 */
	void NextTurn(Game_Battler* battler = nullptr);

	/**
	 * Updates the gauge of all battlers based on the highest agi of all.
	 */
	void UpdateGauges();

	void ChangeBackground(const std::string& name);

	int GetTurn();
	bool CheckTurns(int turns, int base, int multiple);

	bool AreConditionsMet(const RPG::TroopPageCondition& condition);

	/**
	 * Runs the current interpreter or starts a new one when pages are pending
	 *
	 * @return true when no interpreter needs to run anymore
	 */
	bool UpdateEvents();

	/**
	 * Checks through all pages and marks them as pending when they want to run.
	 */
	void RefreshEvents();

	/**
	 * Checks through pages the match the predicate and marks them as pending when they want to run.
	 *
	 * @param predicate Predicate to fulfill
	 */
	void RefreshEvents(std::function<bool(const RPG::TroopPage&)> predicate);

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

	/**
	 * Sets the party index of the latest targeted enemy. Only used by battle branch "is target"
	 *
	 * @param target_enemy id of targeted enemy
	 */
	void SetEnemyTargetIndex(int target_enemy);

	/**
	 * Gets the party index of the latest targeted enemy. Only used by battle branch "is target"
	 *
	 * @return targeted enemy
	 */
	int GetEnemyTargetIndex();

	/**
	 * Sets the need refresh flag.
	 * This flag is set when the interpreter altered the state of enemies (e.g. dead)
	 *
	 * @param need_refresh need refresh state.
	 */
	void SetNeedRefresh(bool refresh);

	extern int escape_fail_count;
	extern std::string background_name;
}
