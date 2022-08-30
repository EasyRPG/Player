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
#include "scene_title.h"
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

Scene_Title::Scene_Title() {
	type = Scene::Title;
}

void Scene_Title::Start() {
	Main_Data::game_system->ResetSystemGraphic();

	// Skip background image and music if not used
	if (CheckEnableTitleGraphicAndMusic()) {
		CreateTitleGraphic();
		PlayTitleMusic();
	}

	CreateCommandWindow();
	CreateTranslationWindow();
	CreateHelpWindow();
}

void Scene_Title::CreateHelpWindow() {
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));

	if (Player::IsRPG2k3E() && lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent) {
		help_window->SetBackOpacity(160);
	}

	help_window->SetVisible(false);
	translate_window->SetHelpWindow(help_window.get());
}


void Scene_Title::Continue(SceneType prev_scene) {
	Main_Data::game_system->ResetSystemGraphic();

	if (restart_title_cache) {
		// Clear the cache when the game returns to the title screen
		// e.g. by pressing F12, except the Title Load menu
		Cache::ClearAll();
		AudioSeCache::Clear();

		Player::ResetGameObjects();
		Main_Data::game_ineluki->ExecuteScriptList(FileFinder::Game().FindFile("autorun.script"));

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
	if (Game_Battle::battle_test.enabled || !lcf::Data::system.show_title || Player::new_game_flag)
		return;

	if (prev_scene == Scene::Load || Player::hide_title_flag) {
		Scene::TransitionIn(prev_scene);
		return;
	}
	Transition::instance().InitShow(Transition::TransitionFadeIn, this);
}

void Scene_Title::Suspend(Scene::SceneType next_scene) {
	// Unload title graphic to save memory
	title.reset();
}

void Scene_Title::Update() {
	if (Game_Battle::battle_test.enabled) {
		Player::SetupBattleTest();
		return;
	}

	if (!lcf::Data::system.show_title || Player::new_game_flag) {
		Player::SetupNewGame();
		if (Player::debug_flag && Player::hide_title_flag) {
			Scene::Push(std::make_shared<Scene_Load>());
		}
		return;
	}

	if (active_window == 0) {
		command_window->Update();
	} else {
		translate_window->Update();
	}

	if (Input::IsTriggered(Input::DECISION)) {
		if (active_window == 0) {
			int index = command_window->GetIndex();
			if (index == indices.new_game) {  // New Game
				CommandNewGame();
			} else if (index == indices.continue_game) {  // Load Game
				CommandContinue();
			} else if (index == indices.import) {  // Import (multi-part games)
				CommandImport();
			} else if (index == indices.translate) { // Choose a Translation (Language)
				CommandTranslation();
			} else if (index == indices.exit) {  // Exit Game
				CommandShutdown();
			}
		} else if (active_window == 1) {
			int index = translate_window->GetIndex();
			ChangeLanguage(lang_dirs.at(index));
		}
	} else if (Input::IsTriggered(Input::CANCEL)) {
		if (active_window == 1) {
			// Switch back
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			HideTranslationWindow();
		}
	}
}

void Scene_Title::OnTranslationChanged() {
	Start();

	command_window->SetIndex(indices.translate);
	HideTranslationWindow();

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
		title->SetBitmap(Bitmap::Create(DisplayUi->GetWidth(), DisplayUi->GetHeight(), Color(0, 0, 0, 255)));
	}
}

void Scene_Title::RepositionWindow(Window_Command& window, bool center_vertical) {
	if (!center_vertical) {
		window.SetX(SCREEN_TARGET_WIDTH / 2 - window.GetWidth() / 2);
		window.SetY(SCREEN_TARGET_HEIGHT * 53 / 60 - window.GetHeight());
	} else {
		window.SetX(SCREEN_TARGET_WIDTH / 2 - window.GetWidth() / 2);
		window.SetY(SCREEN_TARGET_HEIGHT / 2 - window.GetHeight() / 2);
	}
}

void Scene_Title::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(ToString(lcf::Data::terms.new_game));
	options.push_back(ToString(lcf::Data::terms.load_game));

	// Reset index to fix issues on reuse.
	indices = CommandIndices();

	// Set "Import" based on metadata
	if (Player::meta->IsImportEnabled()) {
		options.push_back(Player::meta->GetExVocabImportSaveTitleText());
		indices.import = indices.exit;
		indices.exit++;
	}

	// Set "Translate" based on metadata
	if (Player::translation.HasTranslations()) {
		options.push_back(Player::meta->GetExVocabTranslateTitleText());
		indices.translate = indices.exit;
		indices.exit++;
	}

	options.push_back(ToString(lcf::Data::terms.exit_game));

	command_window.reset(new Window_Command(options));
	RepositionWindow(*command_window, Player::hide_title_flag);

	// Enable load game if available
	continue_enabled = FileFinder::HasSavegame();
	if (continue_enabled) {
		command_window->SetIndex(1);
	} else {
		command_window->DisableItem(1);
	}

	// Set the number of frames for the opening animation to last
	if (!Player::hide_title_flag) {
		command_window->SetOpenAnimation(8);
	}

	if (Player::IsRPG2k3E() && lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent) {
		command_window->SetBackOpacity(160);
	}

	command_window->SetVisible(true);
}

void Scene_Title::CreateTranslationWindow() {
	// Build a list of 'Default' and all known languages.
	std::vector<std::string> lang_names;
	lang_names.push_back("Default Language");
	lang_dirs.push_back("");
	lang_helps.push_back("Play the game in its original language.");

	// Push menu entries with the display name, but also save the directory location and help text.
	for (const Language& lg : Player::translation.GetLanguages()) {
		lang_names.push_back(lg.lang_name);
		lang_dirs.push_back(lg.lang_dir);
		lang_helps.push_back(lg.lang_desc);
	}

	translate_window.reset(new Window_Command(lang_names));
	translate_window->UpdateHelpFn = [this](Window_Help& win, int index) {
		if (index >= 0 && index < static_cast<int>(lang_helps.size())) {
			win.SetText(lang_helps[index]);
		} else {
			win.SetText("");
		}
	};
	RepositionWindow(*translate_window, Player::hide_title_flag);

	if (Player::IsRPG2k3E() && lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent) {
		translate_window->SetBackOpacity(160);
	}

	translate_window->SetVisible(false);
}

void Scene_Title::PlayTitleMusic() {
	// Workaround Android problem: BGM doesn't start when game is started again
	Main_Data::game_system->BgmStop();
	// Play BGM
	Main_Data::game_system->BgmPlay(lcf::Data::system.title_music);
}

bool Scene_Title::CheckEnableTitleGraphicAndMusic() {
	return lcf::Data::system.show_title &&
		!Player::new_game_flag &&
		!Game_Battle::battle_test.enabled &&
		!Player::hide_title_flag;
}

bool Scene_Title::CheckValidPlayerLocation() {
	return (lcf::Data::treemap.start.party_map_id > 0);
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
	if (continue_enabled) {
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

void Scene_Title::CommandTranslation() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	// Switch windows
	active_window = 1;
	command_window->SetVisible(false);
	translate_window->SetVisible(true);
	help_window->SetVisible(true);
}

void Scene_Title::ChangeLanguage(const std::string& lang_str) {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	// No-op?
	if (lang_str == Player::translation.GetCurrentLanguage().lang_dir) {
		HideTranslationWindow();
		return;
	}

	// First change the language
	Player::translation.SelectLanguage(lang_str);
}

void Scene_Title::HideTranslationWindow() {
	active_window = 0;
	command_window->SetVisible(true);
	translate_window->SetVisible(false);
	help_window->SetVisible(false);
}

void Scene_Title::CommandShutdown() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	Transition::instance().InitErase(Transition::TransitionFadeOut, this);
	Scene::Pop();
}

void Scene_Title::OnTitleSpriteReady(FileRequestResult* result) {
	title->SetBitmap(Cache::Title(result->file));
}

void Scene_Title::OnGameStart() {
	restart_title_cache = true;
}
