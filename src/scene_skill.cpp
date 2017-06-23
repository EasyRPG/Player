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
#include "game_map.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_targets.h"
#include "input.h"
#include "scene_actortarget.h"
#include "scene_teleport.h"

Scene_Skill::Scene_Skill(int actor_index, int skill_index) :
	actor_index(actor_index), skill_index(skill_index) {
	Scene::type = Scene::Skill;
}

void Scene_Skill::Start() {
	// Create the windows
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	skillstatus_window.reset(new Window_SkillStatus(0, 32, SCREEN_TARGET_WIDTH, 32));
	skill_window.reset(new Window_Skill(0, 64, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT - 64));

	// Assign actors and help to windows
	skill_window->SetActor(Main_Data::game_party->GetActors()[actor_index]->GetId());
	skillstatus_window->SetActor(Main_Data::game_party->GetActors()[actor_index]->GetId());
	skill_window->SetIndex(skill_index);
	skill_window->SetHelpWindow(help_window.get());
}

void Scene_Skill::Update() {
	help_window->Update();
	skillstatus_window->Update();
	skill_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		const RPG::Skill* skill = skill_window->GetSkill();
		int skill_id = skill ? skill->ID : 0;

		Game_Actor* actor = Main_Data::game_party->GetActors()[actor_index];

		if (skill && skill_window->CheckEnable(skill_id)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

			if (skill->type == RPG::Skill::Type_switch) {
				Main_Data::game_party->UseSkill(skill_id, actor, actor);
				Scene::PopUntil(Scene::Map);
				Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
			} else if (skill->type == RPG::Skill::Type_normal || skill->type >= RPG::Skill::Type_subskill) {
				Scene::Push(std::make_shared<Scene_ActorTarget>(skill_id, actor_index));
				skill_index = skill_window->GetIndex();
			} else if (skill->type == RPG::Skill::Type_teleport) {
				Scene::Push(std::make_shared<Scene_Teleport>(*actor, *skill));
			} else if (skill->type == RPG::Skill::Type_escape) {
				Main_Data::game_party->UseSkill(skill_id, actor, actor);

				Main_Data::game_player->ReserveTeleport(*Game_Targets::GetEscapeTarget());
				Main_Data::game_player->StartTeleport();

				Scene::PopUntil(Scene::Map);
			}
		} else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	}
}
