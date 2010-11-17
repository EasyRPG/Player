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
#include "game_temp.h"

////////////////////////////////////////////////////////////
bool Game_Temp::menu_calling;
bool Game_Temp::menu_beep;
Game_Battler* Game_Temp::forcing_battler;
bool Game_Temp::battle_calling;
bool Game_Temp::shop_calling;
bool Game_Temp::name_calling;
bool Game_Temp::save_calling;
bool Game_Temp::gameover;
int Game_Temp::common_event_id;

////////////////////////////////////////////////////////////
void Game_Temp::Init() {
	menu_calling = false;
	menu_beep = false;
	forcing_battler = NULL;
	battle_calling = false;
	shop_calling = false;
	name_calling = false;
	save_calling = false;
	gameover = false;
	common_event_id = 0;
}
