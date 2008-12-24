#include "Skills_Menu_scene.h"
/*
Skills_Menu_Scene::~Skills_Menu_Scene()
{
}
Skills_Menu_Scene:: Skills_Menu_Scene()
{
}
*/

void Skills_Menu_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{
	myteam=TheTeam;
	myaudio=theaudio;
	int j;
	j=(*myteam).select;
	int k =(((*myteam).get_skill_size(j)-1)/2);
	menu.init( theaudio, run, 1,k, 320, 180, 0, 60);
	descripcion.init(320,30,0,0);
	descripcion2.init(320,30,0,30);

	int i;
	int space=16,Size_of_Block=150;
	char stringBuffer[255];
	sprintf(stringBuffer, " NV  %d  Normal  Hp %d / %d Mp %d / %d  ", (*(*myteam).get_Level(j)), (*(*myteam).get_HP(j)), (*(*myteam).get_MaxHP(j)), (*(*myteam).get_MP(j)), (*(*myteam).get_MaxMP(j)));
	descripcion2.add_text(((*myteam).get_name(j)),10,5);
	descripcion2.add_text(stringBuffer,80,5);

	for(i=0;i<(*myteam).get_skill_size(j);i++)
	{
		str_Vector.push_back( (const char *) ((*myteam).get_skill_name(j,i)) );
		sprintf(stringBuffer, "%d ", (*((*myteam).get_skill_mp_price(j,i))));
		menu.add_text(stringBuffer,Size_of_Block-10+((Size_of_Block+10)*((i)%(2))),5+((i/2)*space));
	}
	if(str_Vector.size()%2)//para que no truene si son nones
	{ 
		str_Vector.push_back( " " );
	}

	while(str_Vector.size()<2)//para que no truene si no hay nada
	{ 
		str_Vector.push_back( " " );
	}
	menu.setComands(& str_Vector);
	running=  run;
	NScene=TheScene;
	retardo=0;
}

void Skills_Menu_Scene::update(SDL_Surface* Screen)
{ 
	if(retardo==0)
	{
		menu.draw(Screen);
		descripcion.draw(Screen);
		descripcion2.draw(Screen);
	}
	retardo++;
	if(retardo==5)
	{
		menu.draw(Screen);
		descripcion.draw(Screen);
		retardo=1;
	}
}

void Skills_Menu_Scene::action()
{
/* if(menu.getindexY()==2)
  * running =false;
   if(menu.getindexY()==0)
    * NScene=1;     */
}

void Skills_Menu_Scene::updatekey() 
{
	menu.updatekey();
	if(menu.desition())
	{	
		action();
	}

	if (Key_press_and_realsed(LMK_X ))
	{ 
		(*myaudio).soundload("../Sound/Cansel2.wav");* NScene=4; 
	}
}

void Skills_Menu_Scene::dispose() 
{
	menu.dispose();
	descripcion.dispose();
	descripcion2.dispose();
}
