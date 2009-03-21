/*Map_scene.cpp, Map_Scene routines.
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

#include "Map_scene.h"


void Map_Scene::init( int SCREEN_X, int SCREEN_Y,General_data *TheTeam)
{
    myteam = TheTeam;
    SCREEN_SIZE_X = SCREEN_X;
    SCREEN_SIZE_Y = SCREEN_Y;
    myaudio=&(TheTeam->musica);
	running=&TheTeam->running;
	NScene=&TheTeam->TheScene;

    Actor = myteam->Players.get_chara(0);
    Charas_nps= &(myteam->GCharas_nps);
    pre_chip=&(myteam->Gpre_chip);
    chip=&(myteam->Gchip);
    data=&(myteam->Gdata);
    Background= &(myteam->MBackground);
    Ev_state= &(myteam->GEv_state);
    Evc_state= &(myteam->GEvc_state);
    Ev_management= &(myteam->GEv_management);
    load_map();
    myteam->scroll_active=true;
    Ev_management->init(myaudio,NScene,myteam,Events,Actor,&Mov_management);
    Mov_management.init(myteam);
}


void Map_Scene::load_map()
{
    static int Map_id=-1;
    if((Map_id!=myteam->actual_map)||(myteam->from_title))
    {
    if(!myteam->from_title)
    {
    Ev_management->dispose();
    }
    myteam->from_title=false;
    Ev_state->clear();
    data->clear_events();
    pre_chip->dispose();

        Map_id=  myteam->actual_map;
        actual_map=Map_id;
        myteam->view.x = 0;
        myteam->view.y = 0;
        std::string system_string;
        system_string.clear();
        system_string.append("Map");

        if (Map_id<1000)
            system_string.append("0");
        if (Map_id<100)
            system_string.append("0");
        if (Map_id<10)
            system_string.append("0");

        std::stringstream ss;
        ss << Map_id;

        system_string.append(ss.str());
        system_string.append(".lmu");
        // ===[ LOADING MAP DATA ]==============================================
        Map.Load((char *)system_string.c_str(), data);
        Map.ShowInformation(data);
        if(myteam->lmt.tree_list[Map_id].music==2)
        {
            system_string.clear();
            system_string.append("Music/");
            system_string.append(myteam->lmt.tree_list[Map_id].music_file.name.c_str());
            system_string.append(".mid");

            if(myaudio->actual_music.compare((char *)system_string.c_str()))
            {
                myaudio->load((char *)system_string.c_str());
                myaudio->play(-1);
            }
        }
        system_string.clear();
        system_string.append("ChipSet/");
        system_string.append(myteam->data2.Tilesets[(unsigned int) data->ChipsetID - 1].strGraphic);
        system_string.append(".png");
        pre_chip->GenerateFromFile((char *) system_string.c_str());
        Events = &data->vcEvents;
        chip->init(pre_chip->ChipsetSurface, data, &myteam->data2.Tilesets[(unsigned int) data->ChipsetID - 1] );
        Ev_management->init(myaudio,NScene,myteam,Events,Actor,&Mov_management);
        if(data->ParallaxBackground)
        {
            system_string.clear();
            system_string.append("Panorama/");
            system_string.append(data->BackgroundName);
            system_string.append(".png");
            Background->dispose();
            Background->setimg((char *) system_string.c_str());

            if(data->HorizontalPan)
                Background->x= (-320);
            else
                Background->x= 0;

            if(data->VerticalPan)
                Background->y=(-240);
            else
            Background->y=0;
        }
        init_npc();

        Actor->setposXY(myteam->actual_x_map,myteam->actual_y_map, chip,Charas_nps,NScene);
        Actor->set_dir(myteam->actual_dir);

        Mov_management.init(myteam);
        myteam->scroll_active=true;
        Scroll();
    }

    Control::set_delay(0);
    Control::set_in_delay(0);
    Control::in_map = true;

}


void Map_Scene::init_npc()
{
    unsigned int i;
    std::string system_string;
    Chara npc;
    npc.init_Chara();
    SDL_Surface *temp2;
    E_state original_state;
    original_state.Event_Active=false;
    original_state.id_exe_actual=0;
    original_state.id_actual_active=false;
    Charas_nps->clear();

        for(i=0;i<Charas_nps->size();i++)
	 Charas_nps->at(i).dispose();

    Ev_state->clear();

    for (i = 0; i < Events->size(); i++)
    {
        original_state.Active_page=Ev_management->Active_page(&data->vcEvents[i]);

        Ev_state->push_back(original_state);

        if(original_state.Active_page!=-1)
        {
            system_string.clear();
            system_string.append("CharSet/");
            system_string.append(data->vcEvents[i].vcPage[original_state.Active_page].CharsetName);
            system_string.append(".png");
            if (!system_string.compare("CharSet/.png"))
            {
                temp2 = CreateSurface(24, 32);
                chip->RenderTile(temp2, 4, 16, data->vcEvents[i].vcPage[original_state.Active_page].CharsetID + 0x2710, 0);
                npc.set_surface(temp2);
                npc.dir = 0;
                npc.frame = 1;
            }
            else
            {
                npc.setimg((char *) system_string.c_str(), data->vcEvents[i].vcPage[original_state.Active_page].CharsetID);
                npc.dir = data->vcEvents[i].vcPage[original_state.Active_page].Facing_direction;
                npc.frame =data->vcEvents[i].vcPage[original_state.Active_page].Animation_frame + 1;

            }
                npc.move_dir=data->vcEvents[i].vcPage[original_state.Active_page].Movement_type;
                npc.move_frec=data->vcEvents[i].vcPage[original_state.Active_page].Movement_frequency;
                npc.anim_frec=data->vcEvents[i].vcPage[original_state.Active_page].Movement_speed;
                npc.layer=data->vcEvents[i].vcPage[original_state.Active_page].Event_height;
                npc.setposXY(data->vcEvents[i].X_position, data->vcEvents[i].Y_position);
        }
        else
        {
                temp2 = CreateSurface(24, 32);
                //chip.RenderTile(temp2, 4, 16,0x2711, 0);
                npc.set_surface(temp2);

                npc.dir = data->vcEvents[i].vcPage[0].Facing_direction;
                npc.frame = data->vcEvents[i].vcPage[0].Animation_frame;
                npc.move_dir=data->vcEvents[i].vcPage[0].Movement_type;
                npc.move_frec=data->vcEvents[i].vcPage[0].Movement_frequency;
                npc.anim_frec=data->vcEvents[i].vcPage[0].Movement_speed;
                npc.layer=3;
                npc.setposXY(data->MapWidth,data->MapHeight);
        }
                npc.id=data->vcEvents[i].DB_id;
                Charas_nps->push_back(npc);

    }
     Ev_management->init(myaudio,NScene,myteam,Events,Actor,&Mov_management);

}


void Map_Scene::update(SDL_Surface *Screen)
{
    //WE shuold use layers!!
    SDL_FillRect(Screen, NULL, 0x0);// Clear screen  inutil
    unsigned int i;

    if(data->ParallaxBackground)
    {
    if((data->HorizontalAutoPan)&(data->HorizontalPan))
    {
    Background->x+=data->HorizontalPanSpeed;
    if(Background->x>(0))
    Background->x= (-320);
    }
    if((data->VerticalAutoPan)&(data->VerticalPan))
    {
    Background->y-=data->VerticalPanSpeed;
    if(Background->y<(0))
    Background->y= (-240);
    }

    Background->draw(Screen);

    }

    chip->Render(Screen, 0, myteam->view.x, myteam->view.y); //dibuja mapa capa 1 con repecto a la vista
    chip->Render(Screen, 1, myteam->view.x, myteam->view.y);//dibuja mapa capa 2 con repecto a la vista

    for (i = 0; i < Charas_nps->size(); i++)
    {
        Charas_nps->at(i).addx(- myteam->view.x);
        Charas_nps->at(i).addy(- myteam->view.y);
        if (Charas_nps->at(i).layer == 0)
            Charas_nps->at(i).drawc(Screen);
        if ((Charas_nps->at(i).layer == 1) && (Charas_nps->at(i).GridY <= Actor->GridY))
            Charas_nps->at(i).drawc(Screen);
    }

    Actor->drawc(Screen);

    for (i = 0; i < Charas_nps->size(); i++)
    {
        if (Charas_nps->at(i).layer == 2)
            Charas_nps->at(i).drawc(Screen);
        if ((Charas_nps->at(i).layer== 1) && (Charas_nps->at(i).GridY > Actor->GridY))
            Charas_nps->at(i).drawc(Screen);
    }

    for (i = 0; i < Charas_nps->size(); i++)
    {
        Charas_nps->at(i).addx(+ myteam->view.x);
        Charas_nps->at(i).addy(+ myteam->view.y);
    }

    Ev_management->update(Screen);
      myteam->screen_got_refresh=true;

}


void Map_Scene::Scroll()
{
    if(myteam->scroll_active)
    myteam->view.x = Actor->Clamp((int) sll2dbl(Actor->realX) + 20 - (SCREEN_SIZE_X >> 1), 0, (chip->data->MapWidth << 4) - SCREEN_SIZE_X);
    if (!Actor->outofarea)
    {
        Actor->x = (int) sll2dbl(Actor->realX) - myteam->view.x;
    }
    else
    {
        Actor->x = (SCREEN_SIZE_X >> 1) - 20;
    }
    if(myteam->scroll_active)
    myteam->view.y= Actor->Clamp((int) sll2dbl(Actor->realY) + 24 - (SCREEN_SIZE_Y >> 1), 0, (chip->data->MapHeight << 4) - SCREEN_SIZE_Y);
    if (!Actor->outofarea)
    {
        Actor->y = (int) sll2dbl(Actor->realY) - myteam->view.y ;
    }
    else
    {
        Actor->y = (SCREEN_SIZE_Y >> 1) - 24;
    }
}


void Map_Scene::updatekey()
{
    unsigned int i;
    Ev_management->updatekey(running);
    Actor->MoveOnInput(running);
    Scroll();
    for (i = 0; i < Charas_nps->size(); i++)
    {
        if(!Charas_nps->at(i).move_from_event)
        if (!Charas_nps->at(i).move(Charas_nps->at(i).move_dir))
        {
            if ( Charas_nps->at(i).move_frec_check())//till time to move
            {
                Charas_nps->at(i).move_dir= Mov_management.get_dir(i);

                switch (Charas_nps->at(i).move_dir)
                {
                case DIRECTION_UP:
                    Charas_nps->at(i).GridY-=1;
                    break;
                case DIRECTION_DOWN:
                    Charas_nps->at(i).GridY+=1;
                    break;
                case DIRECTION_LEFT:
                    Charas_nps->at(i).GridX-=1;
                    break;
                case DIRECTION_RIGHT:
                    Charas_nps->at(i).GridX+=1;
                    break;
                }

                Charas_nps->at(i).state=true;
            }
        }
        if(Ev_state->at(i).Active_page!=-1)
        {
        if (((data->vcEvents[i].vcPage[0].Animation_type==1)||(data->vcEvents[i].vcPage[Ev_state->at(i).Active_page].Animation_type==3))&&data->vcEvents[i].vcPage[Ev_state->at(i).Active_page].Movement_type==0)
            Charas_nps->at(i).frameupdate();
        if (data->vcEvents[i].vcPage[Ev_state->at(i).Active_page].Animation_type==5)
        {
            Charas_nps->at(i).nomalanimation=false;
            Charas_nps->at(i).rotationupdate();
        }
        }
    }
    mapnpc();

    if(Actor->tried_to_menu)
    {

        if(myteam->able_to_menu)
        {
            myteam->actual_x_map=Actor->GridX;
            myteam->actual_y_map=Actor->GridY;
            myteam->actual_dir=Actor->dir;
            *NScene = 4;
        }
    Actor->tried_to_menu=false;
    }
}

void Map_Scene::active_event(int event_id)
{
                Ev_state->at(event_id).Event_Active=true; // activalo
                Ev_state->at(event_id).id_exe_actual=0;
                Ev_state->at(event_id).id_actual_active=false;
}
void Map_Scene::mapnpc()
{
    unsigned int event_id;
    Event_comand * comand;

    for (event_id=0;event_id< Charas_nps->size();event_id++)//eventos de mapa
    {
        if(Ev_state->at(event_id).Active_page!=-1)
        {
            if(!Ev_state->at(event_id).Event_Active)//activar eventos
            {
                if(data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].Activation_condition==0)
                    if((Actor->tried_to_talk &&(Actor->npc_subcolision(event_id)))) //si cumple con su condicion de activacion
                    {
                    active_event(event_id);
                    Actor->tried_to_talk=false;
                    }
                if((data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].Activation_condition==1)||(data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].Activation_condition==2))
                    if (Actor->npc_subcolision(event_id)) //si cumple con su condicion de activacion
                    {
                    active_event(event_id);
                    }
                if((data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].Activation_condition==3)||(data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].Activation_condition==4))
                {
                    active_event(event_id);
                }
            }


            if(Ev_state->at(event_id).Event_Active)// si el evento esta activo
            {
                if(Ev_state->at(event_id).Recall_states.size()==0)
                {
                if(Ev_state->at(event_id).id_exe_actual< data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].vcEvent_comand.size())
                {
                    comand=data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].vcEvent_comand[Ev_state->at(event_id).id_exe_actual];// lee el comando
                    //activar comandos
                    if(!Ev_state->at(event_id).id_actual_active)  //si el id actual no esta activa pero el evento  si
                    {

                    Ev_state->at(event_id).id_actual_active=true;
                    Ev_management->exec_comand(data->vcEvents[event_id].vcPage[Ev_state->at(event_id).Active_page].vcEvent_comand,event_id,&Ev_state->at(event_id));// mandalo activar

                    if(Ev_state->at(event_id).Recall_states.size()>0)
                    {
                       event_call_event(event_id,true);
                    }
                        if(actual_map!=myteam->actual_map)
                        {
                            dispose();
                            load_map();
                            break;
                            break;
                        }
                    }
                    if(Ev_state->at(event_id).Recall_states.size()==0)
                    if(Ev_state->at(event_id).id_actual_active)// ejecutar comandos
                    {
                        Ev_management->active_exec_comand(comand,event_id,&Ev_state->at(event_id));
                    }
                }
                else
                {
                    Ev_state->at(event_id).Event_Active=false;
                    Ev_state->at(event_id).id_exe_actual=0;
                    Ev_state->at(event_id).id_actual_active=false;
                }
            }else
            {
                event_call_event(event_id,true);
            }
            }

        }
    }
    Actor->tried_to_talk=false;

    for (event_id=0;event_id< myteam->data2.Event.size();event_id++)//eventos de ldb
    {
//si el id de activacion es 3 o 4 el evento deve ser activado
// ademas que si usa una fase de activacion esta deve estar activa
            if(!Evc_state->at(event_id).Event_Active)//si el evento no esta activo
            {
                //pero cumple con las condiciones de activacion
                if((myteam->data2.Event[event_id].intActivation_condition==3)||(myteam->data2.Event[event_id].intActivation_condition==4))
                {
                    if((!myteam->data2.Event[event_id].blActivate_on_switch)||(myteam->state_swich(myteam->data2.Event[event_id].intSwitch_ID-1)))
                    {
                        Evc_state->at(event_id).Event_Active=true; // activalo
                        Evc_state->at(event_id).id_exe_actual=0;
                        Evc_state->at(event_id).id_actual_active=false;
                    }
                }
            }


            if(Evc_state->at(event_id).Event_Active)// si el evento esta activo
            {//si aun no se ejecuta todos
                if(Evc_state->at(event_id).Recall_states.size()==0)
                {
                    if(Evc_state->at(event_id).id_exe_actual< myteam->data2.Event[event_id].vcEvent_comand.size())
                    {
                        comand=myteam->data2.Event[event_id].vcEvent_comand[Evc_state->at(event_id).id_exe_actual];
                        //activar comandos
                        if(!Evc_state->at(event_id).id_actual_active)  //si el id actual no esta activa pero el evento  si
                        {
                            Evc_state->at(event_id).id_actual_active=true;
                            Ev_management->exec_comand(myteam->data2.Event[event_id].vcEvent_comand,event_id,&Evc_state->at(event_id));// mandalo activar
                            if(Evc_state->at(event_id).Recall_states.size()!=0)
                                event_call_event(event_id,false);

                            if(actual_map!=myteam->actual_map)//para teleport
                            {
                                dispose();
                                load_map();
                                break;
                                break;
                            }
                        }
                        if(Evc_state->at(event_id).Recall_states.size()==0)
                        if(Evc_state->at(event_id).id_actual_active)// ejecutar comandos
                        {
                            Ev_management->active_exec_comand(comand,event_id,&Evc_state->at(event_id));
                        }
                    }
                    else
                    {
                        Evc_state->at(event_id).Event_Active=false;
                        Evc_state->at(event_id).id_exe_actual=0;
                        Evc_state->at(event_id).id_actual_active=false;
                    }
                }
                else
                {
                event_call_event(event_id,false);
                }

            }



    }


}

int Map_Scene::event_call_event(int event_id, bool caller)
{


                Event_comand * comand;
                int i,id_to_call,page_to_call,exe_to_call,original_size;
                E_state * my_state;
                if(caller)
                {
                i =Ev_state->at(event_id).Recall_states.size()-1;
                id_to_call=Ev_state->at(event_id).Recall_states[i].event_id;
                page_to_call=Ev_state->at(event_id).Recall_states[i].Active_page;
                exe_to_call=Ev_state->at(event_id).Recall_states[i].id_exe_actual;
                original_size=Ev_state->at(event_id).Recall_states.size();
                my_state= &Ev_state->at(event_id);

                }else
                {
                i =Evc_state->at(event_id).Recall_states.size()-1;
                id_to_call=Evc_state->at(event_id).Recall_states[i].event_id;
                page_to_call=Evc_state->at(event_id).Recall_states[i].Active_page;
                exe_to_call=Evc_state->at(event_id).Recall_states[i].id_exe_actual;
                original_size=Evc_state->at(event_id).Recall_states.size();
                my_state= &Evc_state->at(event_id);
                }

                if(page_to_call != -2)
                {

                    if(exe_to_call< data->vcEvents[id_to_call].vcPage[page_to_call].vcEvent_comand.size())
                    {

                        comand=data->vcEvents[id_to_call].vcPage[page_to_call].vcEvent_comand[exe_to_call];// lee el comando
                        //activar comandos
                        if(!my_state->Recall_states[i].id_actual_active)  //si el id actual no esta activa pero el evento  si
                        {
                            my_state->Recall_states[i].id_actual_active=true;
                            Ev_management->exec_comand(data->vcEvents[id_to_call].vcPage[page_to_call].vcEvent_comand,id_to_call,&my_state->Recall_states[i]);// mandalo activar

                            if(my_state->Recall_states[i].Recall_states.size()!=0)//if they call event on the other event
                            {
                                E_state temp;
                                temp=my_state->Recall_states[i].Recall_states[0];
                                my_state->Recall_states[i].Recall_states.pop_back();
                                my_state->Recall_states.push_back(temp);
                                event_call_event(event_id,caller);
                            }
                            if(actual_map!=myteam->actual_map)
                            {
                            dispose();
                            load_map();
                            return(1);
                            }
                        }

                        if(my_state->Recall_states[i].id_actual_active)// ejecutar comandos
                        {
                        Ev_management->active_exec_comand(comand,id_to_call,&my_state->Recall_states[i]);
                        }
                    }
                    else
                    {
                    my_state->Recall_states.pop_back();
                    }
                }
                else
                {

                if(exe_to_call< myteam->data2.Event[id_to_call].vcEvent_comand.size())
                {
                    comand=myteam->data2.Event[id_to_call].vcEvent_comand[exe_to_call];
                    //activar comandos
                    if(!my_state->Recall_states[i].id_actual_active)  //si el id actual no esta activa pero el evento  si
                    {

                    my_state->Recall_states[i].id_actual_active=true;
                    Ev_management->exec_comand(myteam->data2.Event[id_to_call].vcEvent_comand,id_to_call,&my_state->Recall_states[i]);// mandalo activar

                    if(my_state->Recall_states[i].Recall_states.size()!=0)//if they call event on the other event
                    {
                    E_state temp;
                    temp=my_state->Recall_states[i].Recall_states[0];
                    my_state->Recall_states[i].Recall_states.pop_back();
                    my_state->Recall_states.push_back(temp);
                    event_call_event(event_id,caller);
                    }
                        if(actual_map!=myteam->actual_map)//para teleport
                        {
                            dispose();
                            load_map();
                            return(1);
                        }
                    }
                    if(my_state->Recall_states[i].id_actual_active)// ejecutar comandos
                    {
                        Ev_management->active_exec_comand(comand,id_to_call,&my_state->Recall_states[i]);
                    printf("%d %d %d \n",id_to_call,page_to_call,my_state->Recall_states[i].id_exe_actual);

                    }
                }
                else
                {
                    my_state->Recall_states.pop_back();
                }

                }
    return(0);
}

void Map_Scene::dispose()
{
    //unsigned int i;

    //for (i = 0; i < Charas_nps->size(); i++)
    //{
      //  Charas_nps->at(i).dispose();
    //}
}
