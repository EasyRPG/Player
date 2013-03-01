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

Scene_Equip::Scene_Equip(int actor_index, int equip_index) :
	actor_index(actor_index),
	equip_index(equip_index) {
	type = Scene::Equip;
}

void Scene_Equip::Start() {
	Game_Actor* actor = Game_Party::GetActors()[actor_index];

	// Create the windows
	help_window.reset(new Window_Help(0, 0, 320, 32));
	equipstatus_window.reset(new Window_EquipStatus(0, 32, 124, 96, actor->GetId()));
	equip_window.reset(new Window_Equip(124, 32, 196, 96, actor->GetId()));

	equip_window->SetIndex(equip_index);

	for (int i = 0; i < 5; ++i) {
		item_windows.push_back(EASYRPG_MAKE_SHARED<Window_EquipItem>(actor->GetId(), i));
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
		Game_Actor* actor = Game_Party::GetActors()[actor_index];
		int old_item = actor->SetEquipment(equip_window->GetIndex(),
			item_window->GetItemId());

		equipstatus_window->SetNewParameters(
			actor->GetAtk(), actor->GetDef(), actor->GetSpi(), actor->GetAgi());

		actor->SetEquipment(equip_window->GetIndex(), old_item);

		equipstatus_window->Refresh();
	}

	equipstatus_window->Update();
}

void Scene_Equip::UpdateEquipSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		equip_window->SetActive(false);
		item_window->SetActive(true);
		item_window->SetIndex(0);
	} else if (Game_Party::GetActors().size() > 1 && Input::IsTriggered(Input::RIGHT)) {
		Game_System::SePlay(Data::system.cursor_se);
		actor_index = (actor_index + 1) % Game_Party::GetActors().size();
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Equip>(actor_index, equip_window->GetIndex()), true);
	} else if (Game_Party::GetActors().size() > 1 && Input::IsTriggered(Input::LEFT)) {
		Game_System::SePlay(Data::system.cursor_se);
		actor_index = (actor_index + Game_Party::GetActors().size() - 1) % Game_Party::GetActors().size();
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Equip>(actor_index, equip_window->GetIndex()), true);
	}
}

void Scene_Equip::UpdateItemSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		equip_window->SetActive(true);
		item_window->SetActive(false);
		item_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);

		Game_Party::GetActors()[actor_index]->ChangeEquipment(
			equip_window->GetIndex(), item_window->GetItemId());

		equip_window->SetActive(true);
		item_window->SetActive(false);
		item_window->SetIndex(-1);

		equip_window->Refresh();

		for (size_t i = 0; i < item_windows.size(); ++i) {
			item_windows[i]->Refresh();
		}
	}
}
