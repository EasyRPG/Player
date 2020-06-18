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

#ifndef EP_GAME_BATTLE_H
#define EP_GAME_BATTLE_H

#include <functional>
#include <lcf/rpg/system.h>
#include <lcf/rpg/troop.h>
#include "teleport_target.h"
#include "utils.h"

class Game_Battler;
class Game_Interpreter;
class Spriteset_Battle;

namespace lcf {
namespace rpg {
	class EventPage;
} // namespace rpg
} // namespace lcf

enum class BattleResult {
	Victory,
	Escape,
	Defeat,
	Abort
};


namespace Game_Battle {
	/**
	 * Initialize Game_Battle.
	 */
	void Init(int troop_id);

	/** @return true if a battle is currently running */
	bool IsBattleRunning();

	/**
	 * Quits (frees) Game_Battle.
	 */
	void Quit();

	/**
	 * Updates the battle animation
	 */
	void UpdateAnimation();

	/**
	 * Updates spriteset graphics
	 */
	void UpdateGraphics();

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
	 * Plays a battle animation against several targets simultaneously.
	 *
	 * @param animation_id the animation ID
	 * @param targets a vector of pointer to the battlers to play against
	 *
	 * @return the number of frames of the animation.
	 */
	int ShowBattleAnimation(int animation_id, std::vector<Game_Battler*> targets, bool only_sound = false, int cutoff = -1);

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
	void UpdateAtbGauges();

	void ChangeBackground(const std::string& name);

	const std::string& GetBackground();

	int GetEscapeFailureCount();

	void IncEscapeFailureCount();

	int GetTurn();
	bool CheckTurns(int turns, int base, int multiple);

	bool AreConditionsMet(const lcf::rpg::TroopPageCondition& condition);

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
	void RefreshEvents(std::function<bool(const lcf::rpg::TroopPage&)> predicate);

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter& GetInterpreter();

	void SetTerrainId(int id);
	int GetTerrainId();

	void SetBattleCondition(lcf::rpg::System::BattleCondition cond);
	lcf::rpg::System::BattleCondition GetBattleCondition();

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
	 * @param refresh need refresh state.
	 */
	void SetNeedRefresh(bool refresh);

	/**
	 * Uses RPG_RT algorithm for performing a variance adjument to damage/healing effects and returns the result.
	 *
	 * @param base - the base amount of the effect
	 * @param var - the variance level from 0 to 10
	 *
	 * @return the adjusted damage amount
	 */
	int VarianceAdjustEffect(int base, int var);

	struct BattleTest {
		bool enabled = false;
		int troop_id = 0;
		std::string background;
		int terrain_id = 0;
		lcf::rpg::System::BattleFormation formation = lcf::rpg::System::BattleFormation_terrain;
		lcf::rpg::System::BattleCondition condition = lcf::rpg::System::BattleCondition_none;
	};

	extern struct BattleTest battle_test;

	/** @return true if a death handler is installed */
	bool HasDeathHandler();

	/** @return death handler common event if one is installed, otherwise 0 */
	int GetDeathHandlerCommonEvent();

	/** @return death teleport handler if one is installed, otherwise an inactive target */
	TeleportTarget GetDeathHandlerTeleport();

	/** @return the active troop for the currently running battle */
	const lcf::rpg::Troop* GetActiveTroop();

	/** Don't reference this, use IsBattleRunning()! */
	extern bool battle_running;
}

inline bool Game_Battle::IsBattleRunning() {
	return battle_running;
}

inline int Game_Battle::VarianceAdjustEffect(int base, int var) {
	if (var > 0) {
		int adj = std::max(1, var * base / 10);
		return base + Utils::GetRandomNumber(0, adj) - adj / 2;
	}
	return base;
}

#endif
