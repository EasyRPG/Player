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
#include "cache.h"
#include "player.h"
#include "baseui.h"
#include "output.h"

constexpr int option_window_num_items = 10;

Scene_Settings::Scene_Settings() {
	Scene::type = Scene::Settings;
}

void Scene_Settings::CreateTitleGraphic() {
	// Load Title Graphic
	if (lcf::Data::system.title_name.empty()) {
		return;
	}
	title = std::make_unique<Sprite>();
	FileRequestAsync* request = AsyncHandler::RequestFile("Title", lcf::Data::system.title_name);
	request->SetGraphicFile(true);
	request_id = request->Bind(&Scene_Settings::OnTitleSpriteReady, this);
	request->Start();
}

void Scene_Settings::CreateMainWindow() {
	std::vector<std::string> options = {
		"Input",
		"Video",
		"Audio",
	};
	main_window = std::make_unique<Window_Command>(std::move(options), 96);
	main_window->SetHeight(176);
	main_window->SetY(32);
	main_window->SetX((SCREEN_TARGET_WIDTH - main_window->GetWidth()) / 2);
}

void Scene_Settings::CreateOptionsWindow() {
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	options_window = std::make_unique<Window_Settings>(32, 32, SCREEN_TARGET_WIDTH - 64, 176);
	options_window->SetHelpWindow(help_window.get());
}

void Scene_Settings::Start() {
	CreateTitleGraphic();
	CreateMainWindow();
	CreateOptionsWindow();

	SetMode(eMain);
}

void Scene_Settings::SetMode(Mode mode) {
	main_window->SetActive(false);
	main_window->SetVisible(false);
	options_window->SetActive(false);
	options_window->SetVisible(false);
	help_window->SetVisible(false);

	this->mode = mode;

	switch (mode) {
		case eMain:
			main_window->SetActive(true);
			main_window->SetVisible(true);
			break;
		case eInput:
			help_window->SetVisible(true);
			options_window->SetActive(true);
			options_window->SetVisible(true);
			options_window->SetMode(Window_Settings::eInput);
			break;
		case eAudio:
			help_window->SetVisible(true);
			options_window->SetActive(true);
			options_window->SetVisible(true);
			options_window->SetMode(Window_Settings::eAudio);
			break;
		case eVideo:
			help_window->SetVisible(true);
			options_window->SetActive(true);
			options_window->SetVisible(true);
			options_window->SetMode(Window_Settings::eVideo);
			break;
	}
}

void Scene_Settings::Update() {
	main_window->Update();
	help_window->Update();
	options_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		switch (mode) {
			case eMain:
				Scene::Pop();
				break;
			case eInput:
			case eAudio:
			case eVideo:
				SetMode(eMain);
				break;
		}

	}

	switch (mode) {
		case eMain:
			UpdateMain();
			break;
		case eInput:
		case eVideo:
		case eAudio:
			UpdateOptions();
			break;
	}
}


void Scene_Settings::OnTitleSpriteReady(FileRequestResult* result) {
	title->SetBitmap(Cache::Title(result->file));
}

void Scene_Settings::UpdateMain() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		auto idx = main_window->GetIndex();
		SetMode(static_cast<Mode>(idx + 1));
	}
}

void Scene_Settings::UpdateOptions() {
	if (Input::IsTriggered(Input::DECISION)) {
		if (options_window->IsCurrentActionEnabled()) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			options_window->DoCurrentAction();
			options_window->Refresh();
		}
	}
}

void Scene_Settings::DrawBackground(Bitmap& dst) {
	if (!title || !title->GetBitmap()) {
		Scene::DrawBackground(dst);
	}
}

