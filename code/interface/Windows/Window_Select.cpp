#include "Window_Select.h"

void Window_Select::init(Audio * theaudio, bool * run,int ComandX,int ComandY,int SizeX,int SizeY,int PosX,int PosY)
{
	menu.init( theaudio,run,ComandX,ComandY);
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
	Comand_X=ComandX;
	Comand_Y=ComandY;
	visible=true;
	Size_of_Block=((Size_X-10)/(ComandX+1))-5*ComandX;
	cursor.set_surface(System.Cube_select(1,Size_of_Block,16));
	cursor.x=PosX+5;
	cursor.y=PosY+5;
	disposing=false;
	Init_text_X=10;
	on_use=true;
}

void Window_Select::add_text(const char * ctext, int x, int y)
{
	text.x=pos_X+x;
	text.y=pos_Y+y;
	text.set_surface(fuente.drawText(ctext));
	Vtext_Sprite.push_back(text);
}

void Window_Select::dispose()
{
	disposing=true;
	int i,tp;
	System.dispose();
	cursor.dispose();
	tapiz.dispose();
	//   text.dispose();

	tp=(* My_vector).size();
	for (i = 0; i < tp; i ++)
	{
		(* My_vector).pop_back();
	}
	tp=(My_Sprite).size();
	for (i = 0; i < tp; i ++)
	{
		(My_Sprite).pop_back();
	}
	tp=(Vtext_Sprite).size();
	for (i = 0; i < tp; i ++)
	{
		(Vtext_Sprite).pop_back();
	}
}

bool Window_Select::desition()
{
     return (menu.desition());
}

void Window_Select::restarmenu()
{
	menu.restarmenu();
}

int Window_Select::getindexY()
{
	return (menu.getindexY());
}

int Window_Select::getindexX()
{
	return (menu.getindexX());
}

void Window_Select::updatekey() 
{

	if(visible)
	{
		if(on_use)
		{
			menu.updatekey();
		}
	}
}

void Window_Select::set_posx_text(int x) 
{
	Init_text_X=x;
}


void Window_Select::setComands(vector <std::string> * str_Vec)
{
	My_vector=str_Vec;
	unsigned int i;
	std::string strd;
	for (i = 0; i < (*My_vector).size(); i ++)
	{
		strd = (*My_vector).at(i);
		text.set_surface(fuente.drawText(strd.c_str()));
		My_Sprite.push_back(text);
	}
}

void Window_Select::draw(SDL_Surface* Screen)
{
	unsigned int i;
	int j;
	int offset=0;
	if(!disposing)
	if(visible)
	{
		tapiz.draw(Screen);
		int Max_to_show= ((Size_Y-20)/fuente.size);//los que caben
		if(Max_to_show>Comand_Y)
		{
			Max_to_show=Comand_Y;
		}
	
		if(getindexY()>Max_to_show)
		{
			offset=getindexY()-Max_to_show;
			System.draw(Screen,25, (pos_X+ Size_X/2-8), (pos_Y+3));//flechas
			System.draw(Screen,26, (pos_X+ Size_X/2), (pos_Y+3));//flechas
		}
		else
		{
			offset=0;
		}

		if((Comand_Y!=getindexY())&&(Max_to_show<Comand_Y))
		{    

			System.draw(Screen,45, (pos_X+ Size_X/2-8), (pos_Y+Size_Y-11));//flechas
			System.draw(Screen,46, (pos_X+ Size_X/2), (pos_Y+Size_Y-11));
		}

		cursor.y=pos_Y+(getindexY()-offset)*fuente.size+5;
		cursor.x=pos_X+(getindexX())*Size_of_Block+10*getindexX()+5;
		cursor.draw(Screen);
		for (j = offset; j <= ((Max_to_show+offset+1)*(Comand_X+1)-1); j ++)  //comandos
		{
			(My_Sprite.at(j)).x=pos_X+Init_text_X;
			if(Comand_X!=0)
			{
				(My_Sprite.at(j)).x= (My_Sprite.at(j)).x+((Size_of_Block+10)*((j)%(Comand_X+1)));
				(My_Sprite.at(j)).y=((pos_Y+5)+((j-offset)/(Comand_X+1))*fuente.size);
			}
			else
			{    				
				(My_Sprite.at(j)).y=((pos_Y+5)+((j-offset)*fuente.size));
			}    
				(My_Sprite.at(j)).draw(Screen);
		}

		for (i = 0; i < (Vtext_Sprite).size(); i ++) //textoadiconal
		{
			((Vtext_Sprite).at(i)).draw(Screen);
		}

	}

}