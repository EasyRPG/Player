#ifndef SKILL_H_
#define SKILL_H_

#include "../sprites/Animacion.h"

class Skill {

protected:
	const char* nombre;
	int mp_price;//numero mp a usar
	Animacion anim;//animacion de batalla
	int damange;
	int level_req;

public:

	void set_name(const char * name);
	const char * get_name();
	void set_anim(Animacion the_anim);
	Animacion * get_anim();
	void set_mp_price(int The_mp_price);
	int* get_mp_price();
	void set_damange(int The_damange);
	int* get_damange();
	void set_level_req(int The_level_req);
	int* get_level_req();

};
#endif
