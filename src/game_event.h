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

#ifndef EP_GAME_EVENT_H
#define EP_GAME_EVENT_H

// Headers
#include <string>
#include <vector>
#include "game_character.h"
#include <lcf/rpg/event.h>
#include <lcf/rpg/savemapevent.h>
#include "game_interpreter_map.h"
#include "async_op.h"

using Game_EventBase = Game_CharacterDataStorage<lcf::rpg::SaveMapEvent>;

/**
 * Game_Event class.
 */
class Game_Event : public Game_EventBase {
public:
	/**
	 * Constructor.
	 */
	Game_Event(int map_id, const lcf::rpg::Event* event);

	/** Load from saved game */
	void SetSaveData(lcf::rpg::SaveMapEvent save);

	/** @return save game data */
	lcf::rpg::SaveMapEvent GetSaveData() const;

	/**
	 * Implementation of abstract methods
	 */
	/** @{ */
	Drawable::Z_t GetScreenZ(bool apply_shift = false) const override;
	bool Move(int dir) override;
	void UpdateNextMovementAction() override;
	bool IsVisible() const override;
	/** @} */

	/**
	 * Re-checks active pages and sets up new page on change.
	 */
	void RefreshPage();

	/**
	 * Gets event ID.
	 *
	 * @return event ID.
	 */
	int GetId() const;

	/**
	 * Gets event name.
	 *
	 * @return event name.
	 */
	StringView GetName() const;

	/** Clears waiting_execution flag */
	void ClearWaitingForegroundExecution();

	/** @return waiting_execution flag.  */
	bool IsWaitingForegroundExecution() const;

	/**
	 * If the event is starting, whether or not it was started
	 * by pushing the decision key.
	 *
	 * @return started by decision key
	 */
	bool WasStartedByDecisionKey() const;

	/**
	 * Gets trigger condition.
	 *
	 * @return trigger condition.
	 */
	lcf::rpg::EventPage::Trigger GetTrigger() const;

	/**
	 * Gets event commands list.
	 *
	 * @return event commands list.
	 */
	const std::vector<lcf::rpg::EventCommand>& GetList() const;

	/**
	 * Event returns to its original direction before talking to the hero.
	 */
	void OnFinishForegroundEvent();

	/**
	 * Schedule the event for execution on the map's foreground interpreter.
	 *
	 * @param triggered_by_decision_key set whether this was triggered by decision key
	 * @param face_hero if scheduled, event faces the player.
	 *
	 * @return true if event was scheduled.
	 */
	bool ScheduleForegroundExecution(bool triggered_by_decision_key, bool face_player);

	/**
	 * Update this for the current frame
	 *
	 * @param resume_async If we're resuming from an async operation.
	 * @return async operation if we should suspend, otherwise returns AsyncOp::eNone
	 */
	AsyncOp Update(bool resume_async);

	bool AreConditionsMet(const lcf::rpg::EventPage& page);

	/**
	 * Returns current index of a "Movement Type Custom" move route.
	 *
	 * @return current original move route index
	 */
	int GetOriginalMoveRouteIndex() const;

	/**
	 * Sets current index of a "Movement Type Custom" move route.
	 *
	 * @param new_index New move route index
	 */
	void SetOriginalMoveRouteIndex(int new_index);

	/**
	 * Returns the event page or nullptr is page does not exist.
	 *
	 * @param page Page number (starting from 1)
	 *
	 * @return page or nullptr
	 */
	const lcf::rpg::EventPage* GetPage(int page) const;

	/**
	 * Returns the active event page or nullptr if no page is active.
	 *
	 * @return active page or nullptr
	 */
	const lcf::rpg::EventPage* GetActivePage() const;

	/** @returns the number of pages this event has */
	int GetNumPages() const;

protected:
	/** Check for and fix incorrect data after loading save game */
	void SanitizeData();
private:
	bool CheckEventAutostart();
	bool CheckEventCollision();
	void SetMaxStopCountForRandom();

	/**
	 * Moves on a random route.
	 */
	void MoveTypeRandom();

	/**
	 * Cycles between moving in default_dir and its opposite.
	 * Tries to move in the event's movement direction, or
	 * default_dir if this is along the wrong axis. Reverses
	 * directions when encountering an obstacle.
	 */
	void MoveTypeCycle(int default_dir);

	/**
	 * Cycles left and right.
	 */
	void MoveTypeCycleLeftRight();

	/**
	 * Cycles up and down.
	 */
	void MoveTypeCycleUpDown();

	/**
	 * Implementation method for walking to
	 * or from the player
	 */
	void MoveTypeTowardsOrAwayPlayer(bool towards);

	/**
	 * Walks to the player.
	 */
	void MoveTypeTowardsPlayer();

	/**
	 * Walks away from the player.
	 */
	void MoveTypeAwayFromPlayer();

	void CheckCollisonOnMoveFailure();

	const lcf::rpg::Event* event = nullptr;
	const lcf::rpg::EventPage* page = nullptr;
	std::unique_ptr<Game_Interpreter_Map> interpreter;
};

inline int Game_Event::GetNumPages() const {
	return event->pages.size();
}

inline bool Game_Event::IsVisible() const {
	return GetActivePage() != nullptr && Game_Character::IsVisible();
}

#endif
