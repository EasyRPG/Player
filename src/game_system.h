#ifndef __game_system__
#define __game_system__

#include <string>

class Game_System {

public:
    int timer;
    bool timer_working;
    bool save_disabled;
    bool menu_disabled;
    bool encounter_disabled;
    int message_position;
    int message_frame;
    int save_count;
    int magic_number;

	Game_System();
	~Game_System();

	std::string get_windowskin_name();
};
#endif // __game_system__
