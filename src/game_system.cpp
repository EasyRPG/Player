#include "game_system.h"
Game_System::Game_System():
    windowskin_id(0),
    title_id(0),
    timer(0),
    timer_working(false),
    save_disabled(false),
    menu_disabled(false),
    encounter_disabled(false),
    message_position(2),
    message_frame(0),
    save_count(0),
    magic_number(0)
{

}

Game_System::~Game_System()
{
}

int Game_System::get_title_id() const 
{
	return title_id;
}

void Game_System::set_title_id(int id) 
{
	title_id = id;
}

int Game_System::get_windowskin_id() const 
{
	return windowskin_id;
}

void Game_System::set_windowskin_id(int id) 
{
	windowskin_id = id;
}
