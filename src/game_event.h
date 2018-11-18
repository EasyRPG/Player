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
#include "game_interpreter.h"

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
	int GetMoveFrequency() const override;
	void SetMoveFrequency(int frequency) override;
	int GetOriginalMoveRouteIndex() const override;
	void SetOriginalMoveRouteIndex(int new_index) override;
	bool GetThrough() const override;
	void SetThrough(bool through) override;
	/** @} */

	/**
	 * Clears starting flag.
	 */
	void ClearStarting();

	/**
	 * Does refresh.
	 */
	void Refresh();

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

	/**
	 * Gets starting flag.
	 *
	 * @return starting flag.
	 */
	bool GetStarting() const;

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
	int GetTrigger() const;

	/**
	 * Gets event commands list.
	 *
	 * @return event commands list.
	 */
	const std::vector<RPG::EventCommand>& GetList() const;

	/**
	 * Event's sprite looks towards the hero but its original direction is remembered.
	 */
	void StartTalkToHero();

	/**
	 * Event returns to its original direction before talking to the hero.
	 */
	void StopTalkToHero();

	void CheckEventTriggers();
	bool CheckEventTriggerTouch(int x, int y) override;
	void Start(bool triggered_by_decision_key = false);
	void Update() override;
	void UpdateParallel();
	bool AreConditionsMet(const RPG::EventPage& page);

	/**
	 * Activates or deactivates the event.
	 *
	 * @param active enables or disables the event.
	 */
	void SetActive(bool active);

	/**
	 * Gets if the event is active.
	 *
	 * @return if the event is active (or inactive via EraseEvent-EventCommand).
	 */
	bool GetActive() const;

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
	 * Walks to the player.
	 */
	void MoveTypeTowardsPlayer();

	/**
	 * Walks to the player.
	 */
	void MoveTypeAwayFromPlayer();

	// Not a reference on purpose.
	// Events change during map change and old are destroyed, breaking the
	// reference.
	std::unique_ptr<RPG::SaveMapEvent> _data_copy;

	bool starting = false, running = false, halting = false;
	bool started_by_decision_key = false;
	int trigger = -1;
	RPG::Event event;
	const RPG::EventPage* page = nullptr;
	std::vector<RPG::EventCommand> list;
	std::shared_ptr<Game_Interpreter> interpreter;
	bool from_save;
	bool updating = false;

	int frame_count_at_last_auto_start_check = -1;
};

inline RPG::SaveMapEvent* Game_Event::data() {
	return static_cast<RPG::SaveMapEvent*>(Game_Character::data());
}

inline const RPG::SaveMapEvent* Game_Event::data() const {
	return static_cast<const RPG::SaveMapEvent*>(Game_Character::data());
}

#endif
