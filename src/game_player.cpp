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
#include "game_party.h"
#include "input.h"
#include "main_data.h"
#include "player.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////
Game_Player::Game_Player() {
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

////////////////////////////////////////////////////////////
// Update
////////////////////////////////////////////////////////////
void Game_Player::Update() {
//	bool last_moving = IsMoving();

	if (Game_Map::GetInterpreter().IsRunning()) return;

	if (!(IsMoving() /*|| Game_System::map_interpreter.IsRunning() ||
		move_route_forcing || Game_Temp::message_window_showing*/)) {
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

	/*if (!IsMoving()) {
		if (last_moving) {
			bool result = check_event_trigger_here(1, 2);
			#ifndef DEBUG
			if (!result && Input.press?(Input::CTRL)) {
				if (encounter_count > 0) encounter_count -= 1;
			}
			#endif
		}
		if Input.trigger?(Input::C)
			check_event_trigger_here([0])
			check_event_trigger_there([0,1,2])
		end
	}*/
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
