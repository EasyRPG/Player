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
#include "scene_actortarget.h"
#include "scene_map.h"
#include "scene_teleport.h"

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

void Scene_Item::Continue() {
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
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

			const RPG::Item& item = *item_window->GetItem();

			if (item.type == RPG::Item::Type_switch) {
				Main_Data::game_party->ConsumeItemUse(item_id);
				Game_Switches[Data::items[item_id - 1].switch_id] = true;
				Scene::PopUntil(Scene::Map);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
			} else if (item.type == RPG::Item::Type_special && item.skill_id > 0) {
				if (Data::skills[item.skill_id - 1].type == RPG::Skill::Type_teleport) {
					Scene::Push(std::make_shared<Scene_Teleport>(item));
				} else if (Data::skills[item.skill_id - 1].type == RPG::Skill::Type_escape) {
					Main_Data::game_party->ConsumeItemUse(item_id);

					Main_Data::game_player->ReserveTeleport(*Game_Targets::GetEscapeTarget());
					Main_Data::game_player->StartTeleport();

					Scene::PopUntil(Scene::Map);
				} else if (Data::skills[item.skill_id - 1].type == RPG::Skill::Type_switch) {
					Main_Data::game_party->ConsumeItemUse(item_id);
					Game_Switches[Data::items[item_id - 1].switch_id] = true;
					Scene::PopUntil(Scene::Map);
					Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
				} else {
					Scene::Push(std::make_shared<Scene_ActorTarget>(item_id));
					item_index = item_window->GetIndex();
				}
			} else {
				Scene::Push(std::make_shared<Scene_ActorTarget>(item_id));
				item_index = item_window->GetIndex();
			}
		} else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	}
}
