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
#include "scene_menu.h"
#include "scene_title.h"
#include "main_data.h"
#include "game_map.h"
#include "game_player.h"
#include "game_system.h"
#include "game_temp.h"
#include "rpg_system.h"
#include "player.h"
#include "graphics.h"
#include "audio.h"
#include "input.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Scene_Map::Scene_Map() {
	message_window = NULL;
	spriteset = NULL;
	
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

	Graphics::Transition(Graphics::FadeIn, 10, true);

	// Scene loop
	while (Main_Data::scene_type == SCENE_MAP) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}

	Graphics::Transition(Graphics::FadeOut, 10, false);

	// Wait for the transition to finish
	do {
		Graphics::Update();
	} while (Graphics::is_in_transition_yet);
	
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

	// ESC-Menu calling
	if (Input::IsTriggered(Input::CANCEL))
	{
		//unless $game_system.map_interpreter.running? or
			//$game_system.menu_disabled

		Main_Data::game_temp->menu_calling = true;
		Main_Data::game_temp->menu_beep = true;
	}

	if (!Main_Data::game_player->IsMoving())
	{
		if (Main_Data::game_temp->menu_calling)
			CallMenu();
	}
}

////////////////////////////////////////////////////////////
/// Menu Calling Stuff
////////////////////////////////////////////////////////////
void Scene_Map::CallBattle() {

}

void Scene_Map::CallShop() {

}

void Scene_Map::CallName() {

}

////////////////////////////////////////////////////////////
/// ESC-Menu Call
////////////////////////////////////////////////////////////
void Scene_Map::CallMenu() {
	Main_Data::game_temp->menu_calling = false;

	if (Main_Data::game_temp->menu_beep) {
		Main_Data::game_system->SePlay(Main_Data::data_system.decision_se);
		Main_Data::game_temp->menu_beep = false;
	}

	// TODO: Main_Data::game_player->Straighten();

	Main_Data::scene = new Scene_Menu();
}

void Scene_Map::CallSave() {

}

void Scene_Map::CallDebug() {

}

