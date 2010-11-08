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
#include "game_map.h"
#include "input.h"
#include "main_data.h"
#include "player.h"

////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////
#define min(a, b)	(((a) < (b)) ? (a) : (b))
#define max(a, b)	(((a) > (b)) ? (a) : (b))

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
bool Game_Player::IsPassable(int x, int y, int d) {
	int new_x = x;
	int new_y = y;

	if (d == 2) {
		++new_y;
	}
	else if (d == 8) {
		--new_y;
	}

	if (d == 4) {
		--new_x;
	}
	else if (d == 6) {
		++new_x;
	}

#ifdef _DEBUG
	if (Input::IsPressed(Input::SHIFT)) {
		// Walk through walls in debug mode
		return true;
	}
#endif

	return Main_Data::game_map->IsValid(new_x, new_y);
}

////////////////////////////////////////////////////////////
// Center
////////////////////////////////////////////////////////////
void Game_Player::Center(int x, int y) {
	int center_x = (Player::GetWidth() / 2 - 16) * 8;
	int center_y = (Player::GetHeight() / 2 - 8) * 8;

	int max_x = (Main_Data::game_map->GetWidth() - Player::GetWidth() / 16) * 128;
	int max_y = (Main_Data::game_map->GetHeight() - Player::GetHeight() / 16) * 128;
	Main_Data::game_map->display_x = max(0, min((x * 128 - center_x), max_x));
	Main_Data::game_map->display_y = max(0, min((y * 128 - center_y), max_y));
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

	if (!(IsMoving() /*|| Main_Data::game_system->map_interpreter.IsRunning() ||
		move_route_forcing || Main_Data::game_temp->message_window_showing*/)) {
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
	
	int center_x = (Player::GetWidth() / 2 - 16) * 8;
	int center_y = (Player::GetHeight() / 2 - 8) * 8;

	if (real_y > last_real_y && real_y - Main_Data::game_map->display_y > center_y)
		Main_Data::game_map->ScrollDown(real_y - last_real_y);

	if (real_x < last_real_x && real_x - Main_Data::game_map->display_x < center_x)
		Main_Data::game_map->ScrollLeft(last_real_x - real_x);

	if (real_x > last_real_x && real_x - Main_Data::game_map->display_x > center_x)
		Main_Data::game_map->ScrollRight(real_x - last_real_x);

	if (real_y < last_real_y && real_y - Main_Data::game_map->display_y < center_y)
		Main_Data::game_map->ScrollUp(last_real_y - real_y);

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
