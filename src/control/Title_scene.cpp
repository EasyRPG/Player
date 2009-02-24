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
    unsigned int i =0;
     char swich=0;
    int var=0;
    TheTeam->Sound_Manager.init();
    TheTeam->read_database();
    TheTeam->actual_map=TheTeam->lmt.party_map_id;
    TheTeam->actual_x_map=TheTeam->lmt.party_x;
    TheTeam->actual_y_map=TheTeam->lmt.party_y;
    TheTeam->actual_dir=2;

    TheTeam->able_to_save=1;
    TheTeam->able_to_menu=1;
    TheTeam->able_to_escape=1;
    TheTeam->able_to_teleport=1;
    TheTeam->Encounter_rate=0;

    for( i=0; (i< (TheTeam->data2.Switch_Names.size()/8));i++)
    TheTeam->world_fase.push_back(swich);
    for( i=0; (i< TheTeam->data2.Switch_Names.size());i++)
    TheTeam->world_var.push_back(var);



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
    //static bool used = false;

    //if (used == false)
    //{
      //  used = true;
        if (menu.getindexY() == 2)
        {
            *running = false;
        }
        if (menu.getindexY() == 0)
        {
            init_party();
            *NScene = 1;
        }
    //}
}

void Title_Scene::init_party()
{

    string system_string;
    int i,id;
    stcHero * actual_hero;
    for(i=0;i< (myteam->data2.System_dat.Heroes_in_starting);i++)
    {
    id=myteam->data2.System_dat.vc_sh_Starting_party[i];
    actual_hero= &(myteam->data2.heros[id-1]);
    myteam->add_player(myteam->get_hero(actual_hero, id));

    }
   /* Item potion;
    potion.set_name("Potion");
    potion.id = 1;
    myteam->add_item(potion);
    on rm2k you can not start with item
    myteam->set_Gold(100);
    myteam->add_item(Espada2);
    myteam->add_item(pocion);
    myteam->add_item(Ether);
*/
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
    menu.restarmenu();
    menu.dispose();
    myaudio->stop();
}
