/*GameOver_Scene.cpp, GO_Scene routines.
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



#include "GameOver_scene.h"


/*GO_Scene::~GO_Scene()
{

}
GO_Scene:: GO_Scene()
{

}
*/

void GO_Scene::init(Audio *theaudio, bool *run, unsigned char *TheScene, General_data *TheTeam)
{
    std::string titles_string;
    std::string music_string;

    myteam = TheTeam;
    myteam->Players.clear_team();
    myteam->clear_obj();
    myaudio = theaudio;

    titles_string.append("GameOver/");
    titles_string.append(TheTeam->data2.System_dat.Game_Over_graphic);
    titles_string.append(".png");

    music_string.append("Music/");
    music_string.append(TheTeam->data2.System_dat.Game_Over_music.Name_of_Music_Background);
    music_string.append(".mid");


    myaudio->load(music_string.c_str());
    myteam->MBackground.x = 0;
    myteam->MBackground.y = 0;
    myteam->MBackground.trasparent_color=false;
    myteam->MBackground.setimg(titles_string.c_str());
    myaudio->play(-1);
    running = run;
    NScene = TheScene;
    Control::pop_action();
    Control::pop_LM();
}

void GO_Scene::update(SDL_Surface *Screen)
{
    myteam->MBackground.draw(Screen);
     myteam->screen_got_refresh=true;
}

void GO_Scene::action()
{
    *NScene = 0;
}

void GO_Scene::updatekey()
{

    int temp;

    static int delay = 0;
    delay++;

        temp =  Control::pop_action();
        switch (temp)
        {
        case DECISION:
                    if (delay >300)
                    {
                        action();
                    }
                    break;
        }
        temp = Control::pop_LM();
        switch (temp)
        {
            default:
                break;
        }

}

void GO_Scene::dispose()
{
   unsigned int i=0;
    myteam->Players.clear_team();
    myteam->clear_obj();
    for(i=0;i<myteam->GCharas_nps.size();i++)
	 myteam->GCharas_nps[i].dispose();
    myteam->GCharas_nps.clear();
    myteam->Gpre_chip.dispose();
    myteam->GEv_state.clear();
    myteam->MBackground.dispose();
    myaudio->stop();
}
