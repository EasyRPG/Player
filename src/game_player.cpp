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
#include "input.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Player::Game_Player()
{
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Player::~Game_Player()
{
}

////////////////////////////////////////////////////////////
/// Is Passable
////////////////////////////////////////////////////////////
bool Game_Player::IsPassable(int x, int y, int d)
{
	return true;
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Game_Player::Update()
{
	/*if (active && item_max > 0 && index >= 0) {
		if (Input::(Input::DOWN)) {
			if ((column_max == 1 && Input::IsTriggered(Input::DOWN)) || index < item_max - column_max) {
				Main_Data::game_system->SePlay(Main_Data::data_system.cursor_se);
				index = (index + column_max) % item_max;
			}
		}
		if (Input::IsRepeated(Input::UP)) {
			if ((column_max == 1 && Input::IsTriggered(Input::UP)) || index >= column_max) {
				Main_Data::game_system->SePlay(Main_Data::data_system.cursor_se);
				index = (index - column_max + item_max) % item_max;
			}
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			if (column_max >= 2 && index < item_max - 1) {
				Main_Data::game_system->SePlay(Main_Data::data_system.cursor_se);
				index += 1;
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			if (column_max >= 2 && index > 0) {
				Main_Data::game_system->SePlay(Main_Data::data_system.cursor_se);
				index -= 1;
			}
		}
	}*/
}
