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
#include "scene_settings.h"
#include "input.h"
#include "game_system.h"

Scene_Settings::Scene_Settings() {
	Scene::type = Scene::Settings;
}

void Scene_Settings::Start() {
	CreateMainWindow();

	main_window->SetActive(true);
}

void Scene_Settings::CreateMainWindow() {
	std::vector<std::string> options = {
		"Input",
		"Video",
		"Audio",
		"Exit"
	};
	main_window = std::make_unique<Window_Command>(std::move(options), 96);
	main_window->SetHeight(176);
	main_window->SetY(32);
	main_window->SetX((SCREEN_TARGET_WIDTH - main_window->GetWidth()) / 2);
}

void Scene_Settings::Update() {
	main_window->Update();

	switch (mode) {
		case eMain:
			UpdateMain();
			break;
		case eInput:
			UpdateInput();
			break;
		case eVideo:
			UpdateVideo();
			break;
		case eAudio:
			UpdateAudio();
			break;
	}
}

void Scene_Settings::UpdateMain() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	}
}

void Scene_Settings::UpdateInput() {
}

void Scene_Settings::UpdateVideo() {
}

void Scene_Settings::UpdateAudio() {
}

