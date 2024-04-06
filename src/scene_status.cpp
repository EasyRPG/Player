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
#include <player.h>

Scene_Status::Scene_Status(int actor_index) :
	actor_index(actor_index) {
	type = Scene::Status;
}

void Scene_Status::Start() {
	int window_actor_info_width = 124;
	int window_actor_info_height = 208;
	int window_gold_width = 124;
	int window_gold_height = 32;
	int window_actor_status_width = 196;
	int window_actor_status_height = 64;
	int window_param_status_width = 196;
	int window_param_status_height = 80;
	int window_equip_width = 196;
	int window_equip_height = 96;

	int actor = Main_Data::game_party->GetActors()[actor_index]->GetId();

	actorinfo_window = std::make_unique<Window_ActorInfo>(this, Player::menu_offset_x, Player::menu_offset_y, window_actor_info_width, window_actor_info_height, actor);
	gold_window = std::make_unique<Window_Gold>(this, Player::menu_offset_x, Player::menu_offset_y + window_actor_info_height, window_gold_width, window_gold_height);
	actorstatus_window = std::make_unique<Window_ActorStatus>(this, Player::menu_offset_x + window_actor_info_width, Player::menu_offset_y, window_actor_status_width, window_actor_status_height, actor);
	paramstatus_window = std::make_unique<Window_ParamStatus>(this, Player::menu_offset_x + window_actor_info_width, Player::menu_offset_y + window_actor_status_height, window_param_status_width, window_param_status_height, actor);
	equip_window = std::make_unique<Window_Equip>(this, Player::menu_offset_x + window_actor_info_width, Player::menu_offset_y + window_actor_status_height + window_param_status_height, window_equip_width, window_equip_height, actor);

	equip_window->SetActive(false);
	paramstatus_window->SetActive(false);

	equip_window->SetIndex(-1);
}

void Scene_Status::vUpdate() {
	gold_window->Update();
	paramstatus_window->Update();
	equip_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::RIGHT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		actor_index = (actor_index + 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(std::make_shared<Scene_Status>(actor_index), true);
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::LEFT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		actor_index = (actor_index + Main_Data::game_party->GetActors().size() - 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(std::make_shared<Scene_Status>(actor_index), true);
	}
}
