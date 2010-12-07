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
#include "game_player.h"
#include "game_actor.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "input.h"
#include "main_data.h"
#include "player.h"
#include "util_macro.h"
#include <algorithm>

////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////
Game_Player::Game_Player() {
	SetAssocChild(this);
}

////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////
Game_Player::~Game_Player() {
}

////////////////////////////////////////////////////////////
// Is Passable
////////////////////////////////////////////////////////////
bool Game_Player::IsPassable(int x, int y, int d) const {
	int new_x = x + (d == 6 ? 1 : d == 4 ? -1 : 0);
	int new_y = y + (d == 2 ? 1 : d == 8 ? -1 : 0);

	if (!Game_Map::IsValid(new_x, new_y)) return false;

#ifdef _DEBUG
	if (Input::IsPressed(Input::DEBUG_THROUGH)) return true;
#endif

	return Game_Character::IsPassable(x, y, d);
}

////////////////////////////////////////////////////////////
// Center
////////////////////////////////////////////////////////////
void Game_Player::Center(int x, int y) {
	int center_x = (DisplayUi->GetWidth() / 2 - 16) * 8;
	int center_y = (DisplayUi->GetHeight() / 2 - 8) * 8;

	int max_x = (Game_Map::GetWidth() - DisplayUi->GetWidth() / 16) * 128;
	int max_y = (Game_Map::GetHeight() - DisplayUi->GetHeight() / 16) * 128;
	Game_Map::SetDisplayX(max(0, min((x * 128 - center_x), max_x)));
	Game_Map::SetDisplayY(max(0, min((y * 128 - center_y), max_y)));
}

////////////////////////////////////////////////////////////
// MoveTo
////////////////////////////////////////////////////////////
void Game_Player::MoveTo(int x, int y) {
	Game_Character::MoveTo(x, y);
	Center(x, y);
	// MakeEncounterCount();
}

void Game_Player::UpdateScroll(int last_real_x, int last_real_y) {
	int center_x = (DisplayUi->GetWidth() / 2 - 16) * 8;
	int center_y = (DisplayUi->GetHeight() / 2 - 8) * 8;

	if (real_y > last_real_y && real_y - Game_Map::GetDisplayY() > center_y)
		Game_Map::ScrollDown(real_y - last_real_y);

	if (real_x < last_real_x && real_x - Game_Map::GetDisplayX() < center_x)
		Game_Map::ScrollLeft(last_real_x - real_x);

	if (real_x > last_real_x && real_x - Game_Map::GetDisplayX() > center_x)
		Game_Map::ScrollRight(real_x - last_real_x);

	if (real_y < last_real_y && real_y - Game_Map::GetDisplayY() < center_y)
		Game_Map::ScrollUp(last_real_y - real_y);

}

////////////////////////////////////////////////////////////
// Update
////////////////////////////////////////////////////////////
void Game_Player::Update() {
	bool last_moving = IsMoving();

	if (!IsMoving() && !Game_Map::GetInterpreter().IsRunning()
		/*move_route_forcing || Game_Temp::message_window_showing*/) {
		switch (Input::dir4) {
			case 2:
				MoveDown();
				break;
			case 4:
				MoveLeft();
				break;
			case 6:
				MoveRight();
				break;
			case 8:
				MoveUp();
		}
	}

	int last_real_x = real_x;
	int last_real_y = real_y;

	Game_Character::Update();

	UpdateScroll(last_real_x, last_real_y);

	UpdateNonMoving(last_moving);
}

void Game_Player::UpdateNonMoving(bool last_moving) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return;

	if ( IsMoving() ) return;

	if ( last_moving && CheckTouchEvent() ) return;

	if ( !Game_Message::visible && Input::IsTriggered(Input::DECISION) ) {
		// TODO 
		//if ( GetOnOffVehicle() ) return;
		if ( CheckActionEvent() ) return;
	}

	if ( last_moving ) {
		// TODO
//		UpdateEncounter();
	}
}

bool Game_Player::CheckActionEvent() {
	// TODO
	//if ( IsInAirship() ) {
		//return false;
	//}
	int triggers_here[] = { 0 };
	std::vector<int> triggers(triggers_here, triggers_here + sizeof triggers_here / sizeof(int));

	if ( CheckEventTriggerHere(triggers) ) {
		return true;
	}

	int triggers_there[] = { 0, 1, 2 };
	triggers.assign(triggers_there, triggers_there + sizeof triggers_there / sizeof(int));

	return CheckEventTriggerThere(triggers);

}

bool Game_Player::CheckTouchEvent() {
	int triggers[] = { 1, 2 };
	std::vector<int> v_triggers( triggers, triggers + sizeof triggers / sizeof(int) );
	return CheckEventTriggerHere(v_triggers);
}

bool Game_Player::CheckEventTriggerHere(const std::vector<int>& triggers) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, this->x, this->y);

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if ( (*i)->GetPriorityType() == 1 && std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end() ) {
			(*i)->Start();
			result = (*i)->GetStarting();
		}
	}
	return result;
}

bool Game_Player::CheckEventTriggerThere(const std::vector<int>& triggers) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	int front_x = Game_Map::XwithDirection(x, direction);
	int front_y = Game_Map::YwithDirection(y, direction);

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, front_x, front_y);

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if ( (*i)->GetPriorityType() == 1 && 
			std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end() 
		) 
		{
			(*i)->Start();
			result = true;
		}
	}

	if ( !result && Game_Map::IsCounter(front_x, front_y) ) {
		front_x = Game_Map::XwithDirection(x, direction);
		front_y = Game_Map::YwithDirection(y, direction);

		Game_Map::GetEventsXY(events, front_x, front_y);

		std::vector<Game_Event*>::iterator i;
		for (i = events.begin(); i != events.end(); i++) {
			if ( (*i)->GetPriorityType() == 1 && 
				std::find(triggers.begin(), triggers.end(), (*i)->GetTrigger() ) != triggers.end() 
			) 
			{
				(*i)->Start();
				result = true;
			}
		}
	}
	return result;
}

bool Game_Player::CheckEventTriggerTouch(int x, int y) {
	if ( Game_Map::GetInterpreter().IsRunning() ) return false;

	bool result = false;

	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);

	std::vector<Game_Event*>::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if ( (*i)->GetPriorityType() == 1 && ((*i)->GetTrigger() == 1 || (*i)->GetTrigger() == 2) ) {
			(*i)->Start();
			result = true;
		}
	}
	return result;
}

void Game_Player::Refresh() {
	Game_Actor* actor;

	if (Game_Party::GetActors().empty()) {
		character_name.clear();
		return;
	}

	actor = Game_Party::GetActors()[0];

	character_name = actor->GetCharacterName();
	character_index = actor->GetCharacterIndex();
}
