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
#include <cassert>
#include "scene_menu.h"
#include "audio.h"
#include "graphics.h"
#include "game_party.h"
#include "game_system.h"
#include "game_temp.h"
#include "input.h"
#include "player.h"
#include "scene_end.h"
#include "scene_equip.h"
#include "scene_item.h"
#include "scene_map.h"
#include "scene_skill.h"
#include "scene_order.h"
#include "scene_save.h"
#include "scene_status.h"

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
void Scene_Menu::Continue() {
	if (command_options[command_window->GetIndex()] == Order) {
		menustatus_window->Refresh();
	}
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
	
	if (Player::engine == Player::EngineRpg2k) {
		command_options.resize(5);
		command_options[0] = Item;
		command_options[1] = Skill;
		command_options[2] = Equipment;
		command_options[3] = Save;
		command_options[4] = Quit;
	} else {
		for (std::vector<int16>::iterator it = Data::system.menu_commands.begin();
			it != Data::system.menu_commands.end(); ++it) {
				command_options.push_back((CommandOptionType)*it);
		}
		command_options.push_back(Quit);
	}

	// Add all menu items
	std::vector<CommandOptionType>::iterator it;
	for (it = command_options.begin(); it != command_options.end(); ++it) {
		switch(*it) {
		case Item:
			options.push_back(Data::terms.command_item);
			break;
		case Skill:
			options.push_back(Data::terms.command_skill);
			break;
		case Equipment:
			options.push_back(Data::terms.menu_equipment);
			break;
		case Save:
			options.push_back(Data::terms.menu_save);
			break;
		case Status:
			options.push_back(Data::terms.status);
			break;
		case Row:
			options.push_back(Data::terms.row);
			break;
		case Order:
			options.push_back(Data::terms.order);
			break;
		case Wait:
			options.push_back(Game_Temp::battle_wait ? Data::terms.wait_on : Data::terms.wait_off);
			break;
		default:
			options.push_back(Data::terms.menu_quit);
			break;
		}
	}

	command_window = new Window_Command(options, 88);
	command_window->SetIndex(menu_index);

	// Disable items
	for (it = command_options.begin(); it != command_options.end(); ++it) {
		switch(*it) {
		case Save:
			// If save is forbidden disable this item
			if (!Game_System::GetAllowSave()) {
				command_window->DisableItem(it - command_options.begin());
			}
		case Wait:
		case Quit:
			break;
		case Order:
			if (Game_Party::GetActors().size() <= 1) {
				command_window->DisableItem(it - command_options.begin());
			}
			break;
		default:
			if (Game_Party::GetActors().empty()) {
				command_window->DisableItem(it - command_options.begin());
			}
			break;
		}
	}
}

////////////////////////////////////////////////////////////
void Scene_Menu::UpdateCommand() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		menu_index = command_window->GetIndex();

		switch (command_options[menu_index]) {
		case Item:
			if (Game_Party::GetActors().empty()) {
				Game_System::SePlay(Data::system.buzzer_se);
			} else {
				Game_System::SePlay(Data::system.decision_se);
				Scene::Push(new Scene_Item());
			}
			break;
		case Skill:
		case Equipment:
		case Status:
		case Row:
			if (Game_Party::GetActors().empty()) {
				Game_System::SePlay(Data::system.buzzer_se);
			} else {
				Game_System::SePlay(Data::system.decision_se);
				command_window->SetActive(false);
				menustatus_window->SetActive(true);
				menustatus_window->SetIndex(0);
			}
			break;
		case Save: // Save
			if (!Game_System::GetAllowSave()) {
				Game_System::SePlay(Data::system.buzzer_se);
			} else {
				Game_System::SePlay(Data::system.decision_se);
				Scene::Push(new Scene_Save());
			}
/*
#ifdef _DEBUG
			// Debug Test code to add items
			for (int i = 1; i < 82; ++i) {
				Game_Party::GainItem(i, 1);
			}
#endif*/
			break;
		case Order:
			if (Game_Party::GetActors().size() <= 1) {
				Game_System::SePlay(Data::system.buzzer_se);
			} else {
				Game_System::SePlay(Data::system.decision_se);
				Scene::Push(new Scene_Order());
			}
			break;
		case Wait:
			Game_System::SePlay(Data::system.decision_se);
			Game_Temp::battle_wait = !Game_Temp::battle_wait;
			command_window->SetItemText(menu_index, Game_Temp::battle_wait ? Data::terms.wait_on : Data::terms.wait_off);
			break;
		case Quit: // Quit Game
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
		switch (command_options[command_window->GetIndex()]) {
		case Skill: // Tech Skill
			Scene::Push(new Scene_Skill(menustatus_window->GetIndex()));
			break;
		case Equipment: // Equipment
			Scene::Push(new Scene_Equip(menustatus_window->GetIndex()));
			break;
		case Status:
			Scene::Push(new Scene_Status(menustatus_window->GetIndex()));
			break;
		case Row:
			//
			break;
		default:
			assert(false);
			break;
		}

		command_window->SetActive(true);
		menustatus_window->SetActive(false);
		menustatus_window->SetIndex(-1);
	}
}
