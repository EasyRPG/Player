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

#ifndef _GAME_EVENT_H_
#define _GAME_EVENT_H_

// Headers
#include <vector>
#include "game_character.h"
#include "rpg_event.h"
#include "game_interpreter.h"
#include <boost/scoped_ptr.hpp>

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

	int GetX() const;
	void SetX(int new_x);
	int GetY() const;
	void SetY(int new_y);

	/**
	 * Clears starting flag.
	 */
	void ClearStarting();

	/**
	 * Does refresh.
	 */
	void Refresh();

	void Setup(RPG::EventPage* new_page);

	/**
	 * Gets event ID.
	 *
	 * @return event ID.
	 */
	int GetId() const;

	/**
	 * Gets starting flag.
	 *
	 * @return starting flag.
	 */
	bool GetStarting() const;

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
	std::vector<RPG::EventCommand>& GetList();

	void CheckEventTriggerAuto();
	bool CheckEventTriggerTouch(int x, int y);
	void Start();
	void Update();
	bool AreConditionsMet(const RPG::EventPage& page);

	/**
	 * Sets disabled flag for the event (enables or disables it).
	 *
	 * @param dis_flag enables or disables the event.
	 */
	void SetDisabled(bool dis_flag);

	/**
	 * Gets if the event has been disabled.
	 *
	 * @return if the event has been disabled (via EraseEvent-EventCommand).
	 */
	bool GetDisabled() const;

	RPG::Event& GetEvent();

private:
	// Not a reference on purpose.
	// Events change during map change and old are destroyed, breaking the
	// reference.
	RPG::SaveMapEvent data;

	int ID;
	bool starting;
	int trigger;
	int map_id;
	RPG::Event event;
	bool erased;
	RPG::EventPage* page;
	std::vector<RPG::EventCommand> list;
	boost::scoped_ptr<Game_Interpreter> interpreter;
};

#endif
