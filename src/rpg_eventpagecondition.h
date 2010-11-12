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

#ifndef _RPG_EVENTPAGECONDITION_H_
#define _RPG_EVENTPAGECONDITION_H_

////////////////////////////////////////////////////////////
/// RPG::EventPageCondition class
////////////////////////////////////////////////////////////
namespace RPG {
	class EventPageCondition {
	public:
		EventPageCondition();
		
		bool switch_a;
		bool switch_b;
		bool variable;
		bool item;
		bool actor;
		bool timer;
		int switch_a_id;
		int switch_b_id;
		int variable_id;
		int variable_value;
		int item_id;
		int actor_id;
		int timer_sec;
	};
}

#endif
