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
#include "reader_util.h"
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
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	item_window.reset(new Window_Item(0, 32, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT - 32));
	item_window->SetHelpWindow(help_window.get());
	item_window->Refresh();
	item_window->SetIndex(item_index);
}

void Scene_Item::Continue(SceneType /* prev_scene */) {
	item_window->Refresh();
}

void Scene_Item::Update() {
	help_window->Update();
	item_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		int item_id = item_window->GetItem() == NULL ? 0 : item_window->GetItem()->ID;

		if (item_id > 0 && item_window->CheckEnable(item_id)) {
			// The party only has valid items
			const RPG::Item& item = *item_window->GetItem();

			if (item.type == RPG::Item::Type_switch) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				Main_Data::game_party->ConsumeItemUse(item_id);
				Main_Data::game_switches->Set(item.switch_id, true);
				Scene::PopUntil(Scene::Map);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
			} else if (item.type == RPG::Item::Type_special && item.skill_id > 0) {
				const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, item.skill_id);
				if (!skill) {
					Output::Warning("Scene Item: Item references invalid skill ID %d", item.skill_id);
					return;
				}

				if (skill->type == RPG::Skill::Type_teleport) {
					Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
					Scene::Push(std::make_shared<Scene_Teleport>(item, *skill));
				} else if (skill->type == RPG::Skill::Type_escape) {
					Main_Data::game_party->ConsumeItemUse(item_id);
					Game_System::SePlay(skill->sound_effect);

					Main_Data::game_player->ReserveTeleport(*Game_Targets::GetEscapeTarget());

					Scene::PopUntil(Scene::Map);
				} else if (skill->type == RPG::Skill::Type_switch) {
					Main_Data::game_party->ConsumeItemUse(item_id);
					Game_System::SePlay(skill->sound_effect);
					Main_Data::game_switches->Set(skill->switch_id, true);
					Scene::PopUntil(Scene::Map);
					Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
				} else {
					Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
					Scene::Push(std::make_shared<Scene_ActorTarget>(item_id));
					item_index = item_window->GetIndex();
				}
			} else {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				Scene::Push(std::make_shared<Scene_ActorTarget>(item_id));
				item_index = item_window->GetIndex();
			}
		} else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	}
}

void Scene_Item::TransitionOut(Scene::SceneType next_scene) {
	const auto* item = item_window->GetItem();
	const RPG::Skill* skill = nullptr;
	if (item && item->type == RPG::Item::Type_special && item->skill_id > 0) {
		skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
	}

	if (next_scene == Map && skill && skill->type == RPG::Skill::Type_escape) {
		Transition::instance().Init(Transition::TransitionFadeOut, this, 32, true);
	} else {
		Scene::TransitionOut(next_scene);
	}
}
