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
#include "main_data.h"
#include "options.h"
#include "player.h"
#include "scene_map.h"
#include "sprite.h"
#include "window_command.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Scene_Title::Scene_Title() {
	Main_Data::scene_type = SCENE_TITLE;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Scene_Title::~Scene_Title() {
}

////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
void Scene_Title::MainFunction() {	
	// Load Database
	LDB_Reader::Load(DATABASE_NAME);
	LMT_Reader::Load(TREEMAP_NAME);

	// Create Game System
	Main_Data::game_system = new Game_System();

	// Load Title Graphic
	Sprite* title = new Sprite();
	title->SetBitmap(Cache::Title(Main_Data::data_system.title_name));
	
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Main_Data::data_terms.new_game);
	options.push_back(Main_Data::data_terms.load_game);
	options.push_back(Main_Data::data_terms.exit_game);

	command_window = new Window_Command(116, options);
	command_window->SetX(160 - command_window->GetWidth() / 2);
	command_window->SetY(224 - command_window->GetHeight());

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
	Main_Data::game_system->BgmPlay(Main_Data::data_system.title_music);

	// Screen transition
	//Graphics::transition();
	
	// Scene loop
	while (Main_Data::scene_type == SCENE_TITLE) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}
	
	// Delete graphical objects
	delete command_window;
	delete title;
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
	Main_Data::game_system->SePlay(Main_Data::data_system.decision_se);
	Audio::BGM_Stop();
	Graphics::framecount= 0;
	Main_Data::game_temp = new Game_Temp();
	Main_Data::game_system = new Game_System();
	Main_Data::game_switches = new Game_Switches();
	Main_Data::game_variables = new Game_Variables();
	Main_Data::game_screen = new Game_Screen();
	Main_Data::game_actors = new Game_Actors();
	Main_Data::game_party = new Game_Party();
	Main_Data::game_troop = new Game_Troop();
	Main_Data::game_map = new Game_Map();
	Main_Data::game_player = new Game_Player();
	Main_Data::game_party->SetupStartingMembers();
	Main_Data::game_map->Setup(Main_Data::data_treemap.start_map_id);
	Main_Data::game_player->MoveTo(Main_Data::data_treemap.start_x, Main_Data::data_treemap.start_y);
	//Main_Data::game_player->Refresh();
	Main_Data::game_map->Autoplay();
	Main_Data::game_map->Update();
	Main_Data::scene = new Scene_Map();
}

////////////////////////////////////////////////////////////
/// CommandContinue
////////////////////////////////////////////////////////////
void Scene_Title::CommandContinue() {
	// Play decision SE
	Main_Data::game_system->SePlay(Main_Data::data_system.decision_se);
	// Change scene
	//Main_Data::scene = new Scene_Load();
}

////////////////////////////////////////////////////////////
/// CommandShutdown
////////////////////////////////////////////////////////////
void Scene_Title::CommandShutdown() {
	// Play decision SE
	Main_Data::game_system->SePlay(Main_Data::data_system.decision_se);
	// Fade out Music
	Audio::BGS_Fade(800);
	// Shutdown
	Main_Data::scene_type = SCENE_NULL;
}
