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
#include "cache.h"
#include "graphics.h"
#include "game_system.h"
#include "input.h"
#include "player.h"
#include "scene_map.h"
#include "scene_skill.h"
#include "scene_title.h"
#include "bitmap.h"

Scene_GameBrowser::Scene_GameBrowser() {
	type = Scene::GameBrowser;
}

void Scene_GameBrowser::Start() {
	Game_System::SetSystemName(CACHE_DEFAULT_BITMAP);
	CreateWindows();

	/*if (Player::new_game_flag) {
		Player::SetupPlayerSpawn();
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Map>());
	}
	else if (Player::load_game_id > 0) {
	std::stringstream ss;
	ss << "Save" << (Player::load_game_id <= 9 ? "0" : "") << Player::load_game_id << ".lsd";

	std::string save_name = FileFinder::FindDefault(ss.str());
	Player::LoadSavegame(save_name);
	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Map>(true));
	}*/
}

void Scene_GameBrowser::Continue() {
	Player::ResetGameObjects();
	Game_System::SetSystemName(CACHE_DEFAULT_BITMAP);
	Game_System::BgmStop();
}

void Scene_GameBrowser::Update() {
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
	options.push_back("Exit");

	command_window.reset(new Window_Command(options, 88));
	command_window->SetY(32);
	command_window->SetIndex(0);

	gamelist_window.reset(new Window_GameList(88, 32, SCREEN_TARGET_WIDTH - 88, SCREEN_TARGET_HEIGHT - 32));
	gamelist_window->Refresh();

	if (!gamelist_window->HasValidGames()) {
		command_window->DisableItem(0);
	}

	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	help_window->SetText("EasyRPG Player - RPG Maker 2000/2003 interpreter");
}

void Scene_GameBrowser::UpdateCommand() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		int menu_index = command_window->GetIndex();

		switch (menu_index) {
		case 0:
			if (!gamelist_window->HasValidGames()) {
				return;
			}
			command_window->SetActive(false);
			command_window->SetIndex(-1);
			gamelist_window->SetActive(true);
			gamelist_window->SetIndex(0);
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
		gamelist_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		BootGame();
	}
}

void Scene_GameBrowser::BootGame() {
	const std::string& path = gamelist_window->GetGamePath();
	EASYRPG_SHARED_PTR<FileFinder::DirectoryTree> tree = FileFinder::CreateDirectoryTree(path);
	FileFinder::SetDirectoryTree(tree);

	Player::CreateGameObjects();

	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Title>());
}
