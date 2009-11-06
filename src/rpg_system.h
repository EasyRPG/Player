#ifndef __system__
#define __system__
#include <string>
#include <vector>

#include "rpg_music.h"
#include "rpg_sound.h"
#include "rpg_testbattler.h"

namespace RPG {
	
class System {
    public:
        System();
		
		std::string boat_name;
		std::string ship_name;
		std::string airship_name;
		int boat_index;
		int ship_index;
		int airship_index;
		std::string title_name;
		std::string gameover_name;
		std::string windowskin_name;
		
		std::vector<int> party_members;
		
		Music *title_music;
		Music *battle_music;
		Music *battle_end_music;
		Music *inn_music;
		Music *boat_music;
		Music *ship_music;
		Music *airship_music;
		Music *gameover_music;
		
		Sound *cursor_se;
		Sound *decision_se;
		Sound *cancel_se;
		Sound *buzzer_se;
		Sound *battle_start_se;
		Sound *escape_se;
		Sound *enemy_collapse_se;
		Sound *enemy_attack_se;
		Sound *enemy_damaged_se;
		Sound *actor_damaged_se;
		Sound *evasion_se;
		Sound *item_use_se;
	
		int map_exit_transition;
		int map_enter_transition;
		int battle_start_fadeout;
		int battle_start_fadein;
		int battle_end_fadeout;
		int battle_end_fadein;
		
		//Message_background ???
		//Font_id ???
		//Selected_condition=0x51, ???
		//Selected_hero=0x52, ???
		
		std::string battleback_name;
		std::vector<TestBattler> test_battlers;
		
		//Times_saved=0x5B, ???

		// RPG Maker 2003
		std::string windowskin2_name;
		
		/*LDB_ID=0x0A,//exclusivo 2003
		System_graphic_2=0x14,//exclusivo 2003
		Num_Comadns_order=0x1A,// exclusivo 2003
		Comadns_order=0x1B,// exclusivo 2003 array 2 bytes
		Show_frame=0x63,	// exclusivo 2003
		In_battle_anim=0x65	// exclusivo 2003*/
};

}
#endif
