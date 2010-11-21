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
Scene_Menu::Scene_Menu(int menu_index) :
	menu_index(menu_index) {
	type = SceneType::Menu;
}

////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
void Scene_Menu::MainFunction() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Data::terms.command_item);
	options.push_back(Data::terms.command_skill);
	options.push_back(Data::terms.menu_equipment);
	options.push_back(Data::terms.menu_save);
	options.push_back(Data::terms.menu_quit);

	command_window = new Window_Command(88, options);
	command_window->SetIndex(menu_index);

	// If there are no actors in the party disable Skills and Equipment
	// RPG2k does not do this, but crashes if you try to access these menus
	if (Game_Party::GetActors().empty()) {
		command_window->DisableItem(1);
		command_window->DisableItem(2);
	}

	// If save is forbidden disable this item
	if (Game_System::save_disabled) {
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

	Graphics::Transition(Graphics::FadeIn, 10, false);

	while (type == SceneType::Menu) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}

	Graphics::Transition(Graphics::FadeOut, 10, false);

	// Wait for the transition to finish // FIXME
	do {
		Graphics::Update();
	} while (Graphics::is_in_transition_yet);

	delete command_window;
	delete gold_window;
	delete menustatus_window;

	Scene::old_instance = this;
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
		Game_System::SePlay(Data::system.cancel_se);
		Scene::instance = new Scene_Map();
	}
}

////////////////////////////////////////////////////////////
/// Update Function if Status Window is Active
////////////////////////////////////////////////////////////
void Scene_Menu::UpdateStatus() {
}
