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
#include "transition.h"
#include <player.h>

Scene_Teleport::Scene_Teleport(Game_Actor& actor, const lcf::rpg::Skill& skill)
		: actor(&actor), skill(&skill) {
	type = Scene::Teleport;
}

Scene_Teleport::Scene_Teleport(const lcf::rpg::Item& item, const lcf::rpg::Skill& skill)
		: skill(&skill), item(&item) {
	type = Scene::Teleport;
	assert(item.skill_id == skill.ID && "Item doesn't invoke the skill");
}

void Scene_Teleport::Start() {
	teleport_window.reset(new Window_Teleport(0, Player::screen_height - 80, Player::screen_width, 80));
	teleport_window->SetActive(true);
	teleport_window->SetIndex(0);
}

void Scene_Teleport::vUpdate() {
	teleport_window->Update();

	if (Input::IsTriggered(Input::DECISION)) {
		if (item) {
			Main_Data::game_party->ConsumeItemUse(item->ID);
		} else {
			Main_Data::game_party->UseSkill(skill->ID, actor, actor);
		}

		Main_Data::game_system->SePlay(skill->sound_effect);

		const lcf::rpg::SaveTarget& target = teleport_window->GetTarget();

		Main_Data::game_player->ForceGetOffVehicle();
		Main_Data::game_player->ReserveTeleport(target);

		Scene::PopUntil(Scene::Map);
	} else if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));

		Scene::Pop();
	}
}

void Scene_Teleport::TransitionOut(SceneType next_scene) {
	if (next_scene == Map) {
		Transition::instance().InitErase(Transition::TransitionFadeOut, this);
	} else {
		Scene::TransitionOut(next_scene);
	}
}
