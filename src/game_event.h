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
#include "rpg_event.h"
#include "rpg_savemapevent.h"
#include "game_interpreter_map.h"
#include "async_op.h"

/**
 * Game_Event class.
 */
class Game_Event : public Game_Character {
public:
	/**
	 * Constructor.
	 */
	Game_Event(int map_id, const RPG::Event& event);

	/**
	 * Constructor.
	 * Create event from save data.
	 */
	Game_Event(int map_id, const RPG::Event& event, const RPG::SaveMapEvent& data);

	/**
	 * Implementation of abstract methods
	 */
	/** @{ */
	bool IsMoveRouteActive() const override;
	void OnMoveFailed(int x, int y) override;
	/** @} */

	/**
	 * Does refresh.
	 */
	void Refresh(bool from_save = false);

	void Setup(const RPG::EventPage* new_page);
	void SetupFromSave(const RPG::EventPage* new_page);

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
	std::string GetName() const;

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
	RPG::EventPage::Trigger GetTrigger() const;

	/**
	 * Gets event commands list.
	 *
	 * @return event commands list.
	 */
	const std::vector<RPG::EventCommand>& GetList() const;

	/**
	 * Event returns to its original direction before talking to the hero.
	 */
	void OnFinishForegroundEvent();

	/** Mark the event as waiting for execution */
	bool SetAsWaitingForegroundExecution(bool face_hero, bool triggered_by_decision_key);

	/** 
	 * Update this for the current frame
	 *
	 * @param resume_async If we're resuming from an async operation.
	 * @return async operation if we should suspend, otherwise returns AsyncOp::eNone
	 */
	AsyncOp Update(bool resume_async);

	bool AreConditionsMet(const RPG::EventPage& page);

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
	const RPG::EventPage* GetPage(int page) const;

	/**
	 * Returns the active event page or nullptr if no page is active.
	 *
	 * @return active page or nullptr
	 */
	const RPG::EventPage* GetActivePage() const;

	const RPG::SaveMapEvent& GetSaveData();

protected:
	RPG::SaveMapEvent* data();
	const RPG::SaveMapEvent* data() const;

private:
	void UpdateSelfMovement() override;
	void CheckEventAutostart();
	void CheckEventCollision();

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

	// Not a reference on purpose.
	// Events change during map change and old are destroyed, breaking the
	// reference.
	std::unique_ptr<RPG::SaveMapEvent> _data_copy;

	RPG::Event event;
	const RPG::EventPage* page = nullptr;
	std::unique_ptr<Game_Interpreter_Map> interpreter;
};

inline RPG::SaveMapEvent* Game_Event::data() {
	return static_cast<RPG::SaveMapEvent*>(Game_Character::data());
}

inline const RPG::SaveMapEvent* Game_Event::data() const {
	return static_cast<const RPG::SaveMapEvent*>(Game_Character::data());
}

#endif
