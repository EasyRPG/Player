#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include "../sprites/map.h"
#include <vector>
#include <string>
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