#ifndef CHARA_H_
#define CHARA_H_



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




class Chara: public Sprite {

private:
    int animation[4][4];//up right down left XP

public:
    int dir;
    void init_Chara();
    void frame_ori();
    void frameupdate();
   	void drawc(SDL_Surface *screen);
};




#endif
