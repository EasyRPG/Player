#ifndef _H_CONTROLC
#define _H_CONTROLC 1

#include "SDL.h"
#include <bitset>
#include <deque>

#define N_KEYS 8


namespace Control
{
    enum {
        DECISION = 0,
        CANCEL,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        SHIFT,
        EXIT
    };

    extern bool stop;
    extern bool in_map;

    int pop_action();
    void update_keys();

    void set_keys();

    void set_delay_default();
    void set_delay(int d);// {delay = d;}
    int get_delay();// {return delay;}

    void set_in_delay(int d);// {in_delay = d;}
    int get_in_delay();// {return in_delay;}

    void cleanup();
}

#endif
