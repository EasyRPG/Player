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
#include "scene_equip.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_system.h"
#include "graphics.h"
#include "input.h"
#include "player.h"
#include "scene_menu.h"
#include "rpg_item.h"

Scene_Equip::Scene_Equip(Game_Actor& actor, int equip_index) :
	actor(actor),
	equip_index(equip_index) {
	type = Scene::Equip;
}

void Scene_Equip::Start() {
	// Create the windows
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	equipstatus_window.reset(new Window_EquipStatus(0, 32, 124, 96, actor.GetId()));
	equip_window.reset(new Window_Equip(124, 32, (SCREEN_TARGET_WIDTH-124),96, actor.GetId()));

	equip_window->SetIndex(equip_index);

	for (int i = 0; i < 5; ++i) {
		item_windows.push_back(std::make_shared<Window_EquipItem>(actor.GetId(), i));
	}

	// Assign the help windows
	equip_window->SetHelpWindow(help_window.get());
	for (size_t i = 0; i < item_windows.size(); ++i) {
		item_windows[i]->SetHelpWindow(help_window.get());
		item_windows[i]->SetActive(false);
		item_windows[i]->Refresh();
	}
}

void Scene_Equip::Update() {
	help_window->Update();

	UpdateEquipWindow();
	UpdateStatusWindow();
	UpdateItemWindows();

	if (equip_window->GetActive()) {
		UpdateEquipSelection();
	} else if (item_window->GetActive()) {
		UpdateItemSelection();
	}
}

void Scene_Equip::UpdateItemWindows() {
	for (size_t i = 0; i < item_windows.size(); ++i) {
		item_windows[i]->SetVisible((unsigned)equip_window->GetIndex() == i);
		item_windows[i]->Update();
	}

	item_window = item_windows[equip_window->GetIndex()];
}

void Scene_Equip::UpdateEquipWindow() {
	equip_window->Update();
}

void Scene_Equip::UpdateStatusWindow() {
	if (equip_window->GetActive()) {
		equipstatus_window->ClearParameters();
	} else if (item_window->GetActive()) {
		const RPG::Item* current_item = item_window->GetItem();
		int current_item_id = current_item ? current_item->ID : 0;

		int old_item = actor.SetEquipment(equip_window->GetIndex() + 1,
			current_item_id);

		equipstatus_window->SetNewParameters(
			actor.GetAtk(), actor.GetDef(), actor.GetSpi(), actor.GetAgi());

		actor.SetEquipment(equip_window->GetIndex() + 1, old_item);

		equipstatus_window->Refresh();
	}

	equipstatus_window->Update();
}

void Scene_Equip::UpdateEquipSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (actor.IsEquipmentFixed()) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			return;
		}

		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		equip_window->SetActive(false);
		item_window->SetActive(true);
		item_window->SetIndex(0);
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::RIGHT)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
		int actor_index = Main_Data::game_party->GetActorPositionInParty(actor.GetId());
		actor_index = (actor_index + 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(std::make_shared<Scene_Equip>((*Main_Data::game_party)[actor_index], equip_window->GetIndex()), true);
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::LEFT)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
		int actor_index = Main_Data::game_party->GetActorPositionInParty(actor.GetId());
		actor_index = (actor_index + Main_Data::game_party->GetActors().size() - 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(std::make_shared<Scene_Equip>((*Main_Data::game_party)[actor_index], equip_window->GetIndex()), true);
	}
}

void Scene_Equip::UpdateItemSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		equip_window->SetActive(true);
		item_window->SetActive(false);
		item_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

		const RPG::Item* current_item = item_window->GetItem();
		int current_item_id = current_item ? current_item->ID : 0;

		actor.ChangeEquipment(
			equip_window->GetIndex() + 1, current_item_id);

		equip_window->SetActive(true);
		item_window->SetActive(false);
		item_window->SetIndex(-1);

		equip_window->Refresh();

		for (size_t i = 0; i < item_windows.size(); ++i) {
			item_windows[i]->Refresh();
		}
	}
}
