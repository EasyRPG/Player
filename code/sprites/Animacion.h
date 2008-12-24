#ifndef ANIMACION_H_
#define ANIMACION_H_

// *****************************************************************************
// =============================================================================





#include <SDL.h>

#include "../tools/SDL_rotozoom.h"

#include "../tools/tools.h"

#include <SDL_image.h>

#include <stdio.h>

#include <stdlib.h>

#include "sprite.h"


// =============================================================================
// *****************************************************************************
 

class Animacion: public Sprite {

private:
    int maxframe;
    short delay;

public:
    bool endanim;
    void reset();
    void init_Anim(int the_cols,int the_rows);
    void frameupdate();
    void draw(SDL_Surface *screen);
};



#endif
