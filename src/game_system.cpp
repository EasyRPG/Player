#include "game_system.h"
Game_System::Game_System()
{
    timer = 0;
    timer_working = false;
    save_disabled = false;
    menu_disabled = false;
    encounter_disabled = false;
    message_position = 2;
    message_frame = 0;
    save_count = 0;
    magic_number = 0; 
}

Game_System::~Game_System()
{
}

std::string Game_System::get_windowskin_name() {
	return "";
}
