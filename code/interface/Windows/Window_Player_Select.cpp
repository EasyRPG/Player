
#include"Window_Player_Select.h"

voidWindow_Player_Select::init(Audio*theaudio,bool*run,intComandX,intComandY,intSizeX,intSizeY,intPosX,intPosY,intcurX,intcurY)
{
	menu.init(theaudio,run,ComandX,ComandY);
	System.init_Sistem();
	System.setimg("../System/system.png");
	tapiz.set_surface(System.Exdraw(SizeX,SizeY));
	tapiz.x=PosX;
	tapiz.y=PosY;
	fuente.init_Font();
	Size_of_Block=((Size_X-10)/(ComandX+1))-5*ComandX;
	cursor.set_surface(System.Cube_select(1,curX,curY));
	Moves_cursorY=curY+12;
	cursor.x=PosX+5;
	cursor.y=PosY+5;
	disposing=false;
	pos_X=PosX;
	pos_Y=PosY;
	Size_X=SizeX;
	Size_Y=SizeY;
	Comand_X=ComandX;
	Comand_Y=ComandY;
	Cur_pos_X=5;
	Cur_pos_Y=5;
	visible=false;//delcursor
	visible_window=true;//delaventana
}

voidWindow_Player_Select::init_curXY(intx,inty)
{
	Cur_pos_X=x;
	Cur_pos_Y=y;
}

voidWindow_Player_Select::set_curY(inty)
{
	Cur_pos_Y=y;
}
voidWindow_Player_Select::add_text(constchar*ctext,intx,inty)
{

	text.x=pos_X+x;
	text.y=pos_Y+y;
	text.set_surface(fuente.drawText(ctext));
	Vtext_Sprite.push_back(text);

}

voidWindow_Player_Select::dispose()
{
	disposing=true;
	inti,tp;
	System.dispose();
	cursor.dispose();
	tapiz.dispose();
	tp=(V_Sprite).size();
	for(i=0;i<tp;i++)
	{
		(V_Sprite).pop_back();
	}
	tp=(Vtext_Sprite).size();
	for(i=0;i<tp;i++)
	{
		(Vtext_Sprite).pop_back();
	}
	restarmenu();
}

boolWindow_Player_Select::desition()
{
	return(menu.desition());
}

voidWindow_Player_Select::restarmenu()
{
	menu.restarmenu();
}

intWindow_Player_Select::getindexY()
{
	return(menu.getindexY());
}

intWindow_Player_Select::getindexX()
{
	return(menu.getindexX());
}

voidWindow_Player_Select::updatekey()
{
	if(visible)
	{	
		menu.updatekey();
	}
}

voidWindow_Player_Select::add_sprite(Sprite*the_sprite,intx,inty)
{
	(*the_sprite).x=pos_X+x;
	(*the_sprite).y=pos_Y+y;
	V_Sprite.push_back((int)the_sprite);
}

voidWindow_Player_Select::draw(SDL_Surface*Screen)
{
	unsignedinti;
	intoffset=0;
	if(visible_window)
	if(!disposing)
	{
		tapiz.draw(Screen);
		intMax_to_show=((Size_Y-20)/fuente.size);//losquecaben
		if(Max_to_show>Comand_Y)
		{
			Max_to_show=Comand_Y;
		}
		if(getindexY()>Max_to_show)
		{
			offset=getindexY()-Max_to_show;
			System.draw(Screen,25,(pos_X+Size_X/2-8),(pos_Y+3));//flechas
			System.draw(Screen,26,(pos_X+Size_X/2),(pos_Y+3));//flechas
		}
		else
		{
			offset=0;
		}

		if((Comand_Y!=getindexY())&&(Max_to_show<Comand_Y))
		{	
			System.draw(Screen,45,(pos_X+Size_X/2-8),(pos_Y+Size_Y-11));//flechas
			System.draw(Screen,46,(pos_X+Size_X/2),(pos_Y+Size_Y-11));
		}

		if(visible)
		{
			cursor.y=pos_Y+(getindexY()-offset)*Moves_cursorY+Cur_pos_Y;
			cursor.x=pos_X+(getindexX())*Size_of_Block+10*getindexX()+Cur_pos_X;
			cursor.draw(Screen);
		}

		for(i=0;i<(Vtext_Sprite).size();i++)//textoadicional
		{
			((Vtext_Sprite).at(i)).draw(Screen);
		}

		Sprite*the_sprite;//dibujodeimagenes
		for(i=0;i<(V_Sprite).size();i++)
		{
			the_sprite=(Sprite*)((V_Sprite).at(i));
			(*the_sprite).draw(Screen);
		}

	}
}
