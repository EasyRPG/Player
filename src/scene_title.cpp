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
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "scene_title.h"
#include "audio.h"
#include "bitmap.h"
#include "cache.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_party.h"
#include "game_player.h"
#include "game_screen.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_troop.h"
#include "game_variables.h"
#include "graphics.h"
#include "input.h"
#include "ldb_reader.h"
#include "lmt_reader.h"
#include "reader.h"
#include "main_data.h"
#include "options.h"
#include "player.h"
#include "scene_map.h"
#include "sprite.h"
#include "window_command.h"
#include "output.h"

#ifdef GEKKO
	#include <gccore.h>
#endif

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Scene_Title::Scene_Title() :
	command_window(NULL) {
	type = Scene::Title;
}

////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
void Scene_Title::MainFunction() {
	// Clear the cache when the game returns to title screen
	// e.g. by pressing F12
	Cache::Clear();

	// Load Database
	Data::Clear();

	if (!LDB_Reader::Load(DATABASE_NAME)) {
		Output::ErrorStr(Reader::GetError());
	}
	if (!LMT_Reader::Load(TREEMAP_NAME)) {
		Output::ErrorStr(Reader::GetError());
	}

	// Create Game System
	Game_System::Init();

	// Load Title Graphic
	Sprite* title = new Sprite();
	title->SetBitmap(Cache::Title(Data::system.title_name));
	
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Data::terms.new_game);
	options.push_back(Data::terms.load_game);
	options.push_back(Data::terms.exit_game);
	
	// TODO: Calculate window width from max text length from options
	command_window = new Window_Command(60, options);
	command_window->SetX(160 - command_window->GetWidth() / 2);
	command_window->SetY(224 - command_window->GetHeight());

	// Set the number of frames for the opening animation to last
	command_window->SetAnimation(32);

	// Enable load game if available
	bool continue_enabled = false;
	for (int i = 0; i < 15; i++) {
		char name[11];
		sprintf(name, "Save%2d.lsd", i);
		std::ifstream file(name);
		if (file.is_open()) {
			continue_enabled = true;
			file.close();
			break;
		}
	}
	if (continue_enabled) {
		command_window->SetIndex(1);
	} else {
		command_window->DisableItem(1);
	}

	// Play music
	Game_System::BgmPlay(Data::system.title_music);

	// Screen transition
	Graphics::Transition(Graphics::FadeIn, 30, true);

#ifdef GEKKO
	if (!Player::wii_reset_callback) {
		Player::wii_reset_callback = 1;
		SYS_SetResetCallback(Player::WiiResetPressed);
	}
#endif

	// Scene loop
	while (Scene::instance == this) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}

	// Delete graphical objects
	delete command_window;
	delete title;

	Scene::old_instance = this;
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Scene_Title::Update() {
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

////////////////////////////////////////////////////////////
/// CommandNewGame
////////////////////////////////////////////////////////////
void Scene_Title::CommandNewGame() {
	Game_System::SePlay(Data::system.decision_se);
	Audio::BGM_Stop();
	Graphics::framecount= 0;
	Game_Temp::Init();
	Main_Data::game_screen = new Game_Screen();
	Game_Actors::Init();
	Game_Party::Init();
	Main_Data::game_troop = new Game_Troop();
	Game_Map::Init();
	Main_Data::game_player = new Game_Player();
	Game_Party::SetupStartingMembers();
	Game_Map::Setup(Data::treemap.start_map_id);
	Main_Data::game_player->MoveTo(Data::treemap.start_x, Data::treemap.start_y);
	Main_Data::game_player->Refresh();
	Game_Map::Autoplay();
	Game_Map::Update();
	Scene::instance = new Scene_Map();
}

////////////////////////////////////////////////////////////
/// CommandContinue
////////////////////////////////////////////////////////////
void Scene_Title::CommandContinue() {
	// Play decision SE
	Game_System::SePlay(Data::system.decision_se);
	// Change scene
	//Main_Data::scene = new Scene_Load();
}

////////////////////////////////////////////////////////////
/// CommandShutdown
////////////////////////////////////////////////////////////
void Scene_Title::CommandShutdown() {
	Game_System::SePlay(Data::system.decision_se);
	Audio::BGS_Fade(800);
	type = Scene::Null;
	instance = NULL;
}
