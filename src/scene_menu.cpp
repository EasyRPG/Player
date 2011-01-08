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
#include "audio.h"
#include "graphics.h"
#include "game_party.h"
#include "game_system.h"
#include "input.h"
#include "player.h"
#include "scene_end.h"
#include "scene_equip.h"
#include "scene_item.h"
#include "scene_map.h"
#include "scene_skill.h"

////////////////////////////////////////////////////////////
Scene_Menu::Scene_Menu(int menu_index) :
	menu_index(menu_index) {
	type = Scene::Menu;
}

////////////////////////////////////////////////////////////
void Scene_Menu::Start() {
	CreateCommandWindow();

	// Gold Window
	gold_window = new Window_Gold(0, 208, 88, 32);

	// Status Window
	menustatus_window = new Window_MenuStatus(88, 0, 232, 240);
	menustatus_window->SetActive(false);
}

////////////////////////////////////////////////////////////
void Scene_Menu::Terminate() {
	delete command_window;
	delete gold_window;
	delete menustatus_window;
}

////////////////////////////////////////////////////////////
void Scene_Menu::Update() {
	command_window->Update();
	gold_window->Update();
	menustatus_window->Update();

	if (command_window->GetActive()) {
		UpdateCommand();
	}
	else if (menustatus_window->GetActive()) {
		UpdateActorSelection();
	}
}

////////////////////////////////////////////////////////////
void Scene_Menu::CreateCommandWindow() {
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
		//command_window->DisableItem(0);
		command_window->DisableItem(1);
		command_window->DisableItem(2);
	}

	// If save is forbidden disable this item
	if (Game_System::save_disabled) {
		command_window->DisableItem(3);
	}
}

////////////////////////////////////////////////////////////
void Scene_Menu::UpdateCommand() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		menu_index = command_window->GetIndex();

		switch (menu_index) {
		case 0: // Item
			Game_System::SePlay(Data::system.decision_se);
			Scene::Push(new Scene_Item());
			break;
		case 1: // Tech Skill
		case 2: // Equipment
			if (Game_Party::GetActors().empty()) {
				Game_System::SePlay(Data::system.buzzer_se);
			} else {
				Game_System::SePlay(Data::system.decision_se);
				command_window->SetActive(false);
				menustatus_window->SetActive(true);
				menustatus_window->SetIndex(0);
			}
			break;
		case 3: // Save
			if (Game_System::save_disabled) {
				Game_System::SePlay(Data::system.buzzer_se);
			} else {
				Game_System::SePlay(Data::system.decision_se);
			}

			// Debug Test code to add items
			for (int i = 1; i < 82; ++i) {
				Game_Party::GainItem(i, 1);
			}
			break;
		case 4: // Quit Game
			Game_System::SePlay(Data::system.decision_se);
			Scene::Push(new Scene_End());
			break;
		}
	}
}

////////////////////////////////////////////////////////////
void Scene_Menu::UpdateActorSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		command_window->SetActive(true);
		menustatus_window->SetActive(false);
		menustatus_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		switch (command_window->GetIndex()) {
		case 1: // Tech Skill
			Scene::Push(new Scene_Skill(menustatus_window->GetIndex()));
			break;
		case 2: // Equipment
			Scene::Push(new Scene_Equip(menustatus_window->GetIndex()));
			break;
		}
	}
}
