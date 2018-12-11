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
#include "game_actors.h"
#include "game_party.h"
#include "game_system.h"
#include "input.h"
#include "main_data.h"
#include "scene_item.h"
#include "scene_skill.h"
#include "output.h"
#include "reader_util.h"

Scene_ActorTarget::Scene_ActorTarget(int item_id) :
	id(item_id), actor_index(0), use_item(true) {
	Scene::type = Scene::ActorTarget;
}

Scene_ActorTarget::Scene_ActorTarget(
	int skill_id, int actor_index) :
	id(skill_id), actor_index(actor_index), use_item(false) {
	Scene::type = Scene::ActorTarget;
}

void Scene_ActorTarget::Start() {
	// Create the windows
	help_window.reset(new Window_Help(0, 0, 136, 32));
	target_window.reset(new Window_ActorTarget(136, 0, 184, SCREEN_TARGET_HEIGHT));
	status_window.reset(new Window_TargetStatus(0, 32, 136, 32));

	target_window->SetActive(true);
	target_window->SetIndex(0);

	if (use_item) {
		const RPG::Item* item = ReaderUtil::GetElement(Data::items, id);
		if (!item) {
			Output::Warning("Scene ActorTarget: Invalid item ID %d", id);
			Scene::Pop();
			return;
		}
		const RPG::Skill* skill = nullptr;
		if (item->type == RPG::Item::Type_special) {
			skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("Scene ActorTarget: Item %d has invalid skill ID %d", id, item->skill_id);
				Scene::Pop();
				return;
			}
			if (skill->scope == RPG::Skill::Scope_party) {
				target_window->SetIndex(-100);
			}
		} else {
			if (item->entire_party) {
				target_window->SetIndex(-100);
			}
		}
		status_window->SetData(id, true, 0);
		help_window->SetText(item->name);
		return;
	} else {
		const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, id);
		if (!skill) {
			Output::Warning("Scene ActorTarget: Invalid skill ID %d", id);
			Scene::Pop();
			return;
		}

		if (skill->scope == RPG::Skill::Scope_self) {
			target_window->SetIndex(-actor_index);
		} else if (skill->scope == RPG::Skill::Scope_party) {
			target_window->SetIndex(-100);
		}

		status_window->SetData(id, false, actor_index);
		help_window->SetText(skill->name);
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

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	}
}

void Scene_ActorTarget::UpdateItem() {
	if (Input::IsTriggered(Input::DECISION)) {
		if (Main_Data::game_party->GetItemCount(id) <= 0) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			return;
		}
		if (Main_Data::game_party->UseItem(id, target_window->GetActor())) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_UseItem));
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}

		status_window->Refresh();
		target_window->Refresh();
	}
}

void Scene_ActorTarget::UpdateSkill() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_Actor* actor = &(*Main_Data::game_party)[actor_index];

		if (actor->GetSp() < actor->CalculateSkillCost(id)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			return;
		}
		if (Main_Data::game_party->UseSkill(id, actor, target_window->GetActor())) {
			RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, id);
			RPG::Animation* animation = ReaderUtil::GetElement(Data::animations, skill->animation_id);
			if (animation) {
				Game_System::SePlay(*animation);
			}
			else {
				Output::Warning("UpdateSkill: Skill %d references invalid animation %d", id, skill->animation_id);
			}
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}

		status_window->Refresh();
		target_window->Refresh();
	}
}
