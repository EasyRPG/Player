#include "Window_Base.h"

void Window_Base::init(int SizeX,int SizeY,int PosX,int PosY)
{    
	System.init_Sistem();
	System.setimg("../System/system.png");
	tapiz.set_surface(System.Exdraw(SizeX,SizeY));
	tapiz.x=PosX;
	tapiz.y=PosY;
	fuente.init_Font();
	pos_X=PosX;
	pos_Y=PosY;
	Size_X=SizeX;
	Size_Y=SizeY;
	disposing=false;
	visible=true;
}
void Window_Base::dispose()
{    
	disposing=true;
	System.dispose();
	tapiz.dispose();
	text.dispose();
	int i,tp;
	tp=(V_Sprite).size();
	for (i = 0; i < tp; i ++)
	{	
	(V_Sprite).pop_back();
	}
	tp=(Vtext_Sprite).size();
	for (i = 0; i < tp; i ++)
	{
	(Vtext_Sprite).pop_back();
	}
}

void Window_Base::add_text(const char * ctext, int x, int y)
{
	text.x=pos_X+x;
	text.y=pos_Y+y;
	text.set_surface(fuente.drawText(ctext));
	Vtext_Sprite.push_back(text);
}

void Window_Base::add_sprite(Sprite * the_sprite, int x, int y)
{
	(*the_sprite).x=pos_X+x;
	(*the_sprite).y=pos_Y+y;
	V_Sprite.push_back((int)the_sprite);
}

void Window_Base::draw(SDL_Surface* Screen)
{
	if(visible)
	{
		if(  !disposing  )
		{
			tapiz.draw(Screen);
			unsigned int i;
			for (i = 0; i < (Vtext_Sprite).size(); i ++)
			{
				((Vtext_Sprite).at(i)).draw(Screen);
			}
			Sprite * the_sprite;
			for (i = 0; i < (V_Sprite).size(); i ++)
			{
				the_sprite=(Sprite *)((V_Sprite).at(i));
				(*the_sprite).draw(Screen);
			}
		}
	}
}
