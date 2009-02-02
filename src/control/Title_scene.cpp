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
    TheTeam->actual_dir=2;
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

    //desde que aun no lemos el archivo simulamos
    string system_string;
    int i,id,id_skill,start_level;
    unsigned int j;
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
Alex.Skills.clear();
for (j=0;j<(actual_hero->skills.size());j++)
{
    id_skill=actual_hero->skills[j].Spell_ID-1;
    Veneno.set_name(myteam->data2.skill[id_skill].strName.c_str());
    Veneno.set_damange(myteam->data2.skill[id_skill].intBasevalue);
    Veneno.set_level_req(1);
    Veneno.set_mp_price(  myteam->data2.skill[id_skill].intCost);
    Alex.add_skill(Veneno);
}

    Alex.set_Weapon(load_item(actual_hero->sh_Weapon));
    Alex.set_Shield(load_item(actual_hero->sh_Shield));
    Alex.set_Armor(load_item(actual_hero->sh_Armor));
    Alex.set_Helmet(load_item(actual_hero->sh_Head));
    Alex.set_Accessory(load_item(actual_hero->sh_Accessory));

    Alex.set_Weapon_type(4);
    Alex.set_Shield_type(5);
    Alex.set_Armor_type(6);
    Alex.set_Helmet_type(7);
    Alex.set_Accessory_type(8);
    myteam->add_player(Alex);

    }

/* on rm2k you can not start with item
    myteam->set_Gold(100);
    myteam->add_item(Espada2);
    myteam->add_item(pocion);
    myteam->add_item(Ether);
*/
}
Item Title_Scene::load_item(int item_id)
{
Item X;
if(item_id==0)
{
    X.set_name("");
    X.set_NOI(1);
    X.set_type(0);
    X.id = 0;

}
else{
    item_id--;
    X.set_name(myteam->data2.items[item_id].Name.c_str());
    X.set_NOI(1);
    X.set_type(myteam->data2.items[item_id].Type);
    X.id = 1;
    }return(X);
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
