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
#include <algorithm>
#include <string>
#include <vector>
#include "scene_order.h"
#include "game_actor.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "input.h"
#include "scene_map.h"

Scene_Order::Scene_Order() :
	actor_counter(0) {
	type = Scene::Order;
}

void Scene_Order::Start() {
	actors.resize(Main_Data::game_party->GetActors().size());

	CreateCommandWindow();
}

void Scene_Order::vUpdate() {
	window_left->Update();
	window_right->Update();
	window_confirm->Update();

	if (window_left->GetActive()) {
		UpdateOrder();
	} else if (window_confirm->GetActive()) {
		UpdateConfirm();
	}
}

void Scene_Order::UpdateOrder() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		if (actor_counter == 0) {
			Scene::Pop();
		} else {
			--actor_counter;
			window_left->SetItemText(actors[actor_counter] - 1, Main_Data::game_party->GetActors()[actors[actor_counter] - 1]->GetName());
			window_right->SetItemText(actor_counter, "");
			actors[actor_counter] = 0;
		}
	} else if (Input::IsTriggered(Input::DECISION) && window_left->GetIndex() < Main_Data::game_party->GetActors().size()) {
		if (std::find(actors.begin(), actors.end(), window_left->GetIndex() + 1) != actors.end()) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		} else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			window_left->SetItemText(window_left->GetIndex(), "");
			window_right->SetItemText(actor_counter, Main_Data::game_party->GetActors()[window_left->GetIndex()]->GetName());

			actors[actor_counter] = window_left->GetIndex() + 1;

			++actor_counter;

			// Display Confirm/Redo window
			if (actor_counter == (int)Main_Data::game_party->GetActors().size()) {
				window_left->SetIndex(-1);
				window_left->SetActive(false);
				window_confirm->SetIndex(0);
				window_confirm->SetActive(true);
				window_confirm->SetVisible(true);
			}
		}
	}
}

void Scene_Order::UpdateConfirm() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Redo();
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (window_confirm->GetIndex() == 0) {
			Confirm();
			Scene::Pop();
		} else if(window_confirm->GetIndex() == 1) {
			Redo();
		}
	}
}

void Scene_Order::CreateCommandWindow() {
	std::vector<std::string> options_left;
	std::vector<std::string> options_right;
	std::vector<std::string> options_confirm;

	const auto actors = Main_Data::game_party->GetActors();
	for (auto& actor: actors) {
		options_left.push_back(ToString(actor->GetName()));
		options_right.push_back("");
	}

	options_confirm.push_back(lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_order_scene_confirm, "Confirm"));
	options_confirm.push_back(lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_order_scene_redo, "Redo"));

	window_left = std::make_unique<Window_Command>(this, options_left, 88, 4);
	window_left->SetX(Player::menu_offset_x + 68);
	window_left->SetY(Player::menu_offset_y + 48);

	window_right = std::make_unique<Window_Command>(this, options_right, 88, 4);
	window_right->SetX(Player::menu_offset_x + 164);
	window_right->SetY(Player::menu_offset_y + 48);
	window_right->SetActive(false);
	window_right->SetIndex(-1);

	window_confirm = std::make_unique<Window_Command>(this, options_confirm, 80);
	window_confirm->SetX(Player::menu_offset_x + 120);
	window_confirm->SetY(Player::menu_offset_y + 144);
	window_confirm->SetActive(false);
	window_confirm->SetVisible(false);
}

void Scene_Order::Redo() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));

	actors.clear();
	actors.resize(Main_Data::game_party->GetActors().size());

	std::vector<Game_Actor*> actors = Main_Data::game_party->GetActors();
	for (std::vector<Game_Actor*>::const_iterator it = actors.begin();
		it != actors.end(); ++it) {
		int index = it - actors.begin();
		window_left->SetItemText(index, (*it)->GetName());
		window_right->SetItemText(index, "");
	}

	window_left->SetActive(true);
	window_left->SetIndex(0);

	window_confirm->SetActive(false);
	window_confirm->SetVisible(false);
	window_confirm->SetIndex(-1);

	actor_counter = 0;
	actors.clear();
	actors.resize(actors.size());
}

void Scene_Order::Confirm() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	std::vector<Game_Actor*> party_actors = Main_Data::game_party->GetActors();

	std::vector<int>::const_iterator it;

	for (it = actors.begin(); it != actors.end(); ++it) {
		Main_Data::game_party->RemoveActor(party_actors[*it - 1]->GetId());
	}

	for (it = actors.begin(); it != actors.end(); ++it) {
		Main_Data::game_party->AddActor(party_actors[*it - 1]->GetId());
	}

	// TODO: Where is the best place to overwrite the character map graphic?
}
