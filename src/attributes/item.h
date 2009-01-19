/*Item.h, EasyRPG player Item class declaration file.
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
