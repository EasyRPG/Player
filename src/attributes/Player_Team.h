/*Player_Team.h, EasyRPG player Player_Team class declaration file.
    Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef PLAYER_TEAM_H_
#define PLAYER_TEAM_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "../sprites/sprite.h"
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include "../tools/Sound_Manager.h"
#include <vector>
#include <string>
#include "skill.h"
#include "item.h"
#include "Enemy.h"
#include "CActor.h"
#include "Player.h"
#include "../readers/ldb.h"
#include "../readers/lmt.h"
#include "../sprites/chipset.h"
#include "../sprites/Pre_Chipset.h"

struct E_state
{

    bool Event_Active;
    unsigned int id_exe_actual;
    bool id_actual_active;
    int Active_page;

};

class Player_Team {

public:
	std:: vector <Player> Players;
   LDB_data * data2;
    bool is_on_the_team(int id);
    Player get_hero(stcHero * actual_hero, int id);
    void change_players(int remove_add,int id);
    int get_xp_for_level(int Level,int star_exp, int addIncrease, int correction);
    void change_exp(int add_remove,int Hero_ID, int count);
    void change_level(int add_remove,int Hero_ID, int count);
    void change_stats(int add_remove,int Hero_ID, int count,int type);
    void change_skills(int Learn,int Hero_id,int skill_id);
    void change_HP(int Add,int Hero_id,int count);
    void change_MP(int Add,int Hero_id,int count);
    void Full_Recovery(int Hero_id);
    void change_name(int Hero_ID,const char * name);
    void change_class(int Hero_ID,const char * name);
    void change_graphic(int Hero_ID,const char * name,int id);
    void change_face(int Hero_ID,const char * name,int id);
    bool hero_has_skill(int hero_static_id,int id);

	int get_size();
	void clear_team();
	void clear_obj();
	void add_player(Player Myplayer);

	CActor * get_chara(int num);
	Faceset * get_faceset(int num);
	const char * get_name(int num);
	const char * get_job(int num);

	int get_MP(int num);
	int get_MaxMP(int num);
	int get_HP(int num);
	void set_HP(int num,int hp);
	int get_MaxHP(int num);

	int get_Heal(int num);
	int get_Attack(int num);
	int get_Defense(int num);
	int get_Speed(int num);
	int get_Spirit(int num);
	int get_Level(int num);
	int get_Exp(int num);
	int get_MaxExp(int num);


	int get_skill_size(int num);
	const char * get_skill_name(int nump,int num);
	int * get_skill_mp_price(int nump,int num);
	int * get_skill_damange(int nump,int num);
	int * get_skill_level_req(int nump,int num);
	Animacion * get_skill_get_anim(int nump,int num);

	Animacion * get_Weapon_Anim(int num);

	Item* get_Weapon(int num);
	Item* get_Shield(int num);
	Item* get_Armor(int num);
	Item* get_Helmet(int num);
	Item* get_Accessory(int num);

	void set_Weapon(int nump,Item The_Weapon);
	void set_Shield(int nump,Item The_Shield);
	void set_Armor(int nump,Item The_Armor);
	void set_Helmet(int nump,Item The_Helmet);
	void set_Accessory(int nump,Item The_Accessory);

	unsigned char* get_Weapon_type(int nump);
	unsigned char* get_Shield_type(int nump);
	unsigned char* get_Armor_type(int nump);
	unsigned char* get_Helmet_type(int nump);
	unsigned char* get_Accessory_type(int nump);
	void set_Weapon_type(int nump, unsigned char The_Weapon_type);
	void set_Shield_type(int nump, unsigned char The_Shield_type);
	void set_Armor_type(int nump, unsigned char The_Armor_type);
	void set_Helmet_type(int nump, unsigned char The_Helmet_type);
	void set_Accessory_type(int nump, unsigned char The_Accessory_type);

};

#endif
