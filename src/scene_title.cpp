#include "scene_title.h"
#include "ldb_reader.h"

#include <string>
#include <vector>
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "game_system.h"
#include "bitmap.h"
#include "window_command.h"
#include "rpg_cache.h"

Scene_Title::Scene_Title()
{
    int id;
	Main_Data::scene_type = SCENE_TITLE;

	/* Load Database */
	LDB_reader::load(LDB_DATABASE);

	/* Create Game System */
	Main_Data::game_system = new Game_System();

    /* Build RPG::Cache object */
    id = RPG::Cache.load_graphic(Main_Data::data_system->title_name, CCache::TITLE);
    if (id >= 0)
        Main_Data::game_system->set_title_id(id);

    id = RPG::Cache.load_graphic(Main_Data::data_system->windowskin_name, CCache::SYSTEM);
    if (id >= 0)
        Main_Data::game_system->set_windowskin_id(id);
    

	
}

Scene_Title::~Scene_Title()
{
}

void Scene_Title::main_function()
{	
	// Load Title Graphic
	Sprite* title = new Sprite();
    title->set_bitmap(
        RPG::Cache.title(Main_Data::game_system->get_title_id())
    );
	
	// Create Options Window
    std::vector<std::string> options;
    options.push_back(Main_Data::data_words->new_game);
	options.push_back(Main_Data::data_words->load_game);
	options.push_back(Main_Data::data_words->exit_game);
    /* TODO */
	command_window = new Window_Command(116, options);
	
	/* TODO: Disable Load Game if required */
	
	/* TODO: Set index to Load Game if required */
	
	// Play music
	//Main_Data::game_system.bgm_play(Main_Data::data_system.title_music);
    std::string smus(F_MUSIC + Main_Data::data_system->title_music->name);
    Audio::bgm_play(smus);

	Graphics::transition();
	
	// Scene loop
	while(Main_Data::scene_type == SCENE_TITLE) {
		Graphics::update();
		Input::update();
		update();
	}
	
	// Dispose graphical objects
//	title_bmp->dispose();
//	background->dispose();
//	command_window->dispose();
}

void Scene_Title::update()
{
	command_window->update();
    if (Input::is_triggered(Control::EXIT))
        // On [x] mouse button press, exit immediately
        Main_Data::scene_type = SCENE_NULL;

/* Will get some code frome control.cpp */
/*	if(Input::is_triggered(SDL_ENTER) || Input::is_triggered(SDL_Z))
	{
		switch(command_window.index)
		{
			case 0: // New Game
				command_new_game();
				break;
			case 1:  // Load Game
				command_continue();
				break;
			case 2:  // Exit Game
				command_shutdown();
				break;
		}
	}*/
}

void Scene_Title::command_new_game()
{
	
}

void Scene_Title::command_continue()
{
	
}

void Scene_Title::command_shutdown()
{
	// Play decision SE
//    Main_Data::game_system.se_play(Main_Data::data_system.decision_sound);
    // Fade out Music
    Audio::bgm_fade(800);
    // Shutdown
    Main_Data::scene_type = SCENE_NULL;
}
