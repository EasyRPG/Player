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
#include "scene_map.h"
#include "main_data.h"
#include "game_map.h"
#include "game_player.h"
#include "player.h"
#include "graphics.h"
#include "audio.h"
#include "input.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Scene_Map::Scene_Map() {
	Main_Data::scene_type = SCENE_MAP;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Scene_Map::~Scene_Map() {
}

////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
void Scene_Map::MainFunction() {	
	spriteset = new Spriteset_Map();
	//message_window = new Window_Message();

	//Graphics::Transition();

	// Scene loop
	while (Main_Data::scene_type == SCENE_MAP) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}
	
	// Delete objects
	delete spriteset;
	//delete message_window;
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Scene_Map::Update() {
	Main_Data::game_map->Update();
	Main_Data::game_player->Update();
	spriteset->Update();
}
