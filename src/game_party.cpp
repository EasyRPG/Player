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
#include <algorithm>
#include "game_party.h"
#include "game_actors.h"
#include "game_player.h"
#include "output.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
namespace {
	int gold;
	int steps;
	std::vector<Game_Actor*> actors;
	std::map<int, int> items;

	int battle_count;
	int win_count;
	int defeat_count;
	int run_count;
}

////////////////////////////////////////////////////////////
void Game_Party::Init() {
	gold = 0;
	steps = 0;
	actors.clear();
	items.clear();

	battle_count = 0;
	win_count = 0;
	defeat_count = 0;
	run_count = 0;
}

////////////////////////////////////////////////////////////
void Game_Party::SetupStartingMembers() {
	actors.clear();
	for (size_t i = 0; i < Data::system.party.size(); ++i) {
		Game_Actor* actor = Game_Actors::GetActor(Data::system.party[i]);

		if (actor == NULL) {
			Output::Error("Invalid actor (Id: %d) in initial party at index %d.", Data::system.party[i], i);
		} else {
			actors.push_back(actor);
		}
	}
}

////////////////////////////////////////////////////////////
void Game_Party::GetItems(std::vector<int>& item_list) {
	item_list.clear();

	std::map<int, int>::iterator it;
	for (it = items.begin(); it != items.end(); ++it) {
		item_list.push_back(it->first);
	}
}

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
void Game_Party::GainGold(int n) {
	int a = gold + n;
	gold = min(max(a, 0), 999999);
}

////////////////////////////////////////////////////////////
void Game_Party::GainItem(int item_id, int amount, bool include_equip) {
	int a;
	if (item_id > 0 && item_id < Data::items.size()) {
		a = ItemNumber(item_id);
		items[item_id] = min(max(a + amount, 0), 99);
	} else {
		Output::Warning("Can't add item to party.\n%d is not a valid item id.",
			item_id);
	}
}

////////////////////////////////////////////////////////////
void Game_Party::LoseItem(int item_id, int amount, bool include_equip) {
	GainItem(item_id, -amount, include_equip);
}

////////////////////////////////////////////////////////////
bool Game_Party::IsItemUsable(int item_id) {
	if (item_id > 0 && item_id < Data::items.size()) {
		//ToDo: if (Game_Temp::IsInBattle()) {
		//if (Data::items[item_id - 1].type == RPG::Item::Type_medicine) {
		//	return !Data::items[item_id - 1].ocassion_field;
		//} else if (Data::items[item_id - 1].type == RPG::Item::Type_switch) {
		//	return Data::items[item_id - 1].ocassion_battle;
		//} else {
		if (Data::items[item_id - 1].type == RPG::Item::Type_medicine ||
			Data::items[item_id - 1].type == RPG::Item::Type_material ||
			Data::items[item_id - 1].type == RPG::Item::Type_book) {
			return true;
		} else if (Data::items[item_id - 1].type == RPG::Item::Type_switch) {
			return Data::items[item_id - 1].ocassion_field;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////
void Game_Party::AddActor(int actor_id) {
	Game_Actor* actor;

	actor = Game_Actors::GetActor(actor_id);

	// If the party has less than 4 members and this actor is not in the party
	if ( (actors.size() < 4) && (!IsActorInParty(actor)) ) {
		actors.push_back(actor);
		Main_Data::game_player->Refresh();
	}
}

////////////////////////////////////////////////////////////
void Game_Party::RemoveActor(int actor_id) {
	actors.erase(std::find(actors.begin(), actors.end(), Game_Actors::GetActor(actor_id)));
	Main_Data::game_player->Refresh();
}

////////////////////////////////////////////////////////////
bool Game_Party::IsActorInParty(Game_Actor* actor) {	
	return ( std::find(actors.begin(), actors.end(), actor) != actors.end() );
}

////////////////////////////////////////////////////////////
int Game_Party::GetGold() {
	return gold;
}

int Game_Party::GetSteps() {
	return steps;
}

std::vector<Game_Actor*>& Game_Party::GetActors() {
	return actors;
}

int Game_Party::GetBattleCount() {
	return battle_count;
}

int Game_Party::GetWinCount() {
	return win_count;
}

int Game_Party::GetDefeatCount() {
	return defeat_count;
}

int Game_Party::GetRunCount() {
	return run_count;
}
