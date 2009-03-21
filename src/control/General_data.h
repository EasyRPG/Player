#ifndef EVENT_MANAGEMENT_H_
#define EVENT_MANAGEMENT_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include "../sprites/sprite.h"
#include "../sprites/Animation_Manager.h"
#include "../sprites/chipset.h"
#include "../sprites/Pre_Chipset.h"
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include "../readers/map.h"
#include "../readers/lmt.h"
#include "../readers/ldb.h"
#include <vector>
#include <string>
#include "../interface/Windows/Window_Base.h"
#include "../attributes/skill.h"
#include "../attributes/item.h"
#include "../attributes/Enemy.h"
#include "../attributes/Player_Team.h"
#include "../tools/math-sll.h"
#include "../attributes/CActor.h"
#include "events/message.h"
#include "Move_management.h"
#include "Event_management.h"
class E_management;

class General_data {
    private:
	std:: vector <Item> Items;
	int Gold;

	public:

    General_data();
	bool running;
    unsigned char TheScene;
    Music musica;

    Player_Team Players;
	Sound_Manager S_manager;
	std:: vector <Chara> GCharas_nps;
    Pre_Chipset Gpre_chip;
	Chipset Gchip;
    map_data Gdata;
    E_management GEv_management;
    std:: vector <E_state> GEv_state;
    std:: vector <E_state> GEvc_state;
    std:: vector <E_state> GEDinamic_states;

    Sprite MBackground;
    string memorized_BGM;
    std:: vector <char> world_fase;
    bool is_on_the_inventory(int id);
    bool is_equal(int var,int number);
    void add_swich(unsigned char i);
    bool state_swich( int number);
    void set_true_swich( int number);
    void set_false_swich( int number);

	std:: vector <int> world_var;


    bool scroll_active;
    bool scroll_writed;
    bool from_title;

    int original_scroll_x;
    int original_scroll_y;

    int actual_x_map;
    int actual_y_map;
    int actual_dir;
    int actual_map;

    int able_to_save;
    int able_to_menu;
    int able_to_escape;

    int ecape_x_map;
    int ecape_y_map;
    int escape_map;
    bool ecape_active_fase;
    bool screen_got_refresh;
    int ecape_num_fase;
    bool ecape_use_default;
    int able_to_teleport;
    int Encounter_rate;

    LDB_data data2;
    lmt_data lmt;
	void read_database();
	std:: vector <Enemy> Enemys;
	void clear_enemy();
	void add_enemy(Enemy Myplayer);
	void load_group(int id);

	View view;
	int select;
	void set_Gold(int The_Gold);
	int get_Gold();
	void erase_item(int num);
	Item get_item(int num);
	Item load_item(int item_id);
	int get_num_items();
	void change_objets(int remove_add,int item_id,int cout);
    void clear_obj();
	void add_item(Item Myitem);
	int* get_NOI(int num);
	unsigned char * get_type(int num);
	const char * get_item_name(int num);
	Animacion * get_item_anim(int num);

};


#endif
