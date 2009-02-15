#ifndef _H_CONTROLC
#define _H_CONTROLC 1

#include "SDL.h"
#include <bitset>
#include <deque>
#include <iostream>

#define DECISION 0
#define CANCEL 1
#define ARROW_UP 2
#define ARROW_DOWN 3
#define ARROW_LEFT 4
#define ARROW_RIGHT 5
#define SHIFT 6
#define EXIT 7

#define N_KEYS 8


namespace Control
{
    extern bool stop;
    extern bool in_map;
    int pop_LM();
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
