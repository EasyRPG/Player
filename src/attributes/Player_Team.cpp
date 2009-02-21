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
   //my_lmt.print(&lmt);
 //   my_ldb.ShowInformation(&data2);
}
void Player_Team::add_swich(unsigned char i)
{
	world_fase.push_back(i);
}

bool Player_Team::is_on_the_inventory(int id)
{
unsigned int i;
for(i=0;i<Items.size();i++)
{
if(Items[i].id==id)
{
    return true;
}
}
return false;
}

bool Player_Team::is_on_the_team(int id)
{
unsigned int i;
for(i=0;i<Players.size();i++)
{
if(Players[i].id==id)
{
    return true;
}
}
return false;
}


bool Player_Team::is_equal(int var,int number)
{
if(world_var[var-1]==number)
return true;
return false;
}

bool Player_Team::state_swich( int number)
{
int unsigned real_id, position;
unsigned char state,temp;
number--;
real_id =(unsigned int)(number/8);
position= (number%8);
if(real_id<world_fase.size())
state=world_fase[real_id];
else
return(false);
temp=(1<<position);
return( state&temp );
}

void Player_Team::set_true_swich( int number)
{
number--;
int real_id, position;
real_id = (number/8);
position= (number%8);
world_fase[real_id]=(world_fase[real_id]|(1<<(position)));
}

void Player_Team::set_false_swich( int number)
{
number--;
int real_id, position;
real_id = (number/8);
position= (number%8);
world_fase[real_id]=(world_fase[real_id]&(!(1<<position)));
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
    if(!is_on_the_team(Myplayer.id))
	Players.push_back(Myplayer);
}
Item Player_Team::get_item(int num)
{
	return (Items.at(num));
}

Item Player_Team::load_item(int item_id)
{
Item X;
if(item_id==0)
{
    X.set_name("");
    X.set_NOI(1);
    X.set_type(0);
    X.id = 0;

}
else{
    item_id--;
    X.set_name(data2.items[item_id].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2.items[item_id].Type);
    X.id = 1;
    }return(X);
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
void Player_Team::change_players(int remove_add,int id)
{
unsigned int i;

    if(remove_add)
    {
        for(i=0;i<Players.size();i++)
        {
            if(Players[i].id==id)
            {
                Players.erase(Players.begin()+i);
            }
        }
    }
    else
    {
        stcHero * actual_hero;
        actual_hero= &(data2.heros[id-1]);
        add_player(get_hero(actual_hero, id));
    }
}


Player Player_Team::get_hero(stcHero * actual_hero,int id)
{
    int start_level,id_skill;
    unsigned j;
    string system_string;

    Player Alex;
    Alex.id=id;
    Alex.set_name(actual_hero->strName.c_str());
    Alex.set_job(actual_hero->strClass.c_str());
    start_level=actual_hero->intStartlevel;

    Faceset AlexeFase;
    system_string.clear();
    system_string.append("FaceSet/");
    system_string.append(actual_hero->strFacegraphic.c_str());
    system_string.append(".png");

    AlexeFase.setimg(system_string.c_str());
    AlexeFase.init_Faceset(0, 0, actual_hero->intFaceindex);
    Alex.set_faceset(AlexeFase);
    Alex.set_HP(actual_hero->vc_sh_Hp[start_level]);
    Alex.set_MaxHP(actual_hero->vc_sh_Hp[start_level]);
    Alex.set_MP(actual_hero->vc_sh_Mp[start_level]);
    Alex.set_MaxMP(actual_hero->vc_sh_Mp[start_level]);
    Alex.set_Heal(0);
    Alex.set_Attack(actual_hero->vc_sh_Attack[start_level]);
    Alex.set_Defense(actual_hero->vc_sh_Defense[start_level]);
    Alex.set_Speed(actual_hero->vc_sh_Agility[start_level]);
    Alex.set_Spirit(actual_hero->vc_sh_Mind[start_level]);
    Alex.set_Level(start_level);
    Alex.set_Exp(0);
    Alex.set_MaxExp(actual_hero->intEXPBaseline);
//las habilidades del alex que hueva.....
   Skill Veneno;
Alex.Skills.clear();
for (j=0;j<(actual_hero->skills.size());j++)
{
    id_skill=actual_hero->skills[j].Spell_ID-1;
    Veneno.set_name(data2.skill[id_skill].strName.c_str());
    Veneno.set_damange(data2.skill[id_skill].intBasevalue);
    Veneno.set_level_req(1);
    Veneno.set_mp_price(data2.skill[id_skill].intCost);
    Alex.add_skill(Veneno);
}

    Alex.set_Weapon(load_item(actual_hero->sh_Weapon));
    Alex.set_Shield(load_item(actual_hero->sh_Shield));
    Alex.set_Armor(load_item(actual_hero->sh_Armor));
    Alex.set_Helmet(load_item(actual_hero->sh_Head));
    Alex.set_Accessory(load_item(actual_hero->sh_Accessory));

    Alex.set_Weapon_type(4);
    Alex.set_Shield_type(5);
    Alex.set_Armor_type(6);
    Alex.set_Helmet_type(7);
    Alex.set_Accessory_type(8);
    return(Alex);
}

void Player_Team::change_objets(int remove_add,int item_id,int cout)
{
    Item X;
    item_id--;
    X.set_name(data2.items[item_id].Name.c_str());
    X.set_NOI(cout);
    X.set_type(data2.items[item_id].Type);
    X.id = 1;

    if(remove_add)
        erase_item(item_id);
    else
        add_item(X);

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
int Player_Team::get_Gold()
{
	return (Gold);
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
