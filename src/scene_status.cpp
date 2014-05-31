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
#include <string>
#include <vector>
#include "scene_status.h"
#include "game_party.h"
#include "game_system.h"
#include "input.h"

Scene_Status::Scene_Status(int actor_index) :
	actor_index(actor_index) {
	type = Scene::Status;
}

void Scene_Status::Start() {
	int actor = Main_Data::game_party->GetActors()[actor_index]->GetId();

	actorinfo_window.reset(new Window_ActorInfo(0, 0, 124, 208, actor));
	actorstatus_window.reset(new Window_ActorStatus(124, 0, 196, 64, actor));
	gold_window.reset(new Window_Gold(0, 208, 124, 32));
	equipstatus_window.reset(new Window_EquipStatus(124, 64, 196, 80, actor, false));
	equip_window.reset(new Window_Equip(124, 144, 196, 96, actor));

	equip_window->SetActive(false);
	equipstatus_window->SetActive(false);

	equip_window->SetIndex(-1);
}

void Scene_Status::Update() {
	gold_window->Update();
	equipstatus_window->Update();
	equip_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		Scene::Pop();
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::RIGHT)) {
		Game_System::SePlay(Main_Data::game_data.system.cursor_se);
		actor_index = (actor_index + 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Status>(actor_index), true);
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::LEFT)) {
		Game_System::SePlay(Main_Data::game_data.system.cursor_se);
		actor_index = (actor_index + Main_Data::game_party->GetActors().size() - 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Status>(actor_index), true);
	}
}
