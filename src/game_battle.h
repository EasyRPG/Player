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
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/system.h>
#include <lcf/rpg/troop.h>
#include "teleport_target.h"
#include "utils.h"
#include "point.h"

class Game_Battler;
class Game_Enemy;
class Game_Actor;
class Game_Interpreter;
class Game_Interpreter_Battle;
class Spriteset_Battle;

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
	int ShowBattleAnimation(int animation_id, std::vector<Game_Battler*> targets, bool only_sound = false, int cutoff = -1, bool invert = false);

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

	int GetTurn();
	bool CheckTurns(int turns, int base, int multiple);

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter& GetInterpreter();

	/**
	 * Gets the battle game interpreter.
	 *
	 * @return the battle game interpreter.
	 */
	Game_Interpreter_Battle& GetInterpreterBattle();

	void SetTerrainId(int id);
	int GetTerrainId();

	void SetBattleCondition(lcf::rpg::System::BattleCondition cond);
	lcf::rpg::System::BattleCondition GetBattleCondition();

	void SetBattleFormation(lcf::rpg::System::BattleFormation form);
	lcf::rpg::System::BattleFormation GetBattleFormation();

	/**
	 * Calculates the base grid position from the parameters used for both actors and enemies.
	 *
	 * @param party_idx the index of the party member
	 * @param party_size the size of the party
	 * @param table_x Which grid table to use for X position
	 * @param table_y which grid table to use for Y position
	 * @param cond the battle condition
	 * @param form the battle formation
	 * @param terrain_id the battle terrain id
	 */
	Point CalculateBaseGridPosition(
			int party_idx,
			int party_size,
			int x_table,
			int y_table,
			lcf::rpg::System::BattleFormation form,
			int terrain_id);

	/**
	 * Calculate the battler position that would be used for the enemy
	 *
	 * @param enemy the enemy to calculate.
	 */
	Point Calculate2k3BattlePosition(const Game_Enemy& enemy);

	/**
	 * Calculate the battler position that would be used for the enemy
	 *
	 * @param enemy the enemy to calculate.
	 */
	Point Calculate2k3BattlePosition(const Game_Actor& actor);

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

#endif
