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
#include "scene_skill.h"
#include "algo.h"
#include "game_map.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_targets.h"
#include "input.h"
#include "scene_actortarget.h"
#include "scene_teleport.h"
#include "transition.h"

Scene_Skill::Scene_Skill(int actor_index, int skill_index) :
	actor_index(actor_index), skill_index(skill_index) {
	Scene::type = Scene::Skill;
}

void Scene_Skill::Start() {
	// Create the windows
	int window_help_height = 32;
	int window_skillstatus_height = 32;
	help_window = std::make_unique<Window_Help>(this, Player::menu_offset_x, Player::menu_offset_y, MENU_WIDTH, window_help_height);
	skillstatus_window = std::make_unique<Window_SkillStatus>(this, Player::menu_offset_x, Player::menu_offset_y + window_help_height, MENU_WIDTH, window_skillstatus_height);
	skill_window = std::make_unique<Window_Skill>(this, Player::menu_offset_x, Player::menu_offset_y + window_help_height + window_skillstatus_height, MENU_WIDTH, MENU_HEIGHT - (window_help_height + window_skillstatus_height));

	// Assign actors and help to windows
	skill_window->SetActor(Main_Data::game_party->GetActors()[actor_index]->GetId());
	skillstatus_window->SetActor(Main_Data::game_party->GetActors()[actor_index]->GetId());
	skill_window->SetIndex(skill_index);
	skill_window->SetHelpWindow(help_window.get());
}

void Scene_Skill::Continue(SceneType) {
	skillstatus_window->Refresh();
	skill_window->Refresh();
}

void Scene_Skill::vUpdate() {
	help_window->Update();
	skillstatus_window->Update();
	skill_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		const lcf::rpg::Skill* skill = skill_window->GetSkill();
		int skill_id = skill ? skill->ID : 0;

		Game_Actor* actor = Main_Data::game_party->GetActors()[actor_index];

		if (skill && skill_window->CheckEnable(skill_id)) {
			if (skill->type == lcf::rpg::Skill::Type_switch) {
				Main_Data::game_system->SePlay(skill->sound_effect);
				Main_Data::game_party->UseSkill(skill_id, actor, actor);
				Scene::PopUntil(Scene::Map);
				Game_Map::SetNeedRefresh(true);
			} else if (Algo::IsNormalOrSubskill(*skill)) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Scene::Push(std::make_shared<Scene_ActorTarget>(skill_id, actor_index));
				skill_index = skill_window->GetIndex();
			} else if (skill->type == lcf::rpg::Skill::Type_teleport) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
				Scene::Push(std::make_shared<Scene_Teleport>(*actor, *skill));
			} else if (skill->type == lcf::rpg::Skill::Type_escape) {
				Main_Data::game_system->SePlay(skill->sound_effect);
				Main_Data::game_party->UseSkill(skill_id, actor, actor);
				Main_Data::game_player->ForceGetOffVehicle();
				Main_Data::game_player->ReserveTeleport(Main_Data::game_targets->GetEscapeTarget());

				Scene::PopUntil(Scene::Map);
			}
		} else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
	}
}

void Scene_Skill::TransitionOut(SceneType next_scene) {
	const auto* skill = skill_window->GetSkill();
	if (next_scene == Map && skill && skill->type == lcf::rpg::Skill::Type_escape) {
		Transition::instance().InitErase(Transition::TransitionFadeOut, this);
	} else {
		Scene::TransitionOut(next_scene);
	}
}
