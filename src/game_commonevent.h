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
#include <lcf/rpg/commonevent.h>
#include <lcf/rpg/saveeventexecstate.h>
#include "async_op.h"
#include "string_view.h"

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
	void SetSaveData(const lcf::rpg::SaveEventExecState& data);

	/**
	 * Updates common event parallel interpreter.
	 *
	 * @param resume_async If we're resuming from an async operation.
	 * @return async operation if we should suspend, otherwise returns AsyncOp::eNone
	 */
	AsyncOp Update(bool resume_async);

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
	StringView GetName() const;

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
	std::vector<lcf::rpg::EventCommand>& GetList();

	lcf::rpg::SaveEventExecState GetSaveData();

	/** @return true if waiting for foreground execution */
	bool IsWaitingForegroundExecution() const;

	/**
	 * @param force_run force the event to execute even if conditions not met.
	 * @return true if waiting for background execution
	 */
	bool IsWaitingBackgroundExecution(bool force_run) const;

private:
	int common_event_id;

	/** Interpreter for parallel common events. */
	std::unique_ptr<Game_Interpreter_Map> interpreter;
};

#endif
