#include "Animation_Manager.h"

Animacion_Manager::Animacion_Manager()
{
    animation_ended=true;
}
Animacion_Manager::~Animacion_Manager()
{
    anim.dispose();
}

void Animacion_Manager::init_Anim(stcAnimated_battle * Animation_ldb,Sound_Manager * sound_m)
{
    sound_sys=sound_m;
    Animation_data=Animation_ldb;
    system_string.clear();
    system_string.append("Battle/");
    system_string.append(Animation_ldb->strAnimation_file.c_str());
    system_string.append(".png");
    anim.dispose();
    anim.setimg(system_string.c_str());
    anim.init_Anim();
    animation_ended=false;
    actual_frame=0;
    delay=0;
}

void Animacion_Manager::frameupdate()
{
    actual_frame++;
    if(actual_frame>=Animation_data->Framedata.size())
    {
        animation_ended=true;
        actual_frame=0;
    }
}


void Animacion_Manager::check_sound()
{
    int i,j;

for(i=0;i<Animation_data->vecAnimationTiming.size();i++)
    if(Animation_data->vecAnimationTiming[i].Frame==actual_frame)
    {

    system_string.clear();
    system_string.append("Sound/");
    system_string.append(Animation_data->vecAnimationTiming[i].Sound_effect.Name_of_Sound_effect);
    system_string.append(".wav");
    j=sound_sys->load_sound(system_string.c_str());
    sound_sys->play_sound(j);
        break;
    }
}

void Animacion_Manager::draw (SDL_Surface * screen)
{
   if(!animation_ended)
   {
        unsigned int i,j;
        j=Animation_data->Framedata[actual_frame].Cell_data.size();
        for(i=0;i<j;i++)
        {
            anim.frame=Animation_data->Framedata[actual_frame].Cell_data[i].Cell_source;
            anim.x=Animation_data->Framedata[actual_frame].Cell_data[i].X_location+center_X-48;
            anim.y=Animation_data->Framedata[actual_frame].Cell_data[i].Y_location+center_Y-48;
            anim.draw(screen);
        }

        delay++;
        if(delay==3)
        {
        frameupdate();
        check_sound();
        delay=0;
        }
   }
}
