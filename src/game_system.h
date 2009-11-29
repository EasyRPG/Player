#ifndef __game_system__
#define __game_system__

#include <string>

class Game_System {

private:
    int windowskin_id;
    int title_id;

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

	int get_windowskin_id() const;
    void set_windowskin_id(int id);

    void set_title_id(int id);
    int get_title_id() const;

};
#endif // __game_system__
