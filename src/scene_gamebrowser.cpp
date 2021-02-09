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
#include "scene_gamebrowser.h"
#include "audio_secache.h"
#include "cache.h"
#include "game_system.h"
#include "input.h"
#include "player.h"
#include "scene_title.h"
#include "bitmap.h"
#include "audio.h"
#include "output.h"

#ifdef _WIN32
	#include <windows.h>
#endif

Scene_GameBrowser::Scene_GameBrowser() {
	type = Scene::GameBrowser;
}

void Scene_GameBrowser::Start() {
	initial_debug_flag = Player::debug_flag;
	Main_Data::game_system = std::make_unique<Game_System>();
	Main_Data::game_system->SetSystemGraphic(CACHE_DEFAULT_BITMAP, lcf::rpg::System::Stretch_stretch, lcf::rpg::System::Font_gothic);
	filesystems.push_back(FileFinder::Game());
	CreateWindows();
	Game_Clock::ResetFrame(Game_Clock::now());
}

void Scene_GameBrowser::Continue(SceneType /* prev_scene */) {
	Main_Data::game_system->BgmStop();

	Cache::Clear();
	AudioSeCache::Clear();
	lcf::Data::Clear();
	Main_Data::Cleanup();
	FileFinder::Quit();

	Player::game_title = "";
	Player::engine = Player::EngineNone;

	Main_Data::game_system = std::make_unique<Game_System>();
	Main_Data::game_system->SetSystemGraphic(CACHE_DEFAULT_BITMAP, lcf::rpg::System::Stretch_stretch, lcf::rpg::System::Font_gothic);

	Player::debug_flag = initial_debug_flag;
}

void Scene_GameBrowser::Update() {
	if (game_loading) {
		BootGame();
		return;
	}

	command_window->Update();
	gamelist_window->Update();

	if (command_window->GetActive()) {
		UpdateCommand();
	}
	else if (gamelist_window->GetActive()) {
		UpdateGameListSelection();
	}
}

void Scene_GameBrowser::CreateWindows() {
	// Create Options Window
	std::vector<std::string> options;

	options.push_back("Games");
	options.push_back("About");
	options.push_back("Exit");

	command_window.reset(new Window_Command(options, 60));
	command_window->SetY(32);
	command_window->SetIndex(0);

	gamelist_window.reset(new Window_GameList(60, 32, SCREEN_TARGET_WIDTH - 60, SCREEN_TARGET_HEIGHT - 32));
	gamelist_window->Refresh(filesystems.back(), false);

	if (filesystems.size() == 1 && !gamelist_window->HasValidEntry()) {
		command_window->DisableItem(0);
	}

	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	help_window->SetText("EasyRPG Player - RPG Maker 2000/2003 interpreter");

	load_window.reset(new Window_Help(SCREEN_TARGET_WIDTH / 4, SCREEN_TARGET_HEIGHT / 2 - 16, SCREEN_TARGET_WIDTH / 2, 32));
	load_window->SetText("Loading...");
	load_window->SetVisible(false);

	about_window.reset(new Window_About(60, 32, SCREEN_TARGET_WIDTH - 60, SCREEN_TARGET_HEIGHT - 32));
	about_window->Refresh();
	about_window->SetVisible(false);
}

void Scene_GameBrowser::UpdateCommand() {
	int menu_index = command_window->GetIndex();

	switch (menu_index) {
		case GameList:
			gamelist_window->SetVisible(true);
			about_window->SetVisible(false);
			break;
		case About:
			gamelist_window->SetVisible(false);
			about_window->SetVisible(true);
			break;
		default:
			break;
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {

		switch (menu_index) {
			case GameList:
				if (filesystems.size() == 1 && !gamelist_window->HasValidEntry()) {
					return;
				}
				command_window->SetActive(false);
				command_window->SetIndex(-1);
				gamelist_window->SetActive(true);
				gamelist_window->SetIndex(old_gamelist_index);
				break;
			case About:
				break;
			default:
				Scene::Pop();
		}
	}
}

void Scene_GameBrowser::UpdateGameListSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		command_window->SetActive(true);
		command_window->SetIndex(0);
		gamelist_window->SetActive(false);
		old_gamelist_index = gamelist_window->GetIndex();
		gamelist_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		load_window->SetVisible(true);
		game_loading = true;
	} else if (Input::IsTriggered(Input::DEBUG_MENU) || Input::IsTriggered(Input::SHIFT)) {
		Player::debug_flag = true;
		load_window->SetVisible(true);
		game_loading = true;
	}
}

void Scene_GameBrowser::BootGame() {
	if (filesystems.size() > 1 && gamelist_window->GetIndex() == 0) {
		// ".." -> Go one level up
		filesystems.pop_back();
		gamelist_window->Refresh(filesystems.back(), filesystems.size() > 1);
		gamelist_window->SetIndex(0);
		load_window->SetVisible(false);
		game_loading = false;
		return;
	}

	FilesystemView fs = gamelist_window->GetGameFilesystem();

	if (!fs) {
		Output::Warning("The selected file or directory cannot be opened");
		load_window->SetVisible(false);
		game_loading = false;
		return;
	}

	if (!FileFinder::IsValidProject(fs)) {
		// Not a game: Open as directory
		load_window->SetVisible(false);
		game_loading = false;
		if (!gamelist_window->Refresh(fs, true)) {
			Output::Warning("The selected file or directory cannot be opened");
			return;
		}
		filesystems.push_back(fs);
		gamelist_window->SetIndex(0);

		return;
	}

	FileFinder::SetGameFilesystem(fs);

	std::string startup_path = "";
	for (auto f : filesystems) {
		startup_path = FileFinder::MakePath(startup_path, f.GetPath());
	}
	Main_Data::SetProjectPath(FileFinder::MakePath(startup_path, fs.GetPath()));

	Player::CreateGameObjects();

	Scene::Push(std::make_shared<Scene_Title>());

	game_loading = false;
	load_window->SetVisible(false);
}
