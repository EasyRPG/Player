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

void Title_Scene::init(General_data * TheTeam)
{
    unsigned int i =0;
     char swich=0;
    int var=0;
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
    TheTeam->world_fase.clear();
    TheTeam->world_var.clear();
    TheTeam->Players.clear_team();
    TheTeam->clear_obj();
    TheTeam->from_title=true;
    for( i=0; (i< (TheTeam->ldbdata->Switch_Names->size()/8));i++)
    TheTeam->world_fase.push_back(swich);
    for( i=0; (i< TheTeam->ldbdata->Switch_Names->size());i++)
    TheTeam->world_var.push_back(var);
	
    myteam = TheTeam;
    myaudio=&(TheTeam->musica);
	running=&TheTeam->running;
	NScene=&TheTeam->TheScene;

    myaudio->load(case_insensitive_and_format_msc_exist("Music/",(char *)  TheTeam->ldbdata->System_dat->Title_music.Name_of_Music_Background.c_str()));
    myteam->MBackground.x = 0;
    myteam->MBackground.y = 0;
    myteam->MBackground.trasparent_color=false;
	myteam->MBackground.setimg(case_insensitive_and_format_img_exist("Title/",(char *) TheTeam->ldbdata->System_dat->Title_graphic.c_str()));

    menu.init(TheTeam,&TheTeam->running, 0,2, 116, 57, 160 - (116 / 2), 160 - (57 / 2), (char *)case_insensitive_and_format_img_exist("System/",(char *) TheTeam->ldbdata->System_dat->System_graphic.c_str()));
    str_Vector.push_back(TheTeam->ldbdata->Glosary->New_Game);
    str_Vector.push_back(TheTeam->ldbdata->Glosary->Load_Game);
    str_Vector.push_back(TheTeam->ldbdata->Glosary->Exit_to_Windows);
    menu.setComands(&str_Vector);
    retardo = 0;
    myaudio->play(-1);
}

void Title_Scene::update(SDL_Surface *Screen)
{
    if (retardo == 0)
    {
        myteam->MBackground.draw(Screen);
        menu.draw(Screen);
    }
    retardo++;
    if(retardo==5)
    {
    menu.draw(Screen);
    retardo=1;
    myteam->screen_got_refresh=true;
    }
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
    stcHero *actual_hero;
    for(i=0;i< (myteam->ldbdata->System_dat->Heroes_in_starting);i++)
    {
        id=myteam->ldbdata->System_dat->vc_sh_Starting_party[i];
        actual_hero = myteam->ldbdata->heros->at(id-1);
        myteam->Players.add_player(myteam->Players.get_hero(actual_hero, id));
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
    myteam->MBackground.dispose();
    menu.restarmenu();
    menu.dispose();
    myaudio->stop();
    str_Vector.clear();
}
