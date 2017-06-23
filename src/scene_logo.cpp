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
#include "scene_logo.h"
#include "async_handler.h"
#include "bitmap.h"
#include "filefinder.h"
#include "input.h"
#include "player.h"
#include "scene_map.h"
#include "scene_title.h"
#include "scene_gamebrowser.h"
#include "output.h"
#include "logo.h"

Scene_Logo::Scene_Logo() :
	frame_counter(0) {
	type = Scene::Logo;
}

void Scene_Logo::Start() {
	logo.reset(new Sprite());
	if (!Player::debug_flag) {
		logo_img = Bitmap::Create(easyrpg_logo, sizeof(easyrpg_logo), false);
		logo->SetBitmap(logo_img);
	}
}

void Scene_Logo::Update() {
	static bool is_valid = false;

	if (frame_counter == 0) {
#ifdef EMSCRIPTEN
		static bool once = true;
		if (once) {
			FileRequestAsync* index = AsyncHandler::RequestFile("index.json");
			index->SetImportantFile(true);
			request_id = index->Bind(&Scene_Logo::OnIndexReady, this);
			once = false;
			index->Start();
			return;
		}

		if (!async_ready) {
			return;
		}
#endif

		std::shared_ptr<FileFinder::DirectoryTree> tree = FileFinder::CreateDirectoryTree(Main_Data::GetProjectPath(), false);

		if (!tree) {
			Output::Error("%s is not a valid path", Main_Data::GetProjectPath().c_str());
		}

		if (FileFinder::IsValidProject(*tree)) {
			FileFinder::SetDirectoryTree(FileFinder::CreateDirectoryTree(Main_Data::GetProjectPath()));
			Player::CreateGameObjects();
			is_valid = true;
		}
	}

	++frame_counter;

	if (Player::debug_flag ||
		frame_counter == 60 ||
		Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {

		if (is_valid) {
			Scene::Push(std::make_shared<Scene_Title>(), true);
			if (Player::load_game_id > 0) {
				std::shared_ptr<FileFinder::DirectoryTree> tree = FileFinder::CreateSaveDirectoryTree();

				std::stringstream ss;
				ss << "Save" << (Player::load_game_id <= 9 ? "0" : "") << Player::load_game_id << ".lsd";

				Output::Debug("Loading Save %s", ss.str().c_str());

				std::string save_name = FileFinder::FindDefault(*tree, ss.str());
				Player::LoadSavegame(save_name);
				Scene::Push(std::make_shared<Scene_Map>(true));
			}
		}
		else {
			Scene::Push(std::make_shared<Scene_GameBrowser>(), true);
		}
	}
}

void Scene_Logo::OnIndexReady(FileRequestResult*) {
	async_ready = true;

	if (!FileFinder::Exists("index.json")) {
		Output::Debug("index.json not found. The game does not exist or was not correctly deployed.");
		return;
	}

	AsyncHandler::CreateRequestMapping("index.json");

	FileRequestAsync* db = AsyncHandler::RequestFile(DATABASE_NAME);
	db->SetImportantFile(true);
	FileRequestAsync* tree = AsyncHandler::RequestFile(TREEMAP_NAME);
	tree->SetImportantFile(true);
	FileRequestAsync* ini = AsyncHandler::RequestFile(INI_NAME);
	ini->SetImportantFile(true);

	db->Start();
	tree->Start();
	ini->Start();
}
