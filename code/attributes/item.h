#ifndef ITEM_H_
#define ITEM_H_

#include "../sprites/Animacion.h"

class Item {

protected:
	const char* nombre;
	int NOI;//numero de items
	Animacion anim;//animacion de batalla
	unsigned char type;//tipo arama, recuperador, protector etc.
	int coste;//para las tiendas

public:
	int id;
	void set_NOI(int The_NOI);
	int* get_NOI();
	void set_name(const char * name);
	const char * get_name();
	void set_type(unsigned char The_type);
	unsigned char * get_type();
	void set_anim(Animacion the_anim);
	Animacion * get_anim();

};
#endif
