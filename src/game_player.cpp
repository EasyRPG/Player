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

#define min(a, b)	(((a) < (b)) ? (a) : (b))
#define max(a, b)	(((a) > (b)) ? (a) : (b))

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Player::Game_Player() : Game_Character() {
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Player::~Game_Player() {
}

////////////////////////////////////////////////////////////
/// Is Passable
////////////////////////////////////////////////////////////
bool Game_Player::IsPassable(int x, int y, int d)
{
	return true;
}

////////////////////////////////////////////////////////////
/// Center
////////////////////////////////////////////////////////////
void Game_Player::Center(int x, int y) {
	// *16 is the tilemap size
	int max_x = (Main_Data::game_map->GetWidth() - Main_Data::game_map->GetWidth() / 16) * 16;
    int max_y = (Main_Data::game_map->GetHeight() - Main_Data::game_map->GetHeight() / 16) * 16;
	int center_x = (Player::GetWidth() / 2);
	int center_y = (Player::GetHeight() / 2);
    Main_Data::game_map->display_x = max(0, min(x * 16 - center_x, max_x));
    Main_Data::game_map->display_y = max(0, min(y * 16 - center_y, max_y));
}

////////////////////////////////////////////////////////////
/// MoveTo
////////////////////////////////////////////////////////////
void Game_Player::MoveTo(int x, int y) {
	Game_Character::MoveTo(x, y);
	Center(x, y);
	// MakeEncounterCount();
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Game_Player::Update() {

}
