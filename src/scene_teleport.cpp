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
#include "scene_teleport.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "input.h"

Scene_Teleport::Scene_Teleport(Game_Actor& actor, const RPG::Skill& skill)
		: actor(&actor), skill(&skill) {
	type = Scene::Teleport;
}

Scene_Teleport::Scene_Teleport(const RPG::Item& item)
		: item(&item) {
	type = Scene::Teleport;
}

void Scene_Teleport::Start() {
	teleport_window.reset(new Window_Teleport(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));
	teleport_window->SetActive(true);
	teleport_window->SetIndex(0);
}

void Scene_Teleport::Update() {
	teleport_window->Update();

	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_UseItem));

		if (skill) {
			Main_Data::game_party->UseSkill(skill->ID, actor, actor);
		} else if (item) {
			Main_Data::game_party->ConsumeItemUse(item->ID);
		}

		const RPG::SaveTarget& target = teleport_window->GetTarget();

		Main_Data::game_player->ReserveTeleport(target);
		Main_Data::game_player->StartTeleport();

		Scene::PopUntil(Scene::Map);
	} else if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));

		Scene::Pop();
	}
}
