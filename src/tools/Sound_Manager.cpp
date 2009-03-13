#include "Sound_Manager.h"



Sound_Manager::Sound_Manager()
{   int i;
Sound * S;
    last_sound_id = 0;
    for(i=0;i<Max_sounds;i++)
    {
        S=new Sound();
        sound_array.push_back(S);
    }


}
Sound_Manager::~Sound_Manager()
{

}

int Sound_Manager::load_sound(const char* soundf)
 {
    unsigned int i;
 for(i=0;i<sound_array.size();i++)
      {
            if(!sound_array[i]->actual_music.compare(soundf))
            return(i);
      }

    if(last_sound_id>=Max_sounds)
    {
        Sound * S;
        S=new Sound();

    i=last_sound_id%Max_sounds;
    delete sound_array[i];
    sound_array.erase(sound_array.begin()+i);
    sound_array.insert(sound_array.begin()+i,S);
    sound_array[i]->load(soundf);
    last_sound_id++;
    return(i);
    }
    sound_array[last_sound_id]->load(soundf);
    last_sound_id++;
    return(last_sound_id-1);
}

void Sound_Manager::play_sound(int num)
{
    sound_array[num]->play(0);
}
