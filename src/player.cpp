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
#include "scene_battle.h"
#include <cstring>
#include <cstdlib>
#ifdef GEKKO
	#include <fat.h>
#endif

////////////////////////////////////////////////////////////
namespace Player {
	bool exit_flag;
	bool reset_flag;
	bool debug_flag;
	bool hide_title_flag;
	bool window_flag;
	bool battle_test_flag;
	int battle_test_troop_id;
	EngineType engine;
}

////////////////////////////////////////////////////////////
void Player::Init(int argc, char *argv[]) {
	static bool init = false;

	if (init) return;

	exit_flag = false;
	reset_flag = false;
#ifdef _DEBUG
	debug_flag = true;
#else
	debug_flag = false;
#endif
	hide_title_flag = false;
#ifdef _DEBUG
	window_flag = true; // Debug Build needs no fullscreen
#else
	window_flag = false;
#endif
	battle_test_flag = false;
	battle_test_troop_id = 0;

	engine = EngineRpg2k;

	if (argc > 1 && !strcmp(argv[1], "TestPlay")) {
		debug_flag = true;
	}
	if (argc > 2 && !strcmp(argv[2], "HideTitle")) {
		hide_title_flag = true;
	}
	if (argc > 3 && !strcmp(argv[3], "Window")) {
		window_flag = true;
	}
	if (argc > 1 && !strcmp(argv[1], "BattleTest")) {
		battle_test_flag = true;
		if (argc > 4) {
			battle_test_troop_id = atoi(argv[4]);
		} else {
			battle_test_troop_id = 0;
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
#ifdef GEKKO
	// Init libfat (Mount SD/USB)
	if (!fatInitDefault()) {
		Output::Error("Couldn't mount any storage medium!");
	}
#endif

	if (battle_test_flag) {
		Scene::Push(new Scene_Battle());
	} else if (debug_flag) {
		Scene::Push(new Scene_Title());
	} else {
		Scene::Push(new Scene_Logo());
	}

	reset_flag = false;

	// Reset frames before starting
	Graphics::FrameReset();
	
	// Main loop
	while (Scene::instance->type != Scene::Null) {
		Scene::instance->MainFunction();

		if (Scene::old_instances.size() != 0) {
			Output::Debug("Deletion stack:");
		}
		for (size_t i = 0; i < Scene::old_instances.size(); ++i) {
			Output::Debug(Scene::scene_names[Scene::old_instances[i]->type]);
			delete Scene::old_instances[i];
		}
		Scene::old_instances.clear();
	}

	Player::Exit();
}

////////////////////////////////////////////////////////////
void Player::Pause() {
	Audio::BGM_Pause();
}

////////////////////////////////////////////////////////////
void Player::Resume() {
	Input::ResetKeys();
	Audio::BGM_Resume();
	Graphics::FrameReset();
}

////////////////////////////////////////////////////////////
void Player::Update() {
	DisplayUi->ProcessEvents();

	if (exit_flag) {
		Exit();
		exit(EXIT_SUCCESS);
	} else if (reset_flag) {
		reset_flag = false;
		Scene::PopUntil(Scene::Title);
	}
}

////////////////////////////////////////////////////////////
void Player::Exit() {
	Main_Data::Cleanup();
	Graphics::Quit();
	Audio::Quit();
	delete DisplayUi;
}
