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
#include "output.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Party::Game_Party() {
	gold = 0;
	steps = 0;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Party::~Game_Party() {

}

////////////////////////////////////////////////////////////
/// SetupStartingMembers
////////////////////////////////////////////////////////////
void Game_Party::SetupStartingMembers() {
	actors.clear();
	for (unsigned i = 0; i < Main_Data::data_system.party.size(); ++i) {
		Game_Actor* actor;
		actor = Main_Data::game_actors->GetActor(Main_Data::data_system.party[i]);

		if (actor == NULL) {
			Output::Warning(
				"Invalid actor (Id: %d) in initial party at index %d.",
				Main_Data::data_system.party[i], i);
		} else {
			actors.push_back(actor);
		}
	}
}

////////////////////////////////////////////////////////////
/// Item Number
////////////////////////////////////////////////////////////
int Game_Party::ItemNumber(int item_id) {
	std::map<int, int>::iterator it;
	it = items.find(item_id);

	if (it != items.end()) {
		return it->second;
	}
	
	return 0;
}


////////////////////////////////////////////////////////////
/// Gain Gold
////////////////////////////////////////////////////////////
void Game_Party::GainGold(int n) {
	int a = gold + n;
	gold = min(max(a, 0), 9999999);
}

void Game_Party::GainItem(int item_id, int n) {
	int a;
	if (item_id > 0) {
		a = ItemNumber(item_id);
		items[item_id] = min(max(a + n, 0), 99);
	}
}