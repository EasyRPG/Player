/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "player.h"
#include "system.h"
#include "output.h"
#include "audio.h"
#include "graphics.h"
#include "input.h"
#include "cache.h"
#include "main_data.h"
#include "scene_logo.h"
#include "scene_title.h"

////////////////////////////////////////////////////////////
namespace Player {
	bool exit_flag;
	bool reset_flag;
	bool debug_flag;
	bool hide_title_flag;
	bool window_flag;
	bool battle_test_flag;
	//bool battle_test_troop_id;
}

////////////////////////////////////////////////////////////
void Player::Init(int argc, char *argv[]) {
	static bool init = false;

	if (init) return;

	exit_flag = false;
	reset_flag = false;
	debug_flag = false;
	hide_title_flag = false;
	window_flag = false;
	battle_test_flag = false;
	//battle_test_troop_id = NULL;

	if (argc > 1 && !strcmp(argv[1], "TestPlay")) {
		debug_flag = true;
	}
	if (argc > 2 && !strcmp(argv[2], "HideTitle")) {
		hide_title_flag = true;
	}
	if (argc > 3 && !strcmp(argv[3], "Window")) {
		window_flag = true;
	}
	if (argc > 4 && !strcmp(argv[1], "BattleTest")) {
		if (atoi(argv[4])) {
//			battle_test_troop_id = atoi(argv[4]);
		} else {
			Output::Error("Invalid troop ID.\n");
		}
	}
	DisplayUi = BaseUi::CreateBaseUi(
		SCREEN_TARGET_WIDTH,
		SCREEN_TARGET_HEIGHT,
		GAME_TITLE,
		!window_flag,
		RUN_ZOOM
	);

	init = true;
}

////////////////////////////////////////////////////////////
void Player::Run() {
if (debug_flag) {
	Scene::instance = new Scene_Title();
} else {
	Scene::instance = new Scene_Logo();
}

	reset_flag = false;
	
	// Main loop
	while (Scene::type != Scene::Null) {
		Scene::instance->MainFunction();
		delete Scene::old_instance;
	}

	Player::Exit();
}

////////////////////////////////////////////////////////////
void Player::Pause() {
	Graphics::TimerWait();
	Audio::BGM_Pause();
}

////////////////////////////////////////////////////////////
void Player::Resume() {
	Audio::BGM_Resume();
	Graphics::TimerContinue();
}

////////////////////////////////////////////////////////////
void Player::Update() {
	DisplayUi->ProcessEvents();

	if (exit_flag) {
		Exit();
		exit(EXIT_SUCCESS);
	} else if (reset_flag) {
		reset_flag = false;
		Scene::instance = new Scene_Title();
	}
}

////////////////////////////////////////////////////////////
void Player::Exit() {
	Main_Data::Cleanup();
	Graphics::Quit();
	Audio::Quit();
	delete DisplayUi;
}
