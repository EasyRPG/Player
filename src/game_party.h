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

#ifndef _GAME_PARTY_H_
#define _GAME_PARTY_H_

// Headers
#include <vector>
#include <map>
#include "game_actor.h"
#include "main_data.h"

/**
 * Game_Party class.
 */
namespace Game_Party {
	/**
	 * Initializes Game_Party.
	 */
	void Init();

	/**
	 * Setups initial party.
	 */
	void SetupStartingMembers();

	/**
	 * Setups battle test party.
	 */
	//void SetupBattleTestMembers();

	/**
	 * Refreshes party members.
	 */
	//void Refresh();

	/**
	 * Gets maximum level.
	 *
	 * @return max party level.
	 */
	//int MaxLevel();

	/**
	 * Adds an actor to the party.
	 *
	 * @param actor_id database actor ID.
	 */
	void AddActor(int actor_id);

	/**
	 * Removes an actor from the party.
	 *
	 * @param actor_id database actor ID.
	 */
	void RemoveActor(int actor_id);

	/**
	 * Gets if an actor is in party.
	 *
	 * @param actor actor object.
	 * @return whether the actor is in party.
	 */
	bool IsActorInParty(int actor_id);

	/**
	 * Gains gold.
	 *
	 * @param value gained gold.
	 */
	void GainGold(int value);

	/**
	 * Loses gold.
	 *
	 * @param value lost gold.
	 */
	void LoseGold(int value);

	/**
	 * Increases steps in 1.
	 */
	//void IncreaseSteps();

	/**
	 * Returns all items of the party.
	 *
	 * @param item_list vector to fill.
	 */
	void GetItems(std::vector<int>& item_list);

	/**
	 * Gets number of possessed or equipped items.
	 *
	 * @param item_id database item ID.
	 * @get_equipped If true this returns the number
	 *               of equipped items.
	 * @return number of items.
	 */
	int ItemNumber(int item_id, bool get_equipped = false);

	/**
	 * Gains an amount of items.
	 *
	 * @param item_id database item ID.
	 * @param amount gained quantity.
	 */
	void GainItem(int item_id, int amount);

	/**
	 * Loses an amount of items.
	 *
	 * @param item_id database item ID.
	 * @param amount lost quantity.
	 */
	void LoseItem(int item_id, int amount);

	/**
	 * Gets if item can be used.
	 *
	 * @param item_id database item ID.
	 * @return whether the item can be used.
	 */
	bool IsItemUsable(int item_id);

	/**
	 * Clears all actors in party actions.
	 */
	//void ClearActions();

	/**
	 * Determines if a command can be input.
	 *
	 * @return whether a command can be input.
	 */
	//bool IsInputable();

	/**
	 * Determines if everyone is dead.
	 *
	 * @return whether all are dead.
	 */
	//bool AreAllDdead();

	/**
	 * Checks slip damage.
	 */
	//void CheckMapSlipDamage();

	/**
	 * Random selection of target actor.
	 *
	 * @param hp0 : only targets actors with 0 HP.
	 * @return target actor.
	 */
	//Game_Actor* RandomTargetActor(bool hp0 = false);

	/**
	 * Smooth selection of target actor.
	 *
	 * @param actor_index : actor index in party.
	 * @return target actor.
	 */
	//Game_Actor* SmoothTargetActor(int actor_index);

	/**
	 * Gets gold possessed.
	 *
	 * @return gold possessed.
	 */
	int GetGold();

	/**
	 * Gets steps walked.
	 *
	 * @return steps walked.
	 */
	int GetSteps();

	/**
	 * Gets actors in party list.
	 *
	 * @return actors in party list.
	 */
	std::vector<Game_Actor*> GetActors();

	/**
	 * Gets number of battles.
	 *
	 * @return number of battles.
	 */
	int GetBattleCount();

	/**
	 * Gets number of battles wins.
	 *
	 * @return number of battles wins.
	 */
	int GetWinCount();

	/**
	 * Gets number of battles defeats.
	 *
	 * @return number of battles defeats.
	 */
	int GetDefeatCount();

	/**
	 * Gets number of battles escapes.
	 *
	 * @return number of battles escapes.
	 */
	int GetRunCount();

	enum sys_timer {
		Timer1,
		Timer2
	};

	void SetTimer(int which, int seconds);
	void StartTimer(int which, bool visible, bool battle);
	void StopTimer(int which);
	void UpdateTimers();
	int ReadTimer(int which);
}

#endif
