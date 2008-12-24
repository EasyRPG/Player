#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "../sprites/sprite.h"
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include "../sprites/map.h"
#include <vector>
#include <string>
#include "item.h"


void Item::set_NOI(int The_NOI)
{
	NOI=The_NOI;
}
int* Item::get_NOI()
{
	return (&NOI);
}
void Item::set_name(const char * name)
{
	nombre=name;
}
const char * Item::get_name()
{
	return(nombre);
}

void Item::set_type(unsigned char The_type)
{
	type=The_type;
}
unsigned char* Item::get_type()
{
	return (&type);
}
void Item::set_anim(Animacion the_anim)
{
	anim=the_anim;
}
Animacion * Item::get_anim()
{
	return (& anim);
}
