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
#include "scene_menu.h"
#include "graphics.h"
#include "game_party.h"
#include "game_system.h"
#include "input.h"
#include "player.h"
#include "scene_map.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Scene_Menu::Scene_Menu(int menu_index) {
	this->menu_index = menu_index;

	Main_Data::scene_type = SCENE_MENU;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Scene_Menu::~Scene_Menu() {
}

////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
void Scene_Menu::MainFunction() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Main_Data::data_terms.command_item);
	options.push_back(Main_Data::data_terms.command_skill);
	options.push_back(Main_Data::data_terms.menu_equipment);
	options.push_back(Main_Data::data_terms.menu_save);
	options.push_back(Main_Data::data_terms.menu_quit);

	command_window = new Window_Command(88, options);
	command_window->SetIndex(menu_index);

	// If there are no actors in the party disable Skills and Equipment
	// RPG2k does not do this, but crashes if you try to access these menus
	if (Main_Data::game_party->actors.size() == 0) {
		command_window->DisableItem(1);
		command_window->DisableItem(2);
	}

	// If save is forbidden disable this item
	if (Main_Data::game_system->save_disabled) {
		command_window->DisableItem(3);
	}

	// Gold Window
	gold_window = new Window_Gold();
	gold_window->SetX(0);
	gold_window->SetY(208);

	// Status Window
	menustatus_window = new Window_MenuStatus();
	menustatus_window->SetX(88);
	menustatus_window->SetY(0);
	menustatus_window->SetActive(false);

	//Graphics::transition();

	while (Main_Data::scene_type == SCENE_MENU) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}

	delete command_window;
	delete gold_window;
	delete menustatus_window;
}

////////////////////////////////////////////////////////////
/// General Frame Update
////////////////////////////////////////////////////////////
void Scene_Menu::Update() {
	command_window->Update();
	gold_window->Update();
	menustatus_window->Update();

	if (command_window->GetActive())
		UpdateCommand();
	else if (menustatus_window->GetActive())
		UpdateStatus();
}


////////////////////////////////////////////////////////////
/// Update Function if Command Window is Active
////////////////////////////////////////////////////////////
void Scene_Menu::UpdateCommand() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::data_system.cancel_se);
		Main_Data::scene = new Scene_Map();
	}
}

////////////////////////////////////////////////////////////
/// Update Function if Status Window is Active
////////////////////////////////////////////////////////////
void Scene_Menu::UpdateStatus() {
}
