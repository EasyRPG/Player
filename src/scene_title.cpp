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
#include "game_screen.h"
#include "game_system.h"
#include "transition.h"
#include "input.h"
#include "main_data.h"
#include "meta.h"
#include "output.h"
#include "player.h"
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
	Game_System::ResetSystemGraphic();

	// Skip background image and music if not used
	if (CheckEnableTitleGraphicAndMusic()) {
		CreateTitleGraphic();
		PlayTitleMusic();
	}

	CreateCommandWindow();
}


void Scene_Title::Continue(SceneType prev_scene) {
	Game_System::ResetSystemGraphic();

	if (restart_title_cache) {
		// Clear the cache when the game returns to the title screen
		// e.g. by pressing F12, except the Title Load menu
		Cache::Clear();
		AudioSeCache::Clear();

		Player::ResetGameObjects();

		Start();
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

void Scene_Title::TransitionOut(Scene::SceneType next_scene) {
	Scene::TransitionOut(next_scene);

	// Unload title graphic to save memory
	title.reset();
}

void Scene_Title::Update() {
	if (Game_Battle::battle_test.enabled) {
		PrepareBattleTest();
		return;
	}

	if (!lcf::Data::system.show_title || Player::new_game_flag) {
		Player::SetupNewGame();
		if (Player::debug_flag && Player::hide_title_flag) {
			Scene::Push(std::make_shared<Scene_Load>());
		}
		return;
	}

	command_window->Update();

	if (Input::IsTriggered(Input::DECISION)) {
		int index = command_window->GetIndex();
		if (index == new_game_index) {  // New Game
			CommandNewGame();
		} else if (index == continue_index) {  // Load Game
			CommandContinue();
		} else if (index == import_index) {  // Import (multi-part games)
			CommandImport();
		} else if (index == exit_index) {  // Exit Game
			CommandShutdown();
		}
	}
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

void Scene_Title::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(lcf::Data::terms.new_game);
	options.push_back(lcf::Data::terms.load_game);

	// Set "Import" based on metadata
	if (Player::meta->IsImportEnabled()) {
		options.push_back(Player::meta->GetExVocabImportSaveTitleText());
		import_index = 2;
		exit_index = 3;
	}

	options.push_back(lcf::Data::terms.exit_game);

	command_window.reset(new Window_Command(options));
	if (!Player::hide_title_flag) {
		command_window->SetX(SCREEN_TARGET_WIDTH / 2 - command_window->GetWidth() / 2);
		command_window->SetY(SCREEN_TARGET_HEIGHT * 53 / 60 - command_window->GetHeight());
	} else {
		command_window->SetX(SCREEN_TARGET_WIDTH / 2 - command_window->GetWidth() / 2);
		command_window->SetY(SCREEN_TARGET_HEIGHT / 2 - command_window->GetHeight() / 2);
	}
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
		command_window->SetBackOpacity(128);
	}

	command_window->SetVisible(true);
}

void Scene_Title::PlayTitleMusic() {
	// Workaround Android problem: BGM doesn't start when game is started again
	Game_System::BgmStop();
	// Play BGM
	Game_System::BgmPlay(lcf::Data::system.title_music);
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

void Scene_Title::PrepareBattleTest() {
	BattleArgs args;
	args.troop_id = Game_Battle::battle_test.troop_id;
	args.first_strike = false;
	args.allow_escape = true;
	args.background = lcf::Data::system.battletest_background;
	args.terrain_id = 1; //Not used in 2k, for 2k3 only used to determine grid layout if formation == terrain.

	if (Player::IsRPG2k3()) {
		args.formation = Game_Battle::battle_test.formation;
		args.condition = Game_Battle::battle_test.condition;

		if (args.formation == lcf::rpg::System::BattleFormation_terrain) {
			args.terrain_id = Game_Battle::battle_test.terrain_id;
		}

		Output::Debug("BattleTest Mode 2k3 troop=({}) background=({}) formation=({}) condition=({}) terrain=({})",
				args.troop_id, args.background.c_str(), args.formation, args.condition, args.terrain_id);
	} else {
		Output::Debug("BattleTest Mode 2k troop=({}) background=({})", args.troop_id, args.background);
	}

	auto* troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, args.troop_id);
	if (troop == nullptr) {
		Output::Error("BattleTest: Invalid Monster Party ID {}", args.troop_id);
	}

	Scene::Push(Scene_Battle::Create(std::move(args)), true);
}

void Scene_Title::CommandNewGame() {
	if (!CheckValidPlayerLocation()) {
		Output::Warning("The game has no start location set.");
	} else {
		Output::Debug("Starting new game");
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Player::SetupNewGame();
	}
}

void Scene_Title::CommandContinue() {
	if (continue_enabled) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	} else {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	Scene::Push(std::make_shared<Scene_Load>());
}

void Scene_Title::CommandImport() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	Scene::Push(std::make_shared<Scene_Import>());
}

void Scene_Title::CommandShutdown() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	Transition::instance().InitErase(Transition::TransitionFadeOut, this);
	Scene::Pop();
}

void Scene_Title::OnTitleSpriteReady(FileRequestResult* result) {
	title->SetBitmap(Cache::Title(result->file));
}

void Scene_Title::OnGameStart() {
	restart_title_cache = true;
}
