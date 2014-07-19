/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <cassert>
#include "scene_menu.h"
#include "audio.h"
#include "cache.h"
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

Scene_Menu::Scene_Menu(int menu_index) : menu_index(menu_index) { type = Scene::Menu; }

void Scene_Menu::Start() {
	DisplayUi->SetBackcolor(Cache::system_info.bg_color);

	CreateCommandWindow();

	// Gold Window
	gold_window.reset(new Window_Gold(0, (SCREEN_TARGET_HEIGHT - 32), 88, 32));

	// Status Window
	menustatus_window.reset(
	    new Window_MenuStatus(88, 0, (SCREEN_TARGET_WIDTH - 88), SCREEN_TARGET_HEIGHT));
	menustatus_window->SetActive(false);
}

void Scene_Menu::Continue() {
	if (command_options[command_window->GetIndex()] == Order) {
		menustatus_window->Refresh();
	}
}

void Scene_Menu::Update() {
	command_window->Update();
	gold_window->Update();
	menustatus_window->Update();

	if (command_window->GetActive()) {
		UpdateCommand();
	} else if (menustatus_window->GetActive()) {
		UpdateActorSelection();
	}
}

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
		for (std::vector<int16_t>::iterator it = Data::system.menu_commands.begin();
		     it != Data::system.menu_commands.end(); ++it) {
			command_options.push_back((CommandOptionType) * it);
		}
		command_options.push_back(Quit);
	}

	// Add all menu items
	std::vector<CommandOptionType>::iterator it;
	for (it = command_options.begin(); it != command_options.end(); ++it) {
		switch (*it) {
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

	command_window.reset(new Window_Command(options, 88));
	command_window->SetIndex(menu_index);

	// Disable items
	for (it = command_options.begin(); it != command_options.end(); ++it) {
		switch (*it) {
		case Save:
			// If save is forbidden disable this item
			if (!Game_System::GetAllowSave()) {
				command_window->DisableItem(it - command_options.begin());
			}
		case Wait:
		case Quit:
			break;
		case Order:
			if (Main_Data::game_party->GetActors().size() <= 1) {
				command_window->DisableItem(it - command_options.begin());
			}
			break;
		default:
			if (Main_Data::game_party->GetActors().empty()) {
				command_window->DisableItem(it - command_options.begin());
			}
			break;
		}
	}
}

void Scene_Menu::UpdateCommand() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		menu_index = command_window->GetIndex();

		switch (command_options[menu_index]) {
		case Item:
			if (Main_Data::game_party->GetActors().empty()) {
				Game_System::SePlay(Main_Data::game_data.system.buzzer_se);
			} else {
				Game_System::SePlay(Main_Data::game_data.system.decision_se);
				Scene::Push(EASYRPG_MAKE_SHARED<Scene_Item>());
			}
			break;
		case Skill:
		case Equipment:
		case Status:
		case Row:
			if (Main_Data::game_party->GetActors().empty()) {
				Game_System::SePlay(Main_Data::game_data.system.buzzer_se);
			} else {
				Game_System::SePlay(Main_Data::game_data.system.decision_se);
				command_window->SetActive(false);
				menustatus_window->SetActive(true);
				menustatus_window->SetIndex(0);
			}
			break;
		case Save:
			if (!Game_System::GetAllowSave()) {
				Game_System::SePlay(Main_Data::game_data.system.buzzer_se);
			} else {
				Game_System::SePlay(Main_Data::game_data.system.decision_se);
				Scene::Push(EASYRPG_MAKE_SHARED<Scene_Save>());
			}
			break;
		case Order:
			if (Main_Data::game_party->GetActors().size() <= 1) {
				Game_System::SePlay(Main_Data::game_data.system.buzzer_se);
			} else {
				Game_System::SePlay(Main_Data::game_data.system.decision_se);
				Scene::Push(EASYRPG_MAKE_SHARED<Scene_Order>());
			}
			break;
		case Wait:
			Game_System::SePlay(Main_Data::game_data.system.decision_se);
			Game_Temp::battle_wait = !Game_Temp::battle_wait;
			command_window->SetItemText(
			    menu_index, Game_Temp::battle_wait ? Data::terms.wait_on : Data::terms.wait_off);
			break;
		case Quit:
			Game_System::SePlay(Main_Data::game_data.system.decision_se);
			Scene::Push(EASYRPG_MAKE_SHARED<Scene_End>());
			break;
		}
	}
}

void Scene_Menu::UpdateActorSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		command_window->SetActive(true);
		menustatus_window->SetActive(false);
		menustatus_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Main_Data::game_data.system.decision_se);
		switch (command_options[command_window->GetIndex()]) {
		case Skill:
			Scene::Push(EASYRPG_MAKE_SHARED<Scene_Skill>(menustatus_window->GetIndex()));
			break;
		case Equipment:
			Scene::Push(EASYRPG_MAKE_SHARED<Scene_Equip>(menustatus_window->GetIndex()));
			break;
		case Status:
			Scene::Push(EASYRPG_MAKE_SHARED<Scene_Status>(menustatus_window->GetIndex()));
			break;
		case Row: {
			Game_Actor* actor = Main_Data::game_party->GetActors()[menustatus_window->GetIndex()];
			actor->GetBattleRow() == -1 ? actor->SetBattleRow(1) : actor->SetBattleRow(-1);
			menustatus_window->Refresh();
			break;
		}
		default:
			assert(false);
			break;
		}

		command_window->SetActive(true);
		menustatus_window->SetActive(false);
		menustatus_window->SetIndex(-1);
	}
}
