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
#include <lcf/reader_util.h>

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
		const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, id);
		if (!item) {
			Output::Warning("Scene ActorTarget: Invalid item ID {}", id);
			Scene::Pop();
			return;
		}
		const lcf::rpg::Skill* skill = nullptr;
		if (item->type == lcf::rpg::Item::Type_special) {
			skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("Scene ActorTarget: Item {} has invalid skill ID {}", id, item->skill_id);
				Scene::Pop();
				return;
			}
			if (skill->scope == lcf::rpg::Skill::Scope_party) {
				target_window->SetIndex(-100);
			}
		} else {
			if (item->entire_party) {
				target_window->SetIndex(-100);
			}
		}
		status_window->SetData(id, true, 0);
		help_window->SetText(ToString(item->name), Font::ColorDefault, Text::AlignLeft, false);
		return;
	} else {
		const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, id);
		if (!skill) {
			Output::Warning("Scene ActorTarget: Invalid skill ID {}", id);
			Scene::Pop();
			return;
		}

		if (skill->scope == lcf::rpg::Skill::Scope_self) {
			target_window->SetIndex(-actor_index - 1);
		} else if (skill->scope == lcf::rpg::Skill::Scope_party) {
			target_window->SetIndex(-100);
		}

		status_window->SetData(id, false, actor_index);
		help_window->SetText(ToString(skill->name), Font::ColorDefault, Text::AlignLeft, false);
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
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	}
}

void Scene_ActorTarget::UpdateItem() {
	if (Input::IsTriggered(Input::DECISION)) {
		if (Main_Data::game_party->GetItemCount(id) <= 0) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			return;
		}
		if (Main_Data::game_party->UseItem(id, target_window->GetActor())) {
			auto* item = lcf::ReaderUtil::GetElement(lcf::Data::items, id);
			assert(item);

			bool do_skill = (item->type == lcf::rpg::Item::Type_special)
				|| (item->use_skill && (
							item->type == lcf::rpg::Item::Type_weapon
							|| item->type == lcf::rpg::Item::Type_shield
							|| item->type == lcf::rpg::Item::Type_armor
							|| item->type == lcf::rpg::Item::Type_helmet
							|| item->type == lcf::rpg::Item::Type_accessory
							)
				   );

			if (do_skill) {
				auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
				assert(skill);
				auto* animation = lcf::ReaderUtil::GetElement(lcf::Data::animations, skill->animation_id);
				if (animation) {
					Main_Data::game_system->SePlay(*animation);
				}
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_UseItem));
			}
		}
		else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}

		status_window->Refresh();
		target_window->Refresh();
	}
}

void Scene_ActorTarget::UpdateSkill() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_Actor* actor = &(*Main_Data::game_party)[actor_index];

		if (actor->GetSp() < actor->CalculateSkillCost(id) || actor->GetHp() <= actor->CalculateSkillHpCost(id)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			return;
		}
		if (Main_Data::game_party->UseSkill(id, actor, target_window->GetActor())) {
			lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, id);
			lcf::rpg::Animation* animation = lcf::ReaderUtil::GetElement(lcf::Data::animations, skill->animation_id);
			if (animation) {
				Main_Data::game_system->SePlay(*animation);
			}
			else {
				Output::Warning("UpdateSkill: Skill {} references invalid animation {}", id, skill->animation_id);
			}
		}
		else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}

		status_window->Refresh();
		target_window->Refresh();
	}
}
