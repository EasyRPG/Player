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
#include "game_character.h"
#include "game_map.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Character::Game_Character()
{
	int x = 0;
	int y = 0;
	int real_x = 0;
	int real_y = 0;
	int prelock_direction = 0;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Character::~Game_Character()
{
}

////////////////////////////////////////////////////////////
/// MoveTo
////////////////////////////////////////////////////////////
void Game_Character::MoveTo(int x, int y)
{
	x = x % Main_Data::game_map->GetWidth();
    y = y % Main_Data::game_map->GetHeight();
    real_x = x * 128;
    real_y = y * 128;
    prelock_direction = 0;
}
