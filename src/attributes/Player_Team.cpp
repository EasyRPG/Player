/*Player_Team.cpp, Player_Team routines.
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

#include "Player_Team.h"

void Player_Team::read_database()
{
    LDB_reader my_ldb;
    my_ldb.Load("RPG_RT.ldb",&data2);
   lmt_reader my_lmt;
   my_lmt.load("RPG_RT.lmt",&lmt);
   my_lmt.print(&lmt);
   // my_ldb.ShowInformation(&data2);
}

void Player_Team::clear_team()
{
	Players.clear();
}
void Player_Team::clear_obj()
{
	Items.clear();
}
void Player_Team::clear_enemy()
{
	Enemys.clear();
}
void Player_Team::add_enemy(Enemy Myplayer)
{
	Enemys.push_back(Myplayer);
}

void Player_Team::add_player(Player Myplayer)
{
	Players.push_back(Myplayer);
}
Item Player_Team::get_item(int num)
{
	return (Items.at(num));
}
void Player_Team::add_item(Item Myitem)
{

	unsigned int i;
	int the_id;
	the_id=Myitem.id;
	for (i=0;i<Items.size();i++)
    	{
		if(the_id ==(Items.at(i)).id)
		{
			break;
		}
	}
	if (i<Items.size())//lo encontro
	{
		*((Items.at(i)).get_NOI())= *((Items.at(i)).get_NOI())+*Myitem.get_NOI();
    	}
	else
	{
	Items.push_back(Myitem);
	}
}



int Player_Team::get_size()
{
	return (Players.size());
}
Chara * Player_Team::get_chara(int num)
{
	return (((Players.at(num))).get_chara());
}
Faceset * Player_Team::get_faceset(int num)
{
	return (((Players.at(num))).get_faceset());
}
const char * Player_Team::get_name(int num)
{
	return (((Players.at(num))).get_name());
}
const char * Player_Team::get_job(int num)
{
	return (((Players.at(num))).get_job());
}
int * Player_Team::get_HP(int num)
{
	return (((Players.at(num))).get_HP());
}
int * Player_Team::get_MaxHP(int num)
{
	return (((Players.at(num))).get_MaxHP());
}
int* Player_Team::get_MP(int num)
{
	return (((Players.at(num))).get_MP());
}
int* Player_Team::get_MaxMP(int num)
{
	return (((Players.at(num))).get_MaxMP());
}

int * Player_Team::get_Heal(int num)
{
	return (((Players.at(num))).get_Heal());
}
int * Player_Team::get_Attack(int num)
{
	return (((Players.at(num))).get_Attack());
}
int* Player_Team::get_Defense(int num)
{
	return (((Players.at(num))).get_Defense());
}
int* Player_Team::get_Speed(int num)
{
	return (((Players.at(num))).get_Speed());
}

int * Player_Team::get_Spirit(int num)
{
	return (((Players.at(num))).get_Spirit());
}
int * Player_Team::get_Level(int num)
{
	return (((Players.at(num))).get_Level());
}
int* Player_Team::get_Exp(int num)
{
	return (((Players.at(num))).get_Exp());
}
int* Player_Team::get_MaxExp(int num)
{
	return (((Players.at(num))).get_MaxExp());
}

int Player_Team::get_num_items()
{
	return (Items.size());
}
void Player_Team::erase_item(int the_item)
{
	vector<Item>::iterator the_iterator;
	the_iterator=Items.begin();
	int i;
	for(i=0;i<the_item;i++)
	{
		the_iterator++;
	}
	Items.erase( the_iterator);
}
int* Player_Team::get_NOI(int num)
{
	return (((Items.at(num))).get_NOI());
}
unsigned char * Player_Team::get_type(int num)
{
	return (((Items.at(num))).get_type());
}
const char * Player_Team::get_item_name(int num)
{
	return (((Items.at(num))).get_name());
}
Animacion * Player_Team::get_item_anim(int num)
{
	return (((Items.at(num))).get_anim());
}
void Player_Team::set_Gold(int The_Gold)
{
	Gold=The_Gold;
}
int* Player_Team::get_Gold()
{
	return (&Gold);
}
int Player_Team::get_skill_size(int num)
{
	return (((Players.at(num))).get_skill_size());
}
const char * Player_Team::get_skill_name(int nump,int num)
{
	return (((Players.at(nump))).get_skill_name(num));
}
int * Player_Team::get_skill_mp_price(int nump,int num)
{
	return (((Players.at(nump))).get_skill_mp_price(num));
}
int * Player_Team::get_skill_damange(int nump,int num)
{
	return (((Players.at(nump))).get_skill_damange(num));
}
int * Player_Team::get_skill_level_req(int nump,int num)
{
	return (((Players.at(nump))).get_skill_level_req(num));
}

Animacion * Player_Team::get_skill_get_anim(int nump,int num)
{
	return (((Players.at(nump))).get_skill_get_anim(num));
}
Animacion * Player_Team::get_Weapon_Anim(int nump)
{
	return (((Players.at(nump))).get_Weapon_Anim());
}

Item* Player_Team::get_Weapon(int nump)
{
	return ((Players.at(nump)).get_Weapon());
}
Item* Player_Team::get_Shield(int nump)
{
	return ((Players.at(nump)).get_Shield());
}
Item* Player_Team::get_Armor(int nump)
{
	return ((Players.at(nump)).get_Armor());
}
Item* Player_Team::get_Helmet(int nump)
{
	return ((Players.at(nump)).get_Helmet());
}
Item* Player_Team::get_Accessory(int nump)
{
	return ((Players.at(nump)).get_Accessory());
}

void Player_Team::set_Weapon(int nump,Item The_Weapon)
{
	(Players.at(nump)).set_Weapon(The_Weapon);
}
void Player_Team::set_Shield(int nump,Item The_Shield)
{
	(Players.at(nump)).set_Shield(The_Shield);
}
void Player_Team::set_Armor(int nump,Item The_Armor)
{
	(Players.at(nump)).set_Armor(The_Armor);
}
void Player_Team::set_Helmet(int nump,Item The_Helmet)
{
	(Players.at(nump)).set_Helmet(The_Helmet);
}
void Player_Team::set_Accessory(int nump,Item The_Accessory)
{
	(Players.at(nump)).set_Accessory(The_Accessory);
}


unsigned char* Player_Team::get_Weapon_type(int nump)
{
	return ( (Players.at(nump)).get_Weapon_type());
}
unsigned char* Player_Team::get_Shield_type(int nump)
{
	return ( (Players.at(nump)).get_Shield_type());
}
unsigned char* Player_Team::get_Armor_type(int nump)
{
	return ( (Players.at(nump)).get_Armor_type());
}
unsigned char* Player_Team::get_Helmet_type(int nump)
{
	return ( (Players.at(nump)).get_Helmet_type());
}
unsigned char* Player_Team::get_Accessory_type(int nump)
{
	return ( (Players.at(nump)).get_Accessory_type());
}

void Player_Team::set_Weapon_type(int nump, unsigned char The_Weapon_type)
{
	(Players.at(nump)).set_Weapon_type(The_Weapon_type);
}
void Player_Team::set_Shield_type(int nump, unsigned char The_Shield_type)
{
	(Players.at(nump)).set_Shield_type(The_Shield_type);
}
void Player_Team::set_Armor_type(int nump, unsigned char The_Armor_type)
{
	(Players.at(nump)).set_Armor_type(The_Armor_type);
}
void Player_Team::set_Helmet_type(int nump, unsigned char The_Helmet_type)
{
	(Players.at(nump)).set_Helmet_type(The_Helmet_type);
}
void Player_Team::set_Accessory_type(int nump, unsigned char The_Accessory_type)
{
	(Players.at(nump)).set_Accessory_type(The_Accessory_type);
}
