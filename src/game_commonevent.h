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

#ifndef EP_GAME_COMMONEVENT_H
#define EP_GAME_COMMONEVENT_H

// Headers
#include <string>
#include <vector>
#include "game_interpreter_map.h"
#include "rpg_commonevent.h"
#include "rpg_saveeventdata.h"

/**
 * Game_CommonEvent class.
 */
class Game_CommonEvent {
public:
	/**
	 * Constructor.
	 *
	 * @param common_event_id database common event ID.
	 */
	explicit Game_CommonEvent(int common_event_id);

	/**
	 * Set savegame data.
	 *
	 * @param data savegame data.
	 */
	void SetSaveData(const RPG::SaveEventData& data);

	/**
	 * Refreshes the common event.
	 */
	void Refresh();

	/**
	 * Updates common event.
	 */
	void Update();

	/**
	 * Updates common event parallel interpreter.
	 */
	void UpdateParallel();

	/**
	 * Gets common event index.
	 *
	 * @return common event index in list.
	 */
	int GetIndex() const;

	/**
	 * Gets common event name.
	 *
	 * @return event name.
	 */
	std::string GetName() const;

	/**
	 * Gets trigger condition.
	 *
	 * @return trigger condition.
	 */
	int GetTrigger() const;

	/**
	 * Gets if an event has a switch.
	 *
	 * @return flag if that event has a switch.
	 */
	bool GetSwitchFlag() const;

	/**
	 * Gets trigger switch ID.
	 *
	 * @return trigger switch ID.
	 */
	int GetSwitchId() const;

	/**
	 * Gets event commands list.
	 *
	 * @return event commands list.
	 */
	std::vector<RPG::EventCommand>& GetList();

	RPG::SaveEventData GetSaveData();

private:
	int common_event_id;
	/**
	 * If parallel interpreter is running (true) or suspended (false).
	 * When switched to running it continues where it was suspended.
	 */
	bool parallel_running = false;

	/** Interpreter for parallel common events. */
	std::unique_ptr<Game_Interpreter_Map> interpreter;
};

#endif
