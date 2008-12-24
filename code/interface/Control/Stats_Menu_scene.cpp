#include "Stats_Menu_scene.h"

/*
Stats_Menu_Scene::~Stats_Menu_Scene()
{
}
Stats_Menu_Scene:: Stats_Menu_Scene()
{
}
*/

void Stats_Menu_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{
	myteam=TheTeam;
	myaudio=theaudio;
	Status_vit.init(200,80,120,0);
	Status.init(200,80,120,80);
	Aramas.init(200,80,120,160);
	Profesion.init(120,240,0,0);
	int i=(*myteam).select;
	int space=17;
	char stringBuffer[255];
	sprintf(stringBuffer, "Hp %d / %d", (*(*myteam).get_HP(i)), (*(*myteam).get_MaxHP(i)));
	Status_vit.add_text(stringBuffer,10,10);	
	sprintf(stringBuffer, "Mp %d / %d", (*(*myteam).get_MP(i)), (*(*myteam).get_MaxMP(i)));
	Status_vit.add_text(stringBuffer,10,10+(1*space));
	sprintf(stringBuffer, "Exp%d / %d", (*(*myteam).get_Exp(i)), (*(*myteam).get_MaxExp(i)));
	Status_vit.add_text(stringBuffer,10,10+(2*space));

	sprintf(stringBuffer, "Fuerza%d ", (*(*myteam).get_Attack(i)));
	Status.add_text(stringBuffer,10,5);
	sprintf(stringBuffer, "Defenza%d ", (*(*myteam).get_Defense(i)));
	Status.add_text(stringBuffer,10,5+(1*space));
	sprintf(stringBuffer, "Intelecto %d ", (*(*myteam).get_Spirit(i)));
	Status.add_text(stringBuffer,10,5+(2*space));
	sprintf(stringBuffer, "Agilidad  %d ", (*(*myteam).get_Speed(i)));
	Status.add_text(stringBuffer,10,5+(3*space));
	space=40;

	Profesion.add_sprite(((*myteam).get_faceset(i)),10,10);
	Profesion.add_text("Nombre",10,30+(1*space));
	Profesion.add_text(((*myteam).get_name(i)),50,30+(1*space)+16);
	Profesion.add_text("Profesión",10,30+(2*space));
	Profesion.add_text(((*myteam).get_job(i)),50,30+(2*space)+16);
	Profesion.add_text("Estado",10,30+(3*space));
	Profesion.add_text("Normal",50,30+(3*space)+16);//falta el casteo del entero
	Profesion.add_text("Nivel",10,30+(4*space));
	sprintf(stringBuffer, "%d ", (*(*myteam).get_Level(i)));

	Profesion.add_text(stringBuffer,50,30+(4*space)+16);//falta el casteo del entero
	space=14;
	Aramas.add_text(  (*((*myteam).get_Weapon(i))).get_name(),75,5);
	Aramas.add_text(  (*((*myteam).get_Shield(i))).get_name(),75,5+(1*space));
	Aramas.add_text(  (*((*myteam).get_Armor(i))).get_name(),75,5+(2*space));
	Aramas.add_text(  (*((*myteam).get_Helmet(i))).get_name(),75,5+(3*space));
	Aramas.add_text(  (*((*myteam).get_Accessory(i))).get_name(),75,5+(4*space));
	
	Aramas.add_text("Arma",10,5);
	Aramas.add_text("Escudo",10,5+(1*space));
	Aramas.add_text("Armadura",10,5+(2*space));
	Aramas.add_text("Casco",10,5+(3*space));
	Aramas.add_text("Otros",10,5+(4*space));

	retardo=0;
	running=  run;
	NScene=TheScene;

}

void Stats_Menu_Scene::update(SDL_Surface* Screen)
{
	if(retardo==0)
	{  
		Status.draw(Screen);
		Status_vit.draw(Screen);
		Profesion.draw(Screen);
		Aramas.draw(Screen);
		retardo++;
	}
}

void Stats_Menu_Scene::updatekey() 
{
	if (Key_press_and_realsed(LMK_X ))
	{
		(*myaudio).soundload("../Sound/Cansel2.wav");* NScene=4; 
	}
}

void Stats_Menu_Scene::dispose() 
{
	Status.dispose();
	Status_vit.dispose();
	Profesion.dispose();
	Aramas.dispose();
}
