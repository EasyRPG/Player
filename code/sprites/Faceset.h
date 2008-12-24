#ifndef FACESET_H_
#define FACESET_H_

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

 

class Faceset: public Sprite {

public:

    void drawf (SDL_Surface * screen);
    void init_Faceset(int posx,int posy,int theframe);

};


#endif
