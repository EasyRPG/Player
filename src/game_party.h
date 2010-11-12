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

#ifndef _GAME_PARTY_H_
#define _GAME_PARTY_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include <map>
#include "game_actor.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Game_Party class
////////////////////////////////////////////////////////////
class Game_Party {

public:
	Game_Party();
	~Game_Party();

	void SetupStartingMembers();

	int ItemNumber(int item_id);

	void GainGold(int value);
	void GainItem(int item_id, int n);

	int battle_count;
	int win_count;
	int defeat_count;
	int run_count;

	int gold;
	int steps;

	// Active actors
	// Pointers here are already handled by Game_Actors class!
	// Do not delete contents from this class
	std::vector<Game_Actor*> actors;

	// Map item ID with the quantity
	std::map<int, int> items;
};
#endif // _GAME_PARTY_H_
