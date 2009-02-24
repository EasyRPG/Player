/*skill.cpp, Skill routines.
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

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "../tools/key.h"
#include "../tools/font.h"
#include <string>
//#include "../tools/audio.h"
#include <vector>
#include "skill.h"


void Skill::set_name(const char * name)
{
	nombre=name;
}
const char * Skill::get_name()
{
	return(nombre);
}
void Skill::set_anim(Animacion the_anim)
{
	anim=the_anim;
}
Animacion * Skill::get_anim()
{
	return(& anim);
}

void Skill::set_mp_price(int The_mp_price)
{
	mp_price=The_mp_price;
}
int* Skill::get_mp_price()
{
	return (&mp_price);
}
void Skill::set_damange(int The_damange)
{
	damange=The_damange;
}
int* Skill::get_damange()
{
	return (&damange);
}
void Skill::set_level_req(int The_level_req)
{
	level_req=The_level_req;
}
int* Skill::get_level_req()
{
	return (&level_req);
}
