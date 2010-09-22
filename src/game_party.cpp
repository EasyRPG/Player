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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "game_party.h"
#include "game_actors.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Party::Game_Party()
{
	gold = 0;
	steps = 0;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Party::~Game_Party()
{
}

////////////////////////////////////////////////////////////
/// SetupStartingMembers
////////////////////////////////////////////////////////////
void Game_Party::SetupStartingMembers() {
	actors.clear();
	for (unsigned i = 0; i < Main_Data::data_system.party.size(); ++i) {
		actors.push_back((*Main_Data::game_actors)[Main_Data::data_system.party[i]]);
	}
}
