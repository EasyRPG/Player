/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene_skill.h"
#include "game_system.h"
#include "input.h"
#include "scene_menu.h"

////////////////////////////////////////////////////////////
Scene_Skill::Scene_Skill(int actor_index) :
	actor_index(actor_index) {
	Scene::type = Scene::Skill;
}

////////////////////////////////////////////////////////////
Scene_Skill::~Scene_Skill() {
	delete help_window;
	delete skillstatus_window;
	delete skill_window;
}

////////////////////////////////////////////////////////////
void Scene_Skill::Start() {
	// Create the windows
	help_window = new Window_Help(0, 0, 320, 32);
	skillstatus_window = new Window_SkillStatus(0, 32, 320, 32);
	skill_window = new Window_Skill(0, 64, 320, 240 - 64);

	skill_window->SetHelpWindow(help_window);
	help_window->SetText("This scene is work in progress :)");
}

////////////////////////////////////////////////////////////
void Scene_Skill::Update() {
	help_window->Update();
	skillstatus_window->Update();
	skill_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::instance = new Scene_Menu(1); // Select Skill
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
	}
}
