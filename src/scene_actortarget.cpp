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
#include "scene_actortarget.h"
#include "game_system.h"
#include "input.h"
#include "main_data.h"
#include "scene_item.h"
#include "scene_skill.h"

Scene_ActorTarget::Scene_ActorTarget(int item_id, int item_index) :
	id(item_id), index(item_index), use_item(true) {
	Scene::type = Scene::ActorTarget;
}

Scene_ActorTarget::Scene_ActorTarget(
	int skill_id, int actor_index, int skill_index) :
	id(skill_id), index(skill_index),
	actor_index(actor_index), use_item(false) {
	Scene::type = Scene::ActorTarget;
}

void Scene_ActorTarget::Start() {
	// Create the windows
	help_window.reset(new Window_Help(0, 0, 136, 32));
	target_window.reset(new Window_ActorTarget(136, 0, 184, 240));
	status_window.reset(new Window_TargetStatus(0, 32, 136, 32));

	target_window->SetActive(true);
	target_window->SetIndex(0);

	if (use_item) {
		if (Data::items[id - 1].entire_party) {
			target_window->SetIndex(-100);
		}
		status_window->SetData(id, true);
		help_window->SetText(Data::items[id - 1].name);
	} else {
		if (Data::skills[id - 1].scope == RPG::Skill::Scope_self) {
			target_window->SetIndex(-actor_index);
		} else if (Data::skills[id - 1].scope == RPG::Skill::Scope_party) {
			target_window->SetIndex(-100);
		}

		status_window->SetData(id, false);
		help_window->SetText(Data::skills[id - 1].name);
	}
}

void Scene_ActorTarget::Update() {
	help_window->Update();
	target_window->Update();
	status_window->Update();

	if (use_item) {
		UpdateItem();
	} else {
		UpdateSkill();
	}
}

void Scene_ActorTarget::UpdateItem() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		Scene::Pop();
	}
}

void Scene_ActorTarget::UpdateSkill() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		Scene::Pop();
	}
}
