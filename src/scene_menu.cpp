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
#include "game_party.h"
#include "game_system.h"
#include "input.h"
#include "player.h"
#include "scene_settings.h"
#include "scene_debug.h"
#include "scene_end.h"
#include "scene_equip.h"
#include "scene_item.h"
#include "scene_skill.h"
#include "scene_order.h"
#include "scene_save.h"
#include "scene_status.h"
#include "bitmap.h"
#include "feature.h"

constexpr int menu_command_width = 88;
constexpr int gold_window_width = 88;
constexpr int gold_window_height = 32;

Scene_Menu::Scene_Menu(int menu_index) :
	menu_index(menu_index) {
	type = Scene::Menu;
}

void Scene_Menu::Start() {
	CreateCommandWindow();

	// Gold Window
	gold_window.reset(new Window_Gold(Player::menu_offset_x, (Player::screen_height - gold_window_height - Player::menu_offset_y), gold_window_width, gold_window_height));
	gold_window->ApplyRtlMirror();

	// Status Window
	menustatus_window.reset(new Window_MenuStatus(Player::menu_offset_x + menu_command_width, Player::menu_offset_y, (MENU_WIDTH - menu_command_width), MENU_HEIGHT));
	menustatus_window->SetActive(false);
	menustatus_window->ApplyRtlMirror();
}

void Scene_Menu::Continue(SceneType /* prev_scene */) {
	menustatus_window->Refresh();
	gold_window->Refresh();
}

void Scene_Menu::vUpdate() {
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

void Scene_Menu::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;

	if (Player::IsRPG2k()) {
		command_options.push_back(Item);
		command_options.push_back(Skill);
		command_options.push_back(Equipment);
		command_options.push_back(Save);
		if (Player::player_config.settings_in_menu.Get()) {
			command_options.push_back(Settings);
		}
		if (Player::debug_flag) {
			command_options.push_back(Debug);
		}
		command_options.push_back(Quit);
	} else {
		for (std::vector<int16_t>::iterator it = lcf::Data::system.menu_commands.begin();
			it != lcf::Data::system.menu_commands.end(); ++it) {
				switch (*it) {
				case Row:
					if (Feature::HasRow()) {
						command_options.push_back((CommandOptionType)*it);
					}
					break;
				case Wait:
					if (Feature::HasRpg2k3BattleSystem()) {
						command_options.push_back((CommandOptionType)*it);
					}
					break;
				default:
					command_options.push_back((CommandOptionType)*it);
					break;
				}
		}
		if (Player::player_config.settings_in_menu.Get()) {
			command_options.push_back(Settings);
		}
		if (Player::debug_flag) {
			command_options.push_back(Debug);
		}
		command_options.push_back(Quit);
	}

	// Add all menu items
	std::vector<CommandOptionType>::iterator it;
	for (it = command_options.begin(); it != command_options.end(); ++it) {
		switch(*it) {
		case Item:
			options.push_back(ToString(lcf::Data::terms.command_item));
			break;
		case Skill:
			options.push_back(ToString(lcf::Data::terms.command_skill));
			break;
		case Equipment:
			options.push_back(ToString(lcf::Data::terms.menu_equipment));
			break;
		case Save:
			options.push_back(ToString(lcf::Data::terms.menu_save));
			break;
		case Status:
			options.push_back(ToString(lcf::Data::terms.status));
			break;
		case Row:
			options.push_back(ToString(lcf::Data::terms.row));
			break;
		case Order:
			options.push_back(ToString(lcf::Data::terms.order));
			break;
		case Wait:
			options.push_back(ToString(Main_Data::game_system->GetAtbMode() == lcf::rpg::SaveSystem::AtbMode_atb_wait ? lcf::Data::terms.wait_on : lcf::Data::terms.wait_off));
			break;
		case Settings:
			options.push_back("Settings");
			break;
		case Debug:
			options.push_back("Debug");
			break;
		default:
			options.push_back(ToString(lcf::Data::terms.menu_quit));
			break;
		}
	}

	command_window.reset(new Window_Command(options, menu_command_width));
	command_window->SetX(Player::menu_offset_x);
	command_window->SetY(Player::menu_offset_y);
	command_window->ApplyRtlMirror();
	command_window->SetIndex(menu_index);

	// Disable items
	for (it = command_options.begin(); it != command_options.end(); ++it) {
		switch(*it) {
		case Save:
			// If save is forbidden disable this item
			if (!Main_Data::game_system->GetAllowSave()) {
				command_window->DisableItem(it - command_options.begin());
			}
		case Wait:
		case Quit:
		case Settings:
		case Debug:
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
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		menu_index = command_window->GetIndex();

		switch (command_options[menu_index]) {
		case Item:
			if (Main_Data::game_party->GetActors().empty()) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Scene::Push(std::make_shared<Scene_Item>());
			}
			break;
		case Skill:
		case Equipment:
		case Status:
		case Row:
			if (Main_Data::game_party->GetActors().empty()) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				command_window->SetActive(false);
				menustatus_window->SetActive(true);
				menustatus_window->SetIndex(0);
			}
			break;
		case Save:
			if (!Main_Data::game_system->GetAllowSave()) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Scene::Push(std::make_shared<Scene_Save>());
			}
			break;
		case Order:
			if (Main_Data::game_party->GetActors().size() <= 1) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Scene::Push(std::make_shared<Scene_Order>());
			}
			break;
		case Wait:
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Main_Data::game_system->ToggleAtbMode();
			command_window->SetItemText(menu_index,
				Main_Data::game_system->GetAtbMode() == lcf::rpg::SaveSystem::AtbMode_atb_wait ? lcf::Data::terms.wait_on : lcf::Data::terms.wait_off);
			break;
		case Settings:
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Decision));
			Scene::Push(std::make_shared<Scene_Settings>());
			break;
		case Debug:
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Scene::Push(std::make_shared<Scene_Debug>());
			break;
		case Quit:
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Scene::Push(std::make_shared<Scene_End>());
			break;
		}
	}
}

void Scene_Menu::UpdateActorSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		command_window->SetActive(true);
		menustatus_window->SetActive(false);
		menustatus_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		switch (command_options[command_window->GetIndex()]) {
		case Skill:
			if (!menustatus_window->GetActor()->CanAct()) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
				return;
			}
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Scene::Push(std::make_shared<Scene_Skill>(Main_Data::game_party->GetActors(), menustatus_window->GetIndex()));
			break;
		case Equipment:
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Scene::Push(std::make_shared<Scene_Equip>(Main_Data::game_party->GetActors(), menustatus_window->GetIndex()));
			break;
		case Status:
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Scene::Push(std::make_shared<Scene_Status>(Main_Data::game_party->GetActors(), menustatus_window->GetIndex()));
			break;
		case Row:
		{
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			// Don't allow entire party in the back row.
			const auto& actors = Main_Data::game_party->GetActors();
			int num_in_back = 0;
			for (auto* actor: actors) {
				if (actor->GetBattleRow() == Game_Actor::RowType::RowType_back) {
					++num_in_back;
				}
			}
			Game_Actor* actor = actors[menustatus_window->GetIndex()];
			if (actor->GetBattleRow() == Game_Actor::RowType::RowType_front) {
				if (num_in_back < int(actors.size() - 1)) {
					actor->SetBattleRow(Game_Actor::RowType::RowType_back);
				}
			} else {
				actor->SetBattleRow(Game_Actor::RowType::RowType_front);
			}
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
