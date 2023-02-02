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

#ifndef EP_GAME_PARTY_H
#define EP_GAME_PARTY_H

// Headers
#include <cstdint>
#include <vector>
#include "game_party_base.h"
#include "game_actor.h"
#include <lcf/rpg/saveinventory.h>

/**
 * Game_Party class.
 */
class Game_Party : public Game_Party_Base {
public:
	/**
	 * Initializes Game_Party.
	 */
	Game_Party();

	/** Initialize for new game */
	void SetupNewGame();

	/** Setups battle test party */
	void SetupBattleTest();

	/** Initialize from save game */
	void SetupFromSave(lcf::rpg::SaveInventory save);

	/** @return save game data */
	const lcf::rpg::SaveInventory& GetSaveData() const;

	Game_Actor& operator[] (const int index) override;

	int GetBattlerCount() const override;
	int GetVisibleBattlerCount() const override;


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
	 * Removes all actors from the party.
	 */
	void Clear();

	/**
	 * Gets if an actor is in party.
	 *
	 * @param actor_id database actor ID.
	 * @return whether the actor is in party.
	 */
	bool IsActorInParty(int actor_id);

	/**
	 * Gets position of actor in party.
	 *
	 * @param actor_id database actor ID
	 * @return Party position 0 - 3 or -1 if not in
	 */
	int GetActorPositionInParty(int actor_id);

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
	 * Returns all items of the party sorted by item id.
	 *
	 * @param item_list vector to fill.
	 */
	void GetItems(std::vector<int>& item_list);

	/**
	 * Gets number of item in inventory.
	 *
	 * @param item_id database item ID.
	 * @return number of items.
	 */
	int GetItemCount(int item_id) const;

	/**
	 * Gets number of item equipped by the party.
	 *
	 * @param item_id database item ID.
	 * @return number of items.
	 */
	int GetEquippedItemCount(int item_id) const;

	/**
	 * Gets number of item in inventory and equipped by party.
	 *
	 * @param item_id database item ID.
	 * @return number of items.
	 */
	int GetItemTotalCount(int item_id) const;

	/**
	 * Gets maximum number of item allowed in inventory.
	 *
	 * @param item_id database item ID.
	 * @return maximum number of items.
	 */
	int GetMaxItemCount(int item_id) const;

	/**
	 * Gains an amount of items.
	 *
	 * @param item_id database item ID.
	 * @param amount gained quantity.
	 */
	void AddItem(int item_id, int amount);

	/**
	 * Loses an amount of items.
	 *
	 * @param item_id database item ID.
	 * @param amount lost quantity.
	 */
	void RemoveItem(int item_id, int amount);

	/**
	 * Consumes one use of an item (eg. for multi-use items).
	 * Doesn't actually do anything with the item, just uses up one use.
	 *
	 * @param item_id database item ID
	 */
	void ConsumeItemUse(int item_id);

	/**
	 * Gets if item can be used.
	 * When a target is specified this also checks if the target can use the item.
	 *
	 * @param item_id database item ID.
	 * @param target Target to test
	 * @return whether the item can be used.
	 */
	bool IsItemUsable(int item_id, const Game_Actor* target = nullptr) const;

	/**
	 * Uses an item on an actor.
	 * Tests if using that item makes any sense (e.g. for HP healing
	 * items if there are any HP to heal)
	 *
	 * @param item_id ID of item to use
	 * @param target Target the item is used on (or NULL if its for the party)
	 */
	bool UseItem(int item_id, Game_Actor* target = nullptr);

	/**
	 * Determines if a skill can be used.
	 * When a target is specified this also checks if the target can use the item.
	 *
	 * @param skill_id Skill to test
	 * @param target Skill user to test
	 * @param from_item Skill is invoked by an item. This alters the usable check outside of battles.
	 */
	bool IsSkillUsable(int skill_id, const Game_Actor* target = nullptr, bool from_item = false) const;

	/**
	 * Uses a skill on an actor.
	 * Tests if using that skill makes any sense (e.g. for HP healing
	 * skills if there are any HP to heal)
	 *
	 * @param skill_id ID of skill to use
	 * @param source Actor using the skill
	 * @param target Target the skill is used on (or NULL if its for the party)
	 */
	bool UseSkill(int skill_id, Game_Actor* source, Game_Actor* target = NULL);

	/**
	 * Gets gold possessed.
	 *
	 * @return gold possessed.
	 */
	int GetGold() const;

	/**
	 * Gets steps walked.
	 *
	 * @return steps walked.
	 */
	int GetSteps() const;

	/**
	 * Increment the number of steps walked by 1.
	 */
	void IncSteps();

	/**
	 * Gets actors in party list.
	 *
	 * @return actors in party list.
	 */
	std::vector<Game_Actor*> GetActors() const;

	/**
	 * Get's the i'th actor in the party.
	 *
	 * @param idx the party index starting from 0.
	 *
	 * @return actor at index, or nullptr if no such actor
	 */
	Game_Actor* GetActor(int idx) const;

	/**
	 * Gets number of battles.
	 *
	 * @return number of battles.
	 */
	int GetBattleCount() const;

	/**
	 * Increment the number of battles by 1.
	 */
	void IncBattleCount();

	/**
	 * Gets number of battles wins.
	 *
	 * @return number of battles wins.
	 */
	int GetWinCount() const;

	/**
	 * Increment the number of battles wins by 1.
	 */
	void IncWinCount();

	/**
	 * Gets number of battles defeats.
	 *
	 * @return number of battles defeats.
	 */
	int GetDefeatCount() const;

	/**
	 * Increment the number of battles defeats by 1.
	 */
	void IncDefeatCount();

	/**
	 * Gets number of battles escapes.
	 *
	 * @return number of battles escapes.
	 */
	int GetRunCount() const;

	/**
	 * Increment the number of battles escapes by 1.
	 */
	void IncRunCount();

	/**
	 * Damages all actors in party by the same value.
	 * Used by damage terrain on the map.
	 *
	 * @param damage How many damage to apply
	 * @param lethal If the damage can be lethal (kill a character) or not
	 */
	void ApplyDamage(int damage, bool lethal);

	/**
	 * Gets average level of the party (for battle)
	 *
	 * @return average level
	 */
	int GetAverageLevel();

	/**
	 * Gets party exhaustion level (for battle)
	 *
	 * @return exhaustion level
	 */
	int GetFatigue();

	enum sys_timer {
		Timer1,
		Timer2
	};

	/**
	 * Sets a timer.
	 *
	 * @param which which timer to set.
	 * @param seconds the time in seconds.
	 */
	void SetTimer(int which, int seconds);

	/**
	 * Starts a timer.
	 *
	 * @param which which timer to start.
	 * @param visible whether the timer is visible.
	 * @param battle whether the timer runs during battle.
	 */
	void StartTimer(int which, bool visible, bool battle);

	/**
	 * Stops a timer.
	 *
	 * @param which which timer to stop.
	 */
	void StopTimer(int which);

	/**
	 * Updates all timers.
	 */
	void UpdateTimers();

	/**
	 * Get a timer's value in seconds.
	 *
	 * @param which which timer to read.
	 * @return number of seconds remaining.
	 */
	int GetTimerSeconds(int which);

	/**
	 * Get a timer's value in frames.
	 *
	 * @param which which timer to read.
	 * @return number of frames remaining.
	 */
	int GetTimerFrames(int which);

	/**
	 * Returns whether a timer should be visible now
	 *
	 * @param which which timer to read
	 * @param in_battle whether we're currently in a battle.
	 *
	 * @return true if visible.
	 */
	bool GetTimerVisible(int which, bool in_battle);

	/**
	 * Removes invalid actors and items from the party
	 */
	void RemoveInvalidData();

	std::vector<int16_t> GetInflictedStates() const;

	/**
	 * Applies damage to the game party based on their stats.
	 *
	 * @return Whether the actor suffered some damage.
	 */
	bool ApplyStateDamage();

	/**
	 * @return Whether any party member accepts custom battle commands.
	 */
	bool IsAnyControllable();

	/**
	 * Gets the actor with the highest level who can act and use the given item. If there are many, choose the one with the earliest position in the group.
	 *
	 * @param the item to check
	 *
	 * @return The first Highest leveled actor who can act.
	 */
	Game_Actor* GetHighestLeveledActorWhoCanUse(const lcf::rpg::Item*) const;

	/**
	 * If a battle is running, returns the current battle turn for the party.
	 * Otherwise, returns the number of turns of the last battle
	 *
	 * @return number of turns
	 */
	int GetTurns() const;

	/** Increment turn counter */
	void IncTurns();

	/** Reset turn counter to 0 */
	void ResetTurns();

private:
	std::pair<int,bool> GetItemIndex(int item_id) const;

	lcf::rpg::SaveInventory data;
};

// ------ INLINES --------

inline const lcf::rpg::SaveInventory& Game_Party::GetSaveData() const {
	return data;
}

inline int Game_Party::GetBattleCount() const {
	return data.battles;
}

inline void Game_Party::IncBattleCount() {
	++data.battles;
}

inline int Game_Party::GetWinCount() const {
	return data.victories;
}

inline void Game_Party::IncWinCount() {
	++data.victories;
}

inline int Game_Party::GetDefeatCount() const {
	return data.defeats;
}

inline void Game_Party::IncDefeatCount() {
	++data.defeats;
}

inline int Game_Party::GetRunCount() const {
	return data.escapes;
}

inline void Game_Party::IncRunCount() {
	++data.escapes;
}

inline int Game_Party::GetGold() const {
	return data.gold;
}

inline int Game_Party::GetSteps() const {
	return data.steps;
}

inline void Game_Party::IncSteps() {
	++data.steps;
}

inline int Game_Party::GetTurns() const {
	return data.turns;
}

inline void Game_Party::IncTurns() {
	++data.turns;
}

inline void Game_Party::ResetTurns() {
	data.turns = 0;
}


#endif
