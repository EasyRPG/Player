#ifndef ANIMACION_MANAGER_H_
#define ANIMACION_MANAGER_H_

#include "SDL.h"
#include "SDL_rotozoom.h"
#include "../tools/tools.h"
#include "SDL_image.h"
#include <iostream>
#include <stdlib.h>
#include "Animacion.h"
#include "../readers/ldbstr.h"
#include "../tools/Sound_Manager.h"

// =============================================================================
// *****************************************************************************


class Animacion_Manager{

private:
    Animacion anim;
    int actual_frame;
    short delay;
    stcAnimated_battle * Animation_data;
    Sound_Manager * sound_sys;
    void check_sound();
public:
    bool animation_ended;
    int center_X;
    int center_Y;
    std::string system_string;
    Animacion_Manager();
    ~Animacion_Manager();
    void init_Anim(stcAnimated_battle * Animation_ldb,Sound_Manager * sound_m);
    void frameupdate();
    void draw(SDL_Surface *screen);
};

#endif
