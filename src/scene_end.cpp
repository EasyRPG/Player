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
#include "audio.h"
#include "game_system.h"
#include "input.h"
#include "scene_end.h"
#include "scene_menu.h"
#include "scene_title.h"
#include "util_macro.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
Scene_End::Scene_End() :
	help_window(NULL),
	command_window(NULL) {
	Scene::type = Scene::End;
}

////////////////////////////////////////////////////////////
void Scene_End::Start() {
	CreateCommandWindow();
	CreateHelpWindow();
}

////////////////////////////////////////////////////////////
void Scene_End::Terminate() {
	delete command_window;
	delete help_window;
}

////////////////////////////////////////////////////////////
void Scene_End::Update() {
	command_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::Pop(); // Select End Game
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		switch (command_window->GetIndex()) {
		case 0: // Yes
			Audio::BGM_Fade(800);
			Audio::BGS_Fade(800);
			Audio::ME_Fade(800);
			Scene::PopUntil(Scene::Title);
			break;
		case 1: // No
			Scene::Pop();
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

	command_window = new Window_Command(options);
	command_window->SetX(160 - command_window->GetWidth() / 2);
	command_window->SetY(72 + 48);
}

////////////////////////////////////////////////////////////
void Scene_End::CreateHelpWindow() {
	int text_size = Font::Default()->GetSize(Data::terms.exit_game_message).width;

	help_window = new Window_Help(160 - (text_size + 16)/ 2,
		72, text_size + 16, 32);
	help_window->SetText(Data::terms.exit_game_message);

	command_window->SetHelpWindow(help_window);
}
