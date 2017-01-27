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
#include "game_party_base.h"
#include "game_actor.h"
#include "main_data.h"

/**
 * Game_Party class.
 */
class Game_Party : public Game_Party_Base {
public:
	/**
	 * Initializes Game_Party.
	 */
	Game_Party();

	Game_Actor& operator[] (const int index) override;

	int GetBattlerCount() const override;

	/**
	 * Setups battle test party.
	 */
	void SetupBattleTestMembers();

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
	 * @param get_equipped if true equipped items are counted instead
	 * @return number of items.
	 */
	int GetItemCount(int item_id, bool get_equipped = false);

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
	std::vector<Game_Actor*> GetActors() const;

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

	/**
	 * Damages all actors in party by the same value.
	 * Used by damage terrain on the map.
	 *
	 * @param damage How many damage to apply
	 */
	void ApplyDamage(int damage);

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
	 * Get a timer's value.
	 *
	 * @param which which timer to read.
	 * @return number of frames remaining.
	 */
	int GetTimer(int which);

	/**
	 * Get a timer's value.
	 *
	 * @param which which timer to read.
	 * @param[out] visible whether timer is visible
	 * @param[out] battle whether timer runs in battle
	 * @return number of frames remaining.
	 */
	int GetTimer(int which, bool& visible, bool& battle);
};

#endif
