/*Enemy.h, EasyRPG player Enemy class declaration file.
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

#ifndef ENEMY_H_
#define ENEMY_H_

class Enemy {
    
private: 
	int HP;
	int MaxHP;
	int MP;
	int MaxMP;
	int Heal;
	int Attack;
	int Defense;
	int Speed;
	int Spirit;
	int Level;
	int Exp;//esperiencia a dar
	int Goldto;//oro a dar
	const char * nombre;    

public: 
	Sprite Batler;
	std:: vector <Skill> Skills;//skills del moustruo
	std:: vector <Item> Equip;// items a dar     
	void set_HP(int The_HP);
	void set_MaxHP(int The_MaxHP);
	void set_MP(int The_MP);
	void set_MaxMP(int The_MaxMP);
	int * get_HP();
	int * get_MaxHP();
	int* get_MP();
	int* get_MaxMP();
	void set_Heal(int The_Heal);
	void set_Attack(int The_Attack);
	void set_Defense(int The_Defense);
	void set_Speed(int The_Speed);
	void set_Spirit(int The_Spirit);
	void set_Level(int The_Level);
	void set_Exp(int The_Exp);
	int * get_Heal();
	int * get_Attack();
	int* get_Defense();
	int* get_Speed();
	int * get_Spirit();
	int * get_Level();
	int* get_Exp();
	int* get_MaxExp();
	void set_name(const char * name);
	const char * get_name();
	void add_skill(Skill Myskill);
	const char * get_skill_name(int num);
	int * get_skill_mp_price(int num);
	int * get_skill_damange(int num);
	int * get_skill_level_req(int num);
	Animacion * get_skill_get_anim(int num);
	int get_skill_size();
	     
};

#endif 
