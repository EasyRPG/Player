#ifndef _H_PLAYER_
#define _H_PLAYER_

#include <stdio.h>
#include "SDL.h"

#include <new>
#include "main_data.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "scene_title.h"
#include "log.h"

class Player {
public:
    Player();
    ~Player();

    void do_play();
    void set_args(int _argc, char **_argv);

private:
    int argc;
    char **argv;
};

#endif
