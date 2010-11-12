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

#ifndef _H_GAMECOMMONEVENT
#define _H_GAMECOMMONEVENT

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>

namespace RPG {
	class CommonEvent;
	class EventCommand;
}

class Interpreter;

class Game_CommonEvent
{
public:
	Game_CommonEvent(int _common_event_id);
	~Game_CommonEvent();

	std::string Name();
	int Trigger();
	int SwitchId();
	std::vector<RPG::EventCommand> List();

	void Refresh();
	void Update();

private:

	int common_event_id;

	Interpreter* interpreter; // Interpreter for parallel procceses
};

#endif
