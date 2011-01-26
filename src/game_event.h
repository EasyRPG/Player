/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GAME_EVENT_H_
#define _GAME_EVENT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "game_character.h"
#include "rpg_event.h"
#include "game_interpreter.h"

////////////////////////////////////////////////////////////
/// Game_Event class
////////////////////////////////////////////////////////////
class Game_Event : public Game_Character {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	////////////////////////////////////////////////////////
	Game_Event(int map_id, const RPG::Event& event);

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~Game_Event();

	////////////////////////////////////////////////////////
	/// Clear Starting Flag.
	////////////////////////////////////////////////////////
	void ClearStarting();

	////////////////////////////////////////////////////////
	/// Refresh.
	////////////////////////////////////////////////////////
	void Refresh();

	void Setup(RPG::EventPage* new_page);

	/// @return event id
	int GetId() const;

	/// @return starting flag
	bool GetStarting() const;

	/// @return trigger condition
	int GetTrigger() const;

	/// @return event commands list
	std::vector<RPG::EventCommand>& GetList();

	void CheckEventTriggerAuto();
	bool CheckEventTriggerTouch(int x, int y);
	void Start();
	void Update();
	bool AreConditionsMet(const RPG::EventPage& page);

	/// @param dis_flag : Enables or Disables the Event
	void SetDisabled(bool dis_flag);

	/// @return if the event has been disabled (via EraseEvent-EventCommand)
	bool GetDisabled() const;

	RPG::Event& GetEvent();

private:
	int ID;
	bool starting;
	int trigger;
	int map_id;
	RPG::Event event;
	bool erased;
	bool through;
	RPG::EventPage* page;
	std::vector<RPG::EventCommand> list;
	Game_Interpreter* interpreter;
};

#endif
