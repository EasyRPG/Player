/*skill.h, EasyRPG player Skill class declaration file.
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
