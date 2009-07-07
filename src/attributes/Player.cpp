/*Player.cpp, Player routines.
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

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "../sprites/sprite.h"
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include <vector>
#include <string>
#include "skill.h"
#include "item.h"
#include "Enemy.h"
#include "Player.h"



Item* Player::get_Weapon()
{
	return (&Weapon);
}
Item* Player::get_Shield()
{
	return (&Shield);
}
Item* Player::get_Armor()
{
	return (&Armor);
}
Item* Player::get_Helmet()
{
	return (&Helmet);
}
Item* Player::get_Accessory()
{
	return (&Accessory);
}


void Player::set_Weapon(Item The_Weapon)
{
	Weapon=The_Weapon;
}
void Player::set_Shield(Item The_Shield)
{
	Shield=The_Shield;
}

void Player::set_Armor(Item The_Armor)
{
	Armor=The_Armor;
}

void Player::set_Helmet(Item The_Helmet)
{
	Helmet=The_Helmet;
}
void Player::set_Accessory(Item The_Accessory)
{
	Accessory=The_Accessory;
}

unsigned char* Player::get_Weapon_type()
{

	return (&Weapon_type);
}
unsigned char* Player::get_Shield_type()
{
	return (&Shield_type);
}
unsigned char* Player::get_Armor_type()
{
	return (&Armor_type);
}
unsigned char* Player::get_Helmet_type()
{
	return (&Helmet_type);
}
unsigned char* Player::get_Accessory_type()
{
	return (&Accessory_type);
}

void Player::set_Weapon_type(unsigned char The_Weapon_type)
{
	Weapon_type=The_Weapon_type;
}
void Player::set_Shield_type(unsigned char The_Shield_type)
{
	Shield_type=The_Shield_type;
}
void Player::set_Armor_type(unsigned char The_Armor_type)
{
	Armor_type=The_Armor_type;
}
void Player::set_Helmet_type(unsigned char The_Helmet_type)
{
	Helmet_type=The_Helmet_type;
}
void Player::set_Accessory_type(unsigned char The_Accessory_type)
{
	Accessory_type=The_Accessory_type;
}
void Player::set_HP(int The_HP)
{
	HP=The_HP;
}
void Player::set_MaxHP(int The_MaxHP)
{
	MaxHP=The_MaxHP;
}
void Player::set_MP(int The_MP)
{
	MP=The_MP;
}
void Player::set_MaxMP(int The_MaxMP)
{
	MaxMP=The_MaxMP;
}

int Player::get_HP()
{
	return (HP);
}
int Player::get_MaxHP()
{
	return (MaxHP);
}
int Player::get_MP()
{
	return (MP);
}
int Player::get_MaxMP()
{
	return (MaxMP);
}

void Player::set_Heal(int The_Heal)
{
	Heal=The_Heal;
}
void Player::set_Attack(int The_Attack)
{
	Attack=The_Attack;
}
void Player::set_Defense(int The_Defense)
{
	Defense=The_Defense;
}
void Player::set_Speed(int The_Speed)
{
	Speed=The_Speed;
}
void Player::set_Spirit(int The_Spirit)
{
	Spirit=The_Spirit;
}
void Player::set_Level(int The_Level)
{
	Level=The_Level;
}
void Player::set_Exp(int The_Exp)
{
	Exp=The_Exp;
}
void Player::set_MaxExp(int The_MaxExp)
{
	MaxExp=The_MaxExp;
}

int Player::get_Heal()
{
	return (Heal);
}
int Player::get_Attack()
{
	return (Attack);
}
int Player::get_Defense()
{
	return (Defense);
}
int Player::get_Speed()
{
	return (Speed);
}

int Player::get_Spirit()
{
	return (Spirit);
}
int Player::get_Level()
{
	return (Level);
}
int Player::get_Exp()
{
	return (Exp);
}
int Player::get_MaxExp()
{
	return (MaxExp);
}


void Player::set_job(const char * name)
{
	titulo=name;
}
const char * Player::get_job()
{
	return(titulo);
}
void Player::set_name(const char * name)
{
	nombre=name;
}
const char * Player::get_name()
{
	return(nombre);
}
void Player::set_faceset(Faceset theFaceset)
{
	Face=theFaceset;
}
Faceset * Player::get_faceset()
{
	return (& Face);
}
void Player::set_chara(CActor thecharset)
{
	charset=thecharset;
}
CActor * Player::get_chara()
{
	return (& charset);
}
void Player::add_skill(Skill Myskill)
{
	Skills.push_back(Myskill);
}

const char * Player::get_skill_name(int num)
{
	return (((Skills.at(num))).get_name());
}
int * Player::get_skill_mp_price(int num)
{
	return (((Skills.at(num))).get_mp_price());
}
int * Player::get_skill_damange(int num)
{
	return (((Skills.at(num))).get_damange());
}
int * Player::get_skill_level_req(int num)
{
	return (((Skills.at(num))).get_level_req());
}

Animacion * Player::get_Weapon_Anim()
{
	return (Weapon.get_anim());
}

Animacion * Player::get_skill_get_anim(int num)
{
	return (((Skills.at(num))).get_anim());
}
int Player::get_skill_size()
{
	return (Skills.size());
}

