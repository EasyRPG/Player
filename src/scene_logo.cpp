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
#include "game_battle.h"
#include "input.h"
#include "options.h"
#include "player.h"
#include "scene_title.h"
#include "scene_gamebrowser.h"
#include "output.h"
#include "generated/logo.h"
#include "generated/logo2.h"
#include "utils.h"
#include "rand.h"
#include "text.h"
#include "version.h"
#include <ctime>

Scene_Logo::Scene_Logo() :
	frame_counter(0) {
	type = Scene::Logo;
}

static bool detected_game = false;

void Scene_Logo::Start() {
	

	if (!Player::debug_flag && !Game_Battle::battle_test.enabled) {
		// Preload logos if the container is empty
		if (Player::logos_container.empty()) {
			Player::logos_container = Scene_Logo::preloadLogos();
		}

		// Access the logo based on Player::current_logo
		if (Player::current_logo < Player::logos_container.size()) {
			logo_img = Player::logos_container[Player::current_logo];
		}
		else {
			// Handle the case when Player::current_logo exceeds the number of logos
			// You may want to add error handling here.
		}

		DrawText(false);

		logo = std::make_unique<Sprite>();
		logo->SetBitmap(logo_img);
		logo->SetX((Player::screen_width - logo->GetWidth()) / 2);
		logo->SetY((Player::screen_height - logo->GetHeight()) / 2);
	}
}

void Scene_Logo::vUpdate() {
	static bool is_valid = detected_game;
	++frame_counter;

	if (Input::IsPressed(Input::SHIFT) && Player::current_logo == 0) {
		DrawText(true);
		--frame_counter;
	}

	if (Player::debug_flag ||
		Game_Battle::battle_test.enabled ||
		frame_counter == (Player::current_logo == 0 ? 60 : 90) || //had to be longer to cover when Player::CreateGameObjects() doesn't happen
		Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {

		Player::current_logo++;
		if (Player::current_logo < Player::logos_container.size()) {
			Scene::Pop();
			Scene::Push(std::make_shared<Scene_Logo>());
			return;
		}

		Player::current_logo = 0;
		Player::logos_container.clear();

		if (is_valid) {
			if (!Player::startup_language.empty()) {
				Player::translation.SelectLanguage(Player::startup_language);
			}
			Scene::Push(std::make_shared<Scene_Title>(), true);
			if (Player::load_game_id > 0) {
				auto save = FileFinder::Save();

				std::stringstream ss;
				ss << "Save" << (Player::load_game_id <= 9 ? "0" : "") << Player::load_game_id << ".lsd";

				Output::Debug("Loading Save {}", ss.str());

				std::string save_name = save.FindFile(ss.str());
				Player::LoadSavegame(save_name, Player::load_game_id);
			}
		}
		else {
			Scene::Push(std::make_shared<Scene_GameBrowser>(), true);
		}
	}
}

void Scene_Logo::DetectGame() {

	if (detected_game) {
		return;
	}

	auto fs = FileFinder::Game();
	if (!fs) {
		fs = FileFinder::Root().Create(Main_Data::GetDefaultProjectPath());
		if (!fs) {
			Output::Error("{} is not a valid path", Main_Data::GetDefaultProjectPath());
		}
		FileFinder::SetGameFilesystem(fs);
	}

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

	if (FileFinder::IsValidProject(fs)) {
		Player::CreateGameObjects();
		detected_game = true;
	}
}

std::vector<BitmapRef> Scene_Logo::preloadLogos() {
	std::vector<BitmapRef> logos;
	std::time_t t = std::time(nullptr);
	std::tm* tm = std::localtime(&t);

	for (int logoIndex = 0; ; logoIndex++) {
		Filesystem_Stream::InputStream logoStream;
		if (logoIndex != 0) logoStream = FileFinder::OpenImage("Logo", "LOGO" + std::to_string(logoIndex));
		//TODO: Maybe get LOGO1,LOGO2,LOGO3 from rpg_rt too?

		if (!logoStream) {
			if (logoIndex == 0) {
				if (Rand::ChanceOf(1, 32) || (tm->tm_mday == 1 && tm->tm_mon == 3)) {
					logos.push_back(Bitmap::Create(easyrpg_logo2, sizeof(easyrpg_logo2), false));
				}
				else {
					logos.push_back(Bitmap::Create(easyrpg_logo, sizeof(easyrpg_logo), false));
				}
				Scene_Logo::DetectGame();
			}
			else break;  // Stop when no more logos can be found
		}
		else {
			// Read the data from logoStream and store it in a variable
			std::vector<uint8_t> logoData = Utils::ReadStream(logoStream);

			// Access the data as needed
			const uint8_t* cachedLogo = logoData.data();
			size_t logoSize = logoData.size();

			// Create a bitmap using the logo data and store it
			logos.push_back(Bitmap::Create(cachedLogo, logoSize, false));
		}
	}

	return logos;
}

void Scene_Logo::DrawBackground(Bitmap& dst) {
	dst.Clear();
}

void Scene_Logo::DrawText(bool verbose) {
	if (Player::current_logo != 0) return;

	Rect text_rect = { 17, 215, 320 - 32, 16 * verbose }; //last argument (rect height) is now 0 to remove a black rectangle that appears as text background color.
	Color text_color = {185, 199, 173, 255};
	Color shadow_color = {69, 69, 69, 255};
	logo_img->ClearRect(text_rect);

	for (auto& color: {shadow_color, text_color}) {
		logo_img->TextDraw(text_rect, color, "v" + Version::GetVersionString(verbose, verbose), Text::AlignLeft);
		if (!verbose) {
			logo_img->TextDraw(text_rect, color, WEBSITE_ADDRESS, Text::AlignRight);
		}
		text_rect.x--;
		text_rect.y--;
	}

}

void Scene_Logo::OnIndexReady(FileRequestResult*) {
	async_ready = true;

	if (!FileFinder::Game().Exists("index.json")) {
		Output::Debug("index.json not found. The game does not exist or was not correctly deployed.");
		return;
	}

	AsyncHandler::CreateRequestMapping("index.json");

	auto startup_files = Utils::MakeSvArray(
		DATABASE_NAME, // Essential game files
		TREEMAP_NAME,
		INI_NAME,
		EASYRPG_INI_NAME, // EasyRPG specific configuration
		"Font/ExFont", // Custom ExFont
		"Font/Font", // Custom Gothic Font
		"Font/Font2", // Custom Mincho Font
		"easyrpg.soundfont", // Custom SF2 soundfont
		"autorun.script" // Key Patch Startup script
	);

	for (auto file: startup_files) {
		FileRequestAsync* req = AsyncHandler::RequestFile(file);
		req->SetImportantFile(true);
		req->Start();
	}
}
