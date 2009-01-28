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
    TheTeam->actual_map=TheTeam->lmt.party_map_id;
    TheTeam->actual_x_map=TheTeam->lmt.party_x;
    TheTeam->actual_y_map=TheTeam->lmt.party_y;

    std::string titles_string;
    std::string music_string;
    std::string system_string;
    titles_string.append("Title/");
    titles_string.append(TheTeam->data2.System_dat.Title_graphic);
    titles_string.append(".png");

    music_string.append("Music/");
    music_string.append(TheTeam->data2.System_dat.Title_music.Name_of_Music_Background);
    music_string.append(".mid");

    system_string.append("System/");
    system_string.append(TheTeam->data2.System_dat.System_graphic);
    system_string.append(".png");

    myteam = TheTeam;
    myaudio = theaudio;
    myaudio->load(music_string.c_str());
    title.x = 0;
    title.y = 0;
    title.setimg(titles_string.c_str());
    menu.init( myaudio, run, 0,2, 116, 57, 160 - (116 / 2), 160 - (57 / 2), (char *) system_string.c_str());
    str_Vector.push_back(TheTeam->data2.Glosary.New_Game);
    str_Vector.push_back(TheTeam->data2.Glosary.Load_Game);
    str_Vector.push_back(TheTeam->data2.Glosary.Exit_to_Windows);
    menu.setComands(&str_Vector);
    running = run;
    NScene = TheScene;
    retardo = 0;
    myaudio->play(-1);
}

void Title_Scene::update(SDL_Surface *Screen)
{
    if (retardo == 0)
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
    static bool used = false;

    if (used == false)
    {
        used = true;
        if (menu.getindexY() == 2)
        {
            *running = false;
        }
        if (menu.getindexY() == 0)
        {
            init_party();
            *NScene = 1;
        }
    }
}

void Title_Scene::init_party()
{

    //desde que aun no lemos el archivo simulamos
    string system_string;
    int i,id,start_level;
    stcHero * actual_hero;
    Player Alex;
    for(i=0;i< (myteam->data2.System_dat.Heroes_in_starting);i++)
    {
    id=myteam->data2.System_dat.vc_sh_Starting_party[i];
    actual_hero= &(myteam->data2.heros[id-1]);

    Alex.set_name(actual_hero->strName.c_str());
    Alex.set_job(actual_hero->strClass.c_str());
    start_level=actual_hero->intStartlevel;

    Faceset AlexeFase;
        system_string.clear();
        system_string.append("FaceSet/");
        system_string.append(actual_hero->strFacegraphic.c_str());
        system_string.append(".png");

    AlexeFase.setimg(system_string.c_str());
    AlexeFase.init_Faceset(0, 0, actual_hero->intFaceindex);
    Alex.set_faceset(AlexeFase);
    Alex.set_HP(actual_hero->vc_sh_Hp[start_level]);
    Alex.set_MaxHP(actual_hero->vc_sh_Hp[start_level]);
    Alex.set_MP(actual_hero->vc_sh_Mp[start_level]);
    Alex.set_MaxMP(actual_hero->vc_sh_Mp[start_level]);
    Alex.set_Heal(0);
    Alex.set_Attack(actual_hero->vc_sh_Attack[start_level]);
    Alex.set_Defense(actual_hero->vc_sh_Defense[start_level]);
    Alex.set_Speed(actual_hero->vc_sh_Agility[start_level]);
    Alex.set_Spirit(actual_hero->vc_sh_Mind[start_level]);
    Alex.set_Level(start_level);
    Alex.set_Exp(0);
    Alex.set_MaxExp(actual_hero->intEXPBaseline);
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
    Espada.id = 15;

    Animacion Myanim;
    Myanim.setimg("Battle/Sword1.png");
    Myanim.init_Anim(5, 2);


    Espada.set_anim(Myanim);


    Alex.set_Weapon(Espada);
    Item Escudo;
    Escudo.set_name("Escudo de madera");
    Escudo.set_NOI(1);
    Escudo.set_type(5);
    Escudo.id = 16;
    Item Aramadura;
    Aramadura.set_name("Aramadura de cuero");
    Aramadura.set_NOI(1);
    Aramadura.set_type(6);
    Aramadura.id = 17;

    Item Casco;
    Casco.set_name("Casco de cuero");
    Casco.set_NOI(1);
    Casco.set_type(7);
    Casco.id = 18;

    Item Talisman;
    Talisman.set_name("Talisman");
    Talisman.set_NOI(1);
    Talisman.set_type(8);
    Talisman.id = 19;


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
    nulo.id = 0;

    Item nuloarma;
    nuloarma.set_name(" ");
    nuloarma.set_NOI(0);
    nuloarma.set_type(0);
    nuloarma.id = 0;


    Myanim.setimg("Battle/Hit.png");
    Myanim.init_Anim(5, 3);

    nuloarma.set_anim(Myanim);

    myteam->add_player(Alex);

    }

    myteam->set_Gold(100);

    Item pocion;
    pocion.set_name("Pocion");
    pocion.set_NOI(10);
    pocion.set_type(0);
    pocion.id = 1;

    Item Ether;
    Ether.set_name("Ether");
    Ether.set_NOI(5);
    Ether.set_type(0);
    Ether.id = 2;

    Item Espada2;
    Espada2.set_name("Espada de madera");
    Espada2.set_NOI(1);
    Espada2.set_type(4);
    Espada2.id = 25;

    myteam->add_item(Espada2);
    myteam->add_item(pocion);
    myteam->add_item(Ether);

}
void Title_Scene::updatekey()
{
    menu.updatekey();
    if (menu.desition())
    {
        action();
    }
}
void Title_Scene::dispose()
{
    title.dispose();
    menu.dispose();
    myaudio->stop();
}
