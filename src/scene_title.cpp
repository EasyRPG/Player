#include "scene_title.h"
#include "Database/ldb_reader.h"

#include <string>
#include "main_data.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"

Scene_Title::Scene_Title()
{
	Main_Data::scene_type = "Scene_Title";
}

Scene_Title::~Scene_Title()
{
}

void Scene_Title::main_function()
{	
	// Load Database
	LDB_reader::load("RPG_RT.ldb");
    
	// Create Game System
	Main_Data::game_system = new Game_System();
	
	// Load Title Graphic
/*	Bitmap title_bmp(Main_Data::data_system.title_name); */
	
	// Create Background Sprite
	Sprite background();
	
	// Create Options Window
	std::string options[3];
/*	options[0] = Main_Data::data_system.glossary.new_game;
	options[1] = Main_Data::data_system.glossary.load_game;
	options[2] = Main_Data::data_system.glossary.exit_game;*/
//	Window_Command command_window;
//	command_window = new Window_Command(116, &options);
	
	/* TODO: Disable Load Game if required */
	
	/* TODO: Set index to Load Game if required */
	
	// Play music
//	Main_Data::game_system.bgm_play(Main_Data::data_system.title_music);
	
	Graphics::transition();
	
	// Scene loop
	while(Main_Data::scene_type == "Scene_Title") {
		Graphics::update();
		Input::update();
		update();
	}
	
	// Dispose graphical objects
//	title_bmp.dispose();
//	background.dispose();
//	command_window.dispose();
}

void Scene_Title::update()
{
//	command_window.update();
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
    Main_Data::scene_type = "";
}
