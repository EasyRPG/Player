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
#include "scene_item.h"
#include "game_map.h"
#include "game_party.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_targets.h"
#include "input.h"
#include <lcf/reader_util.h>
#include "scene_actortarget.h"
#include "scene_map.h"
#include "scene_teleport.h"
#include "output.h"
#include "transition.h"

Scene_Item::Scene_Item(int item_index) :
	item_index(item_index) {
	Scene::type = Scene::Item;
}

void Scene_Item::Start() {
	// Create the windows
	int menu_help_height = 32;
	help_window = std::make_unique<Window_Help>(this, Player::menu_offset_x, Player::menu_offset_y, MENU_WIDTH, menu_help_height);
	item_window = std::make_unique<Window_Item>(this, Player::menu_offset_x, Player::menu_offset_y + menu_help_height, MENU_WIDTH, MENU_HEIGHT - menu_help_height);
	item_window->SetHelpWindow(help_window.get());
	item_window->Refresh();
	item_window->SetIndex(item_index);
}

void Scene_Item::Continue(SceneType /* prev_scene */) {
	item_window->Refresh();
}

void Scene_Item::vUpdate() {
	help_window->Update();
	item_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		int item_id = item_window->GetItem() == NULL ? 0 : item_window->GetItem()->ID;

		if (item_id > 0 && item_window->CheckEnable(item_id)) {
			// The party only has valid items
			const lcf::rpg::Item& item = *item_window->GetItem();

			if (item.type == lcf::rpg::Item::Type_switch) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Main_Data::game_party->ConsumeItemUse(item_id);
				Main_Data::game_switches->Set(item.switch_id, true);
				Scene::PopUntil(Scene::Map);
				Game_Map::SetNeedRefresh(true);
			} else if (item.type == lcf::rpg::Item::Type_special && item.skill_id > 0) {
				const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item.skill_id);
				if (!skill) {
					Output::Warning("Scene Item: Item references invalid skill ID {}", item.skill_id);
					return;
				}

				if (skill->type == lcf::rpg::Skill::Type_teleport) {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
					Scene::Push(std::make_shared<Scene_Teleport>(item, *skill));
				} else if (skill->type == lcf::rpg::Skill::Type_escape) {
					Main_Data::game_party->ConsumeItemUse(item_id);
					Main_Data::game_system->SePlay(skill->sound_effect);

					Main_Data::game_player->ForceGetOffVehicle();
					Main_Data::game_player->ReserveTeleport(Main_Data::game_targets->GetEscapeTarget());

					Scene::PopUntil(Scene::Map);
				} else if (skill->type == lcf::rpg::Skill::Type_switch) {
					Main_Data::game_party->ConsumeItemUse(item_id);
					Main_Data::game_system->SePlay(skill->sound_effect);
					Main_Data::game_switches->Set(skill->switch_id, true);
					Scene::PopUntil(Scene::Map);
					Game_Map::SetNeedRefresh(true);
				} else {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
					Scene::Push(std::make_shared<Scene_ActorTarget>(item_id));
					item_index = item_window->GetIndex();
				}
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Scene::Push(std::make_shared<Scene_ActorTarget>(item_id));
				item_index = item_window->GetIndex();
			}
		} else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
	}
}

void Scene_Item::TransitionOut(Scene::SceneType next_scene) {
	const auto* item = item_window->GetItem();
	const lcf::rpg::Skill* skill = nullptr;
	if (item && item->type == lcf::rpg::Item::Type_special && item->skill_id > 0) {
		skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
	}

	if (next_scene == Map && skill && skill->type == lcf::rpg::Skill::Type_escape) {
		Transition::instance().InitErase(Transition::TransitionFadeOut, this);
	} else {
		Scene::TransitionOut(next_scene);
	}
}
