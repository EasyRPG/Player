#ifndef _H_CONTROLC
#define _H_CONTROLC 1

#include "SDL.h"
//#include "e_set.h"
#include <iostream>
#include <deque>

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

    int pop_action();
    void update_keys();

    void set_delay(int d);// {delay = d;}
    int get_delay();// {return delay;}

    void set_in_delay(int d);// {in_delay = d;}
    int get_in_delay();// {return in_delay;}
}


/*class Control
{
    private:

        static SDL_Event event;

        static std::deque<int> events;

        static int n_keys[N_KEYS];

        static int read_key();
        static bool push_action(int);

        static int delay;
        static int in_delay;

        static int in_delay_tmp;


    public:

        static bool stop;
        static bool in_map;

        static int pop_action();
        static void update_keys();

        static void set_delay(int d) {delay = d;}
        static int get_delay() {return delay;}

        static void set_in_delay(int d) {in_delay = d;}
        static int get_in_delay() {return in_delay;}

};*/


#endif
