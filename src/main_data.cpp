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
#include "main_data.h"
#include "game_party.h"
#include "game_player.h"
#include "game_screen.h"
#include "game_troop.h"
#include "game_message.h"
#include "game_variables.h"
#include "game_switches.h"

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
Game_Variables_Class Game_Variables;
Game_Switches_Class Game_Switches;

namespace Main_Data {
	// Scene Data
	Scene* scene = NULL;
	Scene* old_scene = NULL;
	unsigned char scene_type;
	
	// Dynamic Game Data
	Game_Screen* game_screen = NULL;
	Game_Player* game_player = NULL;
	Game_Party* game_party = NULL;
	Game_Troop* game_troop = NULL;
	Game_Message* game_message = NULL;
}

void Main_Data::Cleanup() {
	delete scene;
	delete game_screen;
	delete game_party;
	delete game_troop;
	delete game_player;
	scene = NULL;
	game_screen = NULL;
	game_party = NULL;
	game_troop = NULL;
	game_player = NULL;
}
