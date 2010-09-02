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

#ifndef _RPG_COMMONEVENT_H_
#define _RPG_COMMONEVENT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rpg_eventcommand.h"

////////////////////////////////////////////////////////////
/// RPG::CommonEvent class
////////////////////////////////////////////////////////////
namespace RPG {
	class CommonEvent {
	public:
		enum Trigger {
			Trigger_automatic = 3,
			Trigger_parallel = 4,
			Trigger_call = 5
		};

		CommonEvent();
		
		int ID;
		std::string name;
		int trigger;
		bool switch_flag;
		int switch_id;
		std::vector<EventCommand> event_commands;
	};
}

#endif
