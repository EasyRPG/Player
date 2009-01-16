/*Title_Scene.cpp, Title_Scene routines.
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



#include "Title_scene.h"

/*
Title_Scene::~Title_Scene()
{

}
Title_Scene::Title_Scene()
{

}
*/

void Title_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{

TheTeam->read_database();

    std::string titles_string;
    std::string music_string;
    std::string system_string;
    titles_string.append("../title/");
    titles_string.append(TheTeam->data2.System_dat.Title_graphic);
    titles_string.append(".png");

    music_string.append("../Music/");
    music_string.append(TheTeam->data2.System_dat.Title_music.Name_of_Music_Background);
    music_string.append(".mid");

    system_string.append("../System/");
    system_string.append(TheTeam->data2.System_dat.System_graphic);
    system_string.append(".png");

	myteam=TheTeam;
	myaudio=theaudio;
	(*myaudio).load(music_string.c_str());
	title.x=0;
	title.y=0;
	title.setimg(titles_string.c_str());
	menu.init( myaudio, run, 0,2, 116, 57, 160-(116/2), 160-( 57/2),(char *)system_string.c_str());
	str_Vector.push_back(TheTeam->data2.Glosary.New_Game);
	str_Vector.push_back(TheTeam->data2.Glosary.Load_Game);
	str_Vector.push_back(TheTeam->data2.Glosary.Exit_to_Windows);
	menu.setComands(& str_Vector);
	running=  run;
	NScene=TheScene;
	retardo=0;
    (*myaudio).play(-1);
}

void Title_Scene::update(SDL_Surface* Screen)
{
	if(retardo==0)
	{
		title.draw(Screen);
		menu.draw(Screen);
	}
	retardo++;
	//if(retardo==5)
	//{
		menu.draw(Screen);
	//retardo=1;
	//}
}

void Title_Scene::action()
{
	static bool used=false;
	if(used==false)
	{
		used=true;
		if(menu.getindexY()==2)
		{
			* running =false;
		}
		if(menu.getindexY()==0)
		{
			init_party();
    			* NScene=1;
		}
	}
}

void Title_Scene::init_party()
{ //desde que aun no lemos el archivo simulamos
Player Alex;
Alex.set_name("Alex");
Chara Alexchara;
Alexchara.init_Chara();

//Alexchara.setimg("../chara/protagonist1.PNG");
//Alex.set_chara(Alexchara);

Faceset AlexeFase;
AlexeFase.setimg("../Faceset/Chara1.png");
AlexeFase.init_Faceset(0,0,0);

Alex.set_faceset(AlexeFase);
Alex.set_name("Alex");
Alex.set_job("Soldado");
Alex.set_HP(48);
Alex.set_MaxHP(48);
Alex.set_MP(38);
Alex.set_MaxMP(38);
Alex.set_Heal(0);
Alex.set_Attack(27);
Alex.set_Defense(21);
Alex.set_Speed(18);
Alex.set_Spirit(23);
Alex.set_Level(1);
Alex.set_Exp(0);
Alex.set_MaxExp(33);
//las habilidades del alex que hueva.....
Skill Veneno;
Veneno.set_name("Veneno");
Veneno.set_damange(6);
Veneno.set_level_req(1);
Veneno.set_mp_price(10);
Alex.add_skill(Veneno);
Skill Paralisis;
Paralisis.set_name("Paralisis");
Paralisis.set_damange(5);
Paralisis.set_level_req(1);
Paralisis.set_mp_price(5);
Alex.add_skill(Paralisis);

Item Espada;
Espada.set_name("Espada de hierro");
Espada.set_NOI(1);
Espada.set_type(4);
Espada.id=15;

Animacion Myanim;
Myanim.setimg("../Battle/Sword1.png");
Myanim.init_Anim(5,2);


Espada.set_anim(Myanim);


Alex.set_Weapon(Espada);
Item Escudo;
Escudo.set_name("Escudo de madera");
Escudo.set_NOI(1);
Escudo.set_type(5);
Escudo.id=16;
Item Aramadura;
Aramadura.set_name("Aramadura de cuero");
Aramadura.set_NOI(1);
Aramadura.set_type(6);
Aramadura.id=17;

Item Casco;
Casco.set_name("Casco de cuero");
Casco.set_NOI(1);
Casco.set_type(7);
Casco.id=18;

Item Talisman;
Talisman.set_name("Talisman");
Talisman.set_NOI(1);
Talisman.set_type(8);
Talisman.id=19;


Alex.set_Shield(Escudo);
Alex.set_Armor(Aramadura);
Alex.set_Helmet(Casco);
Alex.set_Accessory(Talisman);

      Alex.set_Weapon_type(4);
      Alex.set_Shield_type(5);
      Alex.set_Armor_type(6);
      Alex.set_Helmet_type(7);
      Alex.set_Accessory_type(8);
Item nulo;
nulo.set_name(" ");
nulo.set_NOI(0);
nulo.set_type(0);
nulo.id=0;

Item nuloarma;
nuloarma.set_name(" ");
nuloarma.set_NOI(0);
nuloarma.set_type(0);
nuloarma.id=0;


Myanim.setimg("../Battle/Hit.png");
Myanim.init_Anim(5,3);


nuloarma.set_anim(Myanim);



(*myteam).add_player(Alex);

Player Brian;

Chara Brianchara;
Brianchara.init_Chara();
Brianchara.setimg("../chara/Chara4.png");
Brian.set_chara(Brianchara);
Faceset BrianFase;
BrianFase.setimg("../Faceset/Chara2.png");
BrianFase.init_Faceset(0,0,8);

Brian.set_faceset(BrianFase);
Brian.set_name("Brian");
Brian.set_job("Soldado");
Brian.set_HP(52);
Brian.set_MaxHP(52);
Brian.set_MP(0);
Brian.set_MaxMP(0);
Brian.set_Heal(0);
Brian.set_Attack(18);
Brian.set_Defense(21);
Brian.set_Speed(10);
Brian.set_Spirit(24);
Brian.set_Level(1);
Brian.set_Exp(0);
Brian.set_MaxExp(32);
Brian.set_Weapon(nuloarma);
Brian.set_Shield(nulo);
Brian.set_Armor(nulo);
Brian.set_Helmet(nulo);
Brian.set_Accessory(nulo);
(*myteam).add_player(Brian);


Player Caro;

Chara Carochara;
Carochara.init_Chara();
Carochara.setimg("../chara/Chara1.png");
Caro.set_chara(Carochara);
Faceset CaroFase;
CaroFase.setimg("../Faceset/Chara1.png");
CaroFase.init_Faceset(0,0,5);

Caro.set_faceset(CaroFase);
Caro.set_name("Carola");
Caro.set_job("Maga");
Caro.set_HP(36);
Caro.set_MaxHP(36);
Caro.set_MP(58);
Caro.set_MaxMP(58);
Caro.set_Heal(0);
Caro.set_Attack(18);
Caro.set_Defense(15);
Caro.set_Speed(29);
Caro.set_Spirit(22);
Caro.set_Level(1);
Caro.set_Exp(0);
Caro.set_MaxExp(31);

Item Baston;
Baston.set_name("Baston de madera");
Baston.set_NOI(1);
Baston.set_type(16);
Baston.id=16;
Myanim.setimg("../Battle/Holy.png");
Myanim.init_Anim(5,2);


Baston.set_anim(Myanim);
Caro.set_Weapon(Baston);




Caro.set_Shield(nulo);
Caro.set_Armor(nulo);
Caro.set_Helmet(nulo);
Caro.set_Accessory(nulo);
(*myteam).add_player(Caro);

Player Enrique;

Chara Enriquechara;
Enriquechara.init_Chara();
Enriquechara.setimg("../chara/Chara3.png");
Enrique.set_chara(Enriquechara);
Faceset EnriqueFase;
EnriqueFase.setimg("../Faceset/Chara2.png");
EnriqueFase.init_Faceset(0,0,10);

Enrique.set_faceset(EnriqueFase);
Enrique.set_name("Enrique");
Enrique.set_job("Luchador");
Enrique.set_HP(59);
Enrique.set_MaxHP(59);
Enrique.set_MP(0);
Enrique.set_MaxMP(0);
Enrique.set_Heal(0);
Enrique.set_Attack(28);
Enrique.set_Defense(22);
Enrique.set_Speed(26);
Enrique.set_Spirit(16);
Enrique.set_Level(1);
Enrique.set_Exp(0);
Enrique.set_MaxExp(29);
Enrique.set_Weapon(nuloarma);
Enrique.set_Shield(nulo);
Enrique.set_Armor(nulo);
Enrique.set_Helmet(nulo);
Enrique.set_Accessory(nulo);
(*myteam).add_player(Enrique);
(*myteam).set_Gold(100);

Item pocion;
pocion.set_name("Pocion");
pocion.set_NOI(10);
pocion.set_type(0);
pocion.id=1;
Item Ether;
Ether.set_name("Ether");
Ether.set_NOI(5);
Ether.set_type(0);
Ether.id=2;

Item Espada2;
Espada2.set_name("Espada de madera");
Espada2.set_NOI(1);
Espada2.set_type(4);
Espada2.id=25;
(*myteam).add_item(Espada2);
(*myteam).add_item(pocion);
(*myteam).add_item(Ether);

}
void Title_Scene::updatekey() {
	menu.updatekey();
	if(menu.desition())
	{
     		action();
	}
}
void Title_Scene::dispose() {

	title.dispose();
	menu.dispose();
	(*myaudio).stop();
}
