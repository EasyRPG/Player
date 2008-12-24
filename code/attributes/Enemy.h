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
