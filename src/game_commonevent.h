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

#ifndef _GAME_COMMONEVENT_H_
#define _GAME_COMMONEVENT_H_

// Headers
#include <string>
#include <vector>
#include "game_interpreter.h"
#include "rpg_commonevent.h"
#include <boost/scoped_ptr.hpp>

/**
 * Game_CommonEvent class.
 */
class Game_CommonEvent {
public:
	/**
	 * Constructor.
	 *
	 * @param common_event_id database common event ID.
	 * @param battle FIXME.
	 */
	Game_CommonEvent(int common_event_id, bool battle = false);

	/**
	 * Refreshes the common event.
	 */
	void Refresh();

	/**
	 * Updates common event interpreter.
	 */
	void Update();

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

private:
	int common_event_id;
	bool battle;

	/** Interpreter for parallel common events. */
	boost::scoped_ptr<Game_Interpreter> interpreter;
};

#endif
