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
#include "scene_name.h"
#include "main_data.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"
#include "game_system.h"
#include "game_temp.h"
#include "rpg_system.h"
#include "player.h"
#include "graphics.h"
#include "audio.h"
#include "input.h"

////////////////////////////////////////////////////////////
Scene_Map::Scene_Map() : 
	spriteset(NULL),
	message_window(NULL) {
	type = Scene::Map;
}

////////////////////////////////////////////////////////////
void Scene_Map::Start() {
	spriteset = new Spriteset_Map();
	message_window = new Window_Message(0, 240 - 80, 320, 80);

	Graphics::FrameReset();
}

////////////////////////////////////////////////////////////
void Scene_Map::Terminate() {
	delete spriteset;
	delete message_window;
}

////////////////////////////////////////////////////////////
void Scene_Map::Update() {
	Game_Map::GetInterpreter().Update();
	Game_Map::Update();
	Main_Data::game_player->Update();
	spriteset->Update();
	message_window->Update();

	if (Game_Message::visible) 
		return;

	UpdateTeleportPlayer();

	// ESC-Menu calling
	if (Input::IsTriggered(Input::CANCEL))
	{
		//if (Game_Map::GetInterpreter().IsRunning())
			//return;
		//$game_system.menu_disabled


		Game_Temp::menu_calling = true;
		Game_Temp::menu_beep = true;
	}

	if (!Main_Data::game_player->IsMoving()) {
		if (Game_Temp::menu_calling) {
			CallMenu();
			return;
		}

		if (Game_Temp::name_calling) {
			CallName();
			return;
		}

		if (Game_Temp::transition_processing) {
			Game_Temp::transition_processing = false;

			Graphics::Transition(Game_Temp::transition_type, 32, Game_Temp::transition_erase);
		}
	}
}

void Scene_Map::UpdateTeleportPlayer() {
	if (!Main_Data::game_player->IsTeleporting())
		return;

	Scene::PerformTransition();

	delete spriteset;
	Main_Data::game_player->PerformTeleport();
	Game_Map::Autoplay();
	Game_Map::Update();
	//Graphics::Wait(15);

	spriteset = new Spriteset_Map();

	Scene::PerformTransition();

	Input::Update();
}

////////////////////////////////////////////////////////////
/// Menu Calling Stuff
////////////////////////////////////////////////////////////
void Scene_Map::CallBattle() {

}

void Scene_Map::CallShop() {

}

void Scene_Map::CallName() {
	Game_Temp::name_calling = false;

	Scene::Push(new Scene_Name());
}

////////////////////////////////////////////////////////////
/// ESC-Menu Call
////////////////////////////////////////////////////////////
void Scene_Map::CallMenu() {
	Game_Temp::menu_calling = false;

	if (Game_Temp::menu_beep) {
		Game_System::SePlay(Data::system.decision_se);
		Game_Temp::menu_beep = false;
	}

	// TODO: Main_Data::game_player->Straighten();

	Scene::Push(new Scene_Menu());
}

void Scene_Map::CallSave() {

}

void Scene_Map::CallDebug() {

}

