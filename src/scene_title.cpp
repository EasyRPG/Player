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
#include <vector>
#include "scene_title.h"
#include "audio.h"
#include "audio_secache.h"
#include "cache.h"
#include "game_battle.h"
#include "game_screen.h"
#include "game_system.h"
#include "game_temp.h"
#include "transition.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "scene_battle.h"
#include "scene_load.h"
#include "scene_map.h"
#include "window_command.h"
#include "player.h"

Scene_Title::Scene_Title() {
	type = Scene::Title;
}

void Scene_Title::Start() {
	// Skip background image and music if not used
	if (Data::system.show_title && !Player::new_game_flag &&
		!Game_Battle::battle_test.enabled && !Player::hide_title_flag) {
		CreateTitleGraphic();
		PlayTitleMusic();
	}

	CreateCommandWindow();
}

void Scene_Title::Continue() {
	if (restart_title_cache) {
		// Clear the cache when the game returns to the title screen
		// e.g. by pressing F12, except the Title Load menu
		Cache::Clear();
		AudioSeCache::Clear();

		Player::ResetGameObjects();

		Start();
	}
}

void Scene_Title::TransitionIn() {
	if (Game_Battle::battle_test.enabled || !Data::system.show_title || Player::new_game_flag)
		return;

	if (command_window->GetVisible()) {
		Scene::TransitionIn();
	}
	else if (!Player::hide_title_flag) {
		Player::TransitionShow(Transition::TransitionFadeIn, 32, this);
	} else {
		Player::TransitionShow(Transition::TransitionFadeIn, 6, this);
	}
}

void Scene_Title::Resume() {
	if (!Data::system.show_title || Player::new_game_flag)
		return;

	if (command_window) {
		command_window->SetVisible(true);
	}
}
void Scene_Title::Update() {
	if (Game_Battle::battle_test.enabled) {
		PrepareBattleTest();
		return;
	}

	if (!Data::system.show_title || Player::new_game_flag) {
		Player::SetupPlayerSpawn();
		Scene::Push(std::make_shared<Scene_Map>());
		if (Player::debug_flag && Player::hide_title_flag) {
			Scene::Push(std::make_shared<Scene_Load>());
		}
		return;
	}

	command_window->Update();

	if (Input::IsTriggered(Input::DECISION)) {
		switch (command_window->GetIndex()) {
		case 0: // New Game
			CommandNewGame();
			break;
		case 1: // Load Game
			CommandContinue();
			break;
		case 2: // Exit Game
			CommandShutdown();
		}
	}
}

void Scene_Title::CreateTitleGraphic() {
	// Load Title Graphic
	if (!title && !Data::system.title_name.empty()) // No need to recreate Title on Resume
	{
		title.reset(new Sprite());
		FileRequestAsync* request = AsyncHandler::RequestFile("Title", Data::system.title_name);
		request_id = request->Bind(&Scene_Title::OnTitleSpriteReady, this);
		request->Start();
	}
}

void Scene_Title::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Data::terms.new_game);
	options.push_back(Data::terms.load_game);
	options.push_back(Data::terms.exit_game);

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

	if (Player::IsRPG2k3E() && Data::battlecommands.transparency == RPG::BattleCommands::Transparency_transparent) {
		command_window->SetBackOpacity(128);
	}

	command_window->SetVisible(false);
}

void Scene_Title::PlayTitleMusic() {
	// Workaround Android problem: BGM doesn't start when game is started again
	Game_System::BgmStop();
	// Play BGM
	Game_System::BgmPlay(Data::system.title_music);
}

bool Scene_Title::CheckValidPlayerLocation() {
	return (Data::treemap.start.party_map_id > 0);
}

void Scene_Title::PrepareBattleTest() {
	Scene::Push(Scene_Battle::Create(), true);
}

void Scene_Title::CommandNewGame() {
	if (!CheckValidPlayerLocation()) {
		Output::Warning("The game has no start location set.");
	} else {
		Output::Debug("Starting new game");
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Game_System::BgmStop();
		Player::SetupPlayerSpawn();
		// RPG_RT compatible frame counter.
		Main_Data::game_data.system.frame_count = 0;
		Scene::Push(std::make_shared<Scene_Map>());
	}
}

void Scene_Title::CommandContinue() {
	if (continue_enabled) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	} else {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	restart_title_cache = false;
	Scene::Push(std::make_shared<Scene_Load>());
}

void Scene_Title::CommandShutdown() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	Player::TransitionErase(Transition::TransitionFadeOut, 32, this);
	Scene::Pop();
}

void Scene_Title::OnTitleSpriteReady(FileRequestResult* result) {
	title->SetBitmap(Cache::Title(result->file));
}

void Scene_Title::OnGameLoad() {
	restart_title_cache = true;
}
