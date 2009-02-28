#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include "SDL.h"
#include "SDL_mixer.h"
#include "audio.h"

#define Max_sounds 7

class Sound_Manager
{
    private:
        std:: vector <Sound *> sound_array;
        int last_sound_id;
    public:
        Sound_Manager();
        ~Sound_Manager();
        int load_sound(const char* soundf);
        void play_sound(int num);
};

#endif
