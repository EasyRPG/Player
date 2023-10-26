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
#include "exe_reader.h"
#include "filefinder.h"
#include "game_battle.h"
#include "input.h"
#include "options.h"
#include "player.h"
#include "scene_title.h"
#include "scene_gamebrowser.h"
#include "scene_settings.h"
#include "output.h"
#include "generated/logo.h"
#include "generated/logo2.h"
#include "utils.h"
#include "rand.h"
#include "text.h"
#include "version.h"
#include <ctime>
#include <memory>

Scene_Logo::Scene_Logo() :
	frame_counter(0) {
	type = Scene::Logo;
	skip_logos = Player::debug_flag || Game_Battle::battle_test.enabled;
}

Scene_Logo::Scene_Logo(std::vector<std::vector<uint8_t>> logos, unsigned current_logo_index) :
	frame_counter(0), logos(std::move(logos)), current_logo_index(current_logo_index) {

	type = Scene::Logo;

	assert(current_logo_index > 0);
	detected_game = true;
}

void Scene_Logo::Start() {
	if (!skip_logos) {
		logo_img = LoadLogo();
		DrawTextOnLogo(false);
		DrawLogo(logo_img);
	}
}

void Scene_Logo::vUpdate() {
	if (current_logo_index == 0 && frame_counter == 0) {
		if (!DetectGame()) {
			// async delay for emscripten
			return;
		}

		logos = LoadLogos();
	}

	++frame_counter;

	if (Input::IsPressed(Input::SHIFT)) {
		DrawTextOnLogo(true);
		--frame_counter;
	}

	// Allow calling the settings when the first logo was shown (startup completed)
	if (current_logo_index > 0 && Input::IsTriggered(Input::SETTINGS_MENU)) {
		Scene::Push(std::make_shared<Scene_Settings>());
	}

	// other logos do not invoke the slow CreateGameObjects: display them longer
	bool frame_limit_reached = (frame_counter == (current_logo_index == 0 ? 60 : 90));

	if (skip_logos ||
		frame_limit_reached ||
		Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {

		if (detected_game) {
			if (!skip_logos) {
				// Check for another logo
				if (current_logo_index < logos.size()) {
					Scene::Push(std::make_shared<Scene_Logo>(std::move(logos), current_logo_index + 1), true);
					return;
				}
			}

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

bool Scene_Logo::DetectGame() {
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
		return false;
	}
	if (!async_ready) {
		return false;
	}
#endif

	if (FileFinder::IsValidProject(fs) || FileFinder::OpenViewToEasyRpgFile(fs)) {
		FileFinder::SetGameFilesystem(fs);
		Player::CreateGameObjects();
		detected_game = true;
	}

	return true;
}

BitmapRef Scene_Logo::LoadLogo() {
	BitmapRef current_logo;
	std::time_t t = std::time(nullptr);
	std::tm* tm = std::localtime(&t);

	if (current_logo_index == 0) {
		// Load the built-in logo
		if (Rand::ChanceOf(1, 32) || (tm->tm_mday == 1 && tm->tm_mon == 3)) {
			current_logo = Bitmap::Create(easyrpg_logo2, sizeof(easyrpg_logo2), false);
		}
		else {
			current_logo = Bitmap::Create(easyrpg_logo, sizeof(easyrpg_logo), false);
		}
	} else {
		// Load external logos
		const auto& logo_bytes = logos[current_logo_index - 1];
		current_logo = Bitmap::Create(logo_bytes.data(), logo_bytes.size(), false);
	}

	return current_logo;
}

void Scene_Logo::DrawLogo(BitmapRef logo_img) {
	logo = std::make_unique<Sprite>();
	logo->SetBitmap(logo_img);
	logo->SetX((Player::screen_width - logo->GetWidth()) / 2);
	logo->SetY((Player::screen_height - logo->GetHeight()) / 2);
}

void Scene_Logo::DrawBackground(Bitmap& dst) {
	dst.Clear();
}

void Scene_Logo::DrawTextOnLogo(bool verbose) {
	if (current_logo_index > 0) {
		// only render version info on EasyRPG startup logo
		return;
	}

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

std::vector<std::vector<uint8_t>> Scene_Logo::LoadLogos() {
	if (Player::player_config.show_startup_logos.Get() == StartupLogos::None) {
		return {};
	}

	std::vector<std::vector<uint8_t>> logos;

	for (int i = 1; i < 100; ++i) {
		auto is = FileFinder::OpenImage("Logo", "LOGO" + std::to_string(i));
		if (is) {
			logos.push_back(Utils::ReadStream(is));
		} else {
			break;
		}
	}

#ifndef EMSCRIPTEN
	if (logos.empty()) {
		// Attempt reading Logos from RPG_RT.exe (not supported on Emscripten)
		auto exeis = FileFinder::Game().OpenFile(EXE_NAME);

		if (exeis) {
			std::unique_ptr<EXEReader> exe_reader = std::make_unique<EXEReader>(std::move(exeis));
			logos = exe_reader->GetLogos();
		}
	}
#endif

	return logos;
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
		"autorun.script", // Key Patch Startup script,
		"Logo/Logo1", // up to 3 custom startup logos
		"Logo/Logo2",
		"Logo/Logo3"
	);

	for (auto file: startup_files) {
		FileRequestAsync* req = AsyncHandler::RequestFile(file);
		req->SetImportantFile(true);
		req->Start();
	}
}
