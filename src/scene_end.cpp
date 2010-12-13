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
#include <vector>
#include "game_system.h"
#include "input.h"
#include "scene_end.h"
#include "scene_menu.h"
#include "scene_title.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
Scene_End::Scene_End() :
	command_window(NULL), help_window(NULL) {
	Scene::type = Scene::Title;
}

////////////////////////////////////////////////////////////
Scene_End::~Scene_End() {
	delete command_window;
	delete help_window;
}

////////////////////////////////////////////////////////////
void Scene_End::Start() {
	CreateCommandWindow();
	CreateHelpWindow();
}

////////////////////////////////////////////////////////////
void Scene_End::Update() {
	command_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::instance = new Scene_Menu(4); // Select End Game
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		switch (command_window->GetIndex()) {
		case 0: // Yes
			type = Scene::Null;
			instance = NULL;
			break;
		case 1: // No
			Scene::instance = new Scene_Menu(4);
			break;
		}
	}
}

////////////////////////////////////////////////////////////
void Scene_End::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Data::terms.yes);
	options.push_back(Data::terms.no);

	// TODO: Calculate window width from max text length from options
	int text_size = max(Data::terms.yes.size() * 6, Data::terms.no.size() * 6);
	command_window = new Window_Command(text_size + 24, options);
	command_window->SetX(160 - command_window->GetWidth() / 2);
	command_window->SetY(72 + 48);
}

////////////////////////////////////////////////////////////
void Scene_End::CreateHelpWindow() {
	int text_size = Data::terms.exit_game_message.size() * 6;

	help_window = new Window_Help(160 - (text_size + 16)/ 2,
		72, text_size + 16, 32);
	help_window->SetText(Data::terms.exit_game_message);

	command_window->SetHelpWindow(help_window);
}
