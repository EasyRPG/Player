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
#include <fstream>
#include <sstream>
#include <vector>
#include "game_config.h"
#include "options.h"
#include "scene_settings.h"
#include "scene_title.h"
#include "scene_language.h"
#include "audio.h"
#include "audio_secache.h"
#include "cache.h"
#include "game_battle.h"
#include "game_ineluki.h"
#include "game_screen.h"
#include "game_system.h"
#include "transition.h"
#include "input.h"
#include "main_data.h"
#include "meta.h"
#include "output.h"
#include "player.h"
#include "translation.h"
#include "scene_battle.h"
#include "scene_import.h"
#include "scene_load.h"
#include "window_command.h"
#include "baseui.h"
#include <lcf/reader_util.h>

Scene_Title::CommandOptionType Scene_Title::force_cursor_index = Scene_Title::CommandOption_None;

Scene_Title::Scene_Title() {
	type = Scene::Title;
}

void Scene_Title::Start() {
	Main_Data::game_system->ResetSystemGraphic();

	// Change the resolution of the window
	if (Player::has_custom_resolution) {
		Player::ChangeResolution(Player::screen_width, Player::screen_height);
	} else {
		switch (DisplayUi->GetConfig().game_resolution.Get()) {
			case ConfigEnum::GameResolution::Original:
				Player::ChangeResolution(SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT);
				break;
			case ConfigEnum::GameResolution::Widescreen:
				Player::ChangeResolution(416, SCREEN_TARGET_HEIGHT);
				Player::game_config.fake_resolution.Set(true);
				break;
			case ConfigEnum::GameResolution::Ultrawide:
				Player::ChangeResolution(560, SCREEN_TARGET_HEIGHT);
				Player::game_config.fake_resolution.Set(true);
				break;
		}
	}

	// Skip background image and music if not used
	if (CheckEnableTitleGraphicAndMusic()) {
		CreateTitleGraphic();
		PlayTitleMusic();
	}

	CreateCommandWindow();
}


void Scene_Title::Continue(SceneType prev_scene) {
	Main_Data::game_system->ResetSystemGraphic();

	if (restart_title_cache) {
		// Clear the cache when the game returns to the title screen
		// e.g. by pressing F12, except the Title Load menu
		Cache::ClearAll();
		AudioSeCache::Clear();

		Player::ResetGameObjects();
		if (Player::IsPatchKeyPatch()) {
			Main_Data::game_ineluki->ExecuteScriptList(FileFinder::Game().FindFile("autorun.script"));
		}

		Start();

		restart_title_cache = false;
	} else if (CheckEnableTitleGraphicAndMusic()) {
		CreateTitleGraphic();
	}

	if (prev_scene != Scene::Load && !Player::hide_title_flag) {
		command_window->SetOpenAnimation(8);
	}
}

void Scene_Title::TransitionIn(SceneType prev_scene) {
	if (Game_Battle::battle_test.enabled || CheckStartNewGame())
		return;

	if (prev_scene == Scene::Load || Player::hide_title_flag) {
		Scene::TransitionIn(prev_scene);
		return;
	}
	Transition::instance().InitShow(Transition::TransitionFadeIn, this);
}

void Scene_Title::Suspend(Scene::SceneType scene_type) {
	if (scene_type == Scene::Settings || scene_type == Scene::LanguageMenu) {
		restart_title_cache = true;
	}

	// Unload title graphic to save memory
	title.reset();
}

void Scene_Title::vUpdate() {
	if (Game_Battle::battle_test.enabled) {
		Player::SetupBattleTest();
		return;
	}

	if (CheckStartNewGame()) {
		Player::SetupNewGame();
		if (Player::debug_flag && Player::hide_title_flag) {
			Scene::Push(std::make_shared<Scene_Load>());
		}
		return;
	}

	command_window->Update();

	if (Input::IsTriggered(Input::DECISION)) {
		int index = command_window->GetIndex();
		if (index == indices.new_game) {  // New Game
			CommandNewGame();
		} else if (index == indices.continue_game) {  // Load Game
			CommandContinue();
		} else if (index == indices.import) {  // Import (multi-part games)
			CommandImport();
		} else if (index == indices.settings) {
			CommandSettings();
		} else if (index == indices.translate) { // Choose a Translation (Language)
			CommandTranslation();
		} else if (index == indices.exit) {  // Exit Game
			CommandShutdown();
		}
	} else if (Input::IsTriggered(Input::SHIFT)) {
		// For emscripten: Allow accessing the load scene for file upload with Shift
		int index = command_window->GetIndex();
		if (index == indices.continue_game) {
			CommandContinue();
		}
	}
}

void Scene_Title::Refresh() {
	// Enable load game if available
	continue_enabled = FileFinder::HasSavegame();

	if (force_cursor_index != CommandOption_None) {
		if (auto idx = GetCommandIndex(force_cursor_index); idx != -1) {
			command_window->SetIndex(idx);
		}
		force_cursor_index = CommandOption_None;
	} else if (continue_enabled) {
		command_window->SetIndex(1);
	}
	command_window->SetItemEnabled(1, continue_enabled);
}

int Scene_Title::GetCommandIndex(CommandOptionType cmd) const {
	auto it = std::find(command_options.begin(), command_options.end(), cmd);
	if (it != command_options.end()) {
		return (it - command_options.begin());
	}
	return -1;
}

void Scene_Title::OnTranslationChanged() {
	Start();

	command_window->SetIndex(indices.translate);

	Scene::OnTranslationChanged();
}

void Scene_Title::CreateTitleGraphic() {
	// Load Title Graphic
	if (!lcf::Data::system.title_name.empty()) {
		title.reset(new Sprite());
		FileRequestAsync* request = AsyncHandler::RequestFile("Title", lcf::Data::system.title_name);
		request->SetGraphicFile(true);
		request_id = request->Bind(&Scene_Title::OnTitleSpriteReady, this);
		request->Start();
	} else {
		title.reset(new Sprite());
		title->SetBitmap(Bitmap::Create(Player::screen_width, Player::screen_height, Color(0, 0, 0, 255)));
	}
}

void Scene_Title::RepositionWindow(Window_Command& window, bool center_vertical) {
	if (!center_vertical) {
		window.SetX(Player::screen_width / 2 - window.GetWidth() / 2);
		window.SetY(Player::screen_height * 53 / 60 - window.GetHeight());
	} else {
		window.SetX(Player::screen_width / 2 - window.GetWidth() / 2);
		window.SetY(Player::screen_height / 2 - window.GetHeight() / 2);
	}
}

void Scene_Title::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;

	using Cmd = CommandOptionType;

	// Reset index to fix issues on reuse.
	indices = CommandIndices();

	command_options.push_back(Cmd::NewGame);
	command_options.push_back(Cmd::ContinueGame);

	// Set "Import" based on metadata
	if (Player::meta->IsImportEnabled()) {
		command_options.push_back(Cmd::Import);
	}
	// Set "Settings" based on the configuration
	if (Player::player_config.settings_in_title.Get()) {
		command_options.push_back(Cmd::Settings);
	}
	// Set "Translate" based on metadata
	if (Player::translation.HasTranslations() && Player::player_config.lang_select_in_title.Get()) {
		command_options.push_back(Cmd::Translate);
	}

	command_options.push_back(Cmd::Exit);

	for (int i = 0; i < command_options.size(); ++i) {
		switch (command_options[i]) {
			case Cmd::NewGame:
				indices.new_game = i;
				options.push_back(ToString(lcf::Data::terms.new_game));
				break;
			case Cmd::ContinueGame:
				indices.continue_game = i;
				options.push_back(ToString(lcf::Data::terms.load_game));
				break;
			case Cmd::Import:
				indices.import = i;
				options.push_back(Player::meta->GetExVocabImportSaveTitleText());
				break;
			case Cmd::Settings:
				indices.settings = i;
				// FIXME: Translation? Not shown by default though
				options.push_back("Settings");
				break;
			case Cmd::Translate:
				indices.translate = i;
				options.push_back(Player::meta->GetExVocabTranslateTitleText());
				break;
			case Cmd::Exit:
				indices.exit = i;
				options.push_back(ToString(lcf::Data::terms.exit_game));
				break;
		}
	}

	command_window.reset(new Window_Command(options));
	RepositionWindow(*command_window, Player::hide_title_flag);

	Refresh();

	// Set the number of frames for the opening animation to last
	if (!Player::hide_title_flag) {
		command_window->SetOpenAnimation(8);
	}

	if (Player::IsRPG2k3E() && lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent) {
		command_window->SetBackOpacity(160);
	}

	command_window->SetVisible(true);
}

void Scene_Title::PlayTitleMusic() {
	// Workaround Android problem: BGM doesn't start when game is started again
	Main_Data::game_system->BgmStop();
	// Play BGM
	Main_Data::game_system->BgmPlay(lcf::Data::system.title_music);
}

bool Scene_Title::CheckEnableTitleGraphicAndMusic() {
	return Check2k3ShowTitle() &&
		!CheckStartNewGame() &&
		!Game_Battle::battle_test.enabled &&
		!Player::hide_title_flag;
}

bool Scene_Title::Check2k3ShowTitle() {
	return !Player::IsRPG2k3E() || (Player::IsRPG2k3E() && lcf::Data::system.show_title);
}

bool Scene_Title::CheckValidPlayerLocation() {
	return (lcf::Data::treemap.start.party_map_id > 0);
}

bool Scene_Title::CheckStartNewGame() {
	return !Check2k3ShowTitle() || Player::game_config.new_game.Get();
}

void Scene_Title::CommandNewGame() {
	if (!CheckValidPlayerLocation()) {
		Output::Warning("The game has no start location set.");
	} else {
		Output::Debug("Starting new game");
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		Player::SetupNewGame();
	}
}

void Scene_Title::CommandContinue() {
	if (continue_enabled || Input::IsTriggered(Input::SHIFT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	} else {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		return;
	}

	Scene::Push(std::make_shared<Scene_Load>());
}

void Scene_Title::CommandImport() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	Scene::Push(std::make_shared<Scene_Import>());
}

void Scene_Title::CommandSettings() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	Scene::Push(std::make_unique<Scene_Settings>());
}

void Scene_Title::CommandTranslation() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	Scene::Push(std::make_unique<Scene_Language>());
}

void Scene_Title::CommandShutdown() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	Transition::instance().InitErase(Transition::TransitionFadeOut, this);
	Scene::Pop();
}

void Scene_Title::OnTitleSpriteReady(FileRequestResult* result) {
	BitmapRef bitmapRef = Cache::Title(result->file);

	title->SetBitmap(bitmapRef);

	// If the title sprite doesn't fill the screen, center it to support custom resolutions
	if (bitmapRef->GetWidth() < Player::screen_width) {
		title->SetX(Player::menu_offset_x);
	}
	if (bitmapRef->GetHeight() < Player::screen_height) {
		title->SetY(Player::menu_offset_y);
	}
}

void Scene_Title::OnGameStart() {
	restart_title_cache = true;
}
