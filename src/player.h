#ifndef _H_PLAYER_
#define _H_PLAYER_

#include <stdio.h>
#include "SDL.h"

#include "main_data.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "scene_title.h"

class Player {
public: 
    Player(int _argc, char *_argv[]);
    ~Player();

    void do_play();
    
private:
    int argc;
    char **argv;
};

#endif
