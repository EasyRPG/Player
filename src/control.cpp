#include "control.h"

#ifdef PSP

#define TRIANGLE 0
#define CIRCLE 1
#define CROSS 2
#define SQUARE 3
#define LTRIGGER 4
#define RTRIGGER 5
#define DOWN 6
#define LEFT 7
#define UP 8
#define RIGHT 9
#define SELECT 10
#define START 11
#define HOME 12
#define HOLD 13

#define B_DOWN SDL_JOYBUTTONDOWN
#define B_UP SDL_JOYBUTTONUP

#define REG jbutton.button

#define QUANTITY 32


#elif GP2X

#define GP2X_BUTTON_UP 0
#define GP2X_BUTTON_DOWN 4
#define GP2X_BUTTON_LEFT 2
#define GP2X_BUTTON_RIGHT 6
#define GP2X_BUTTON_UPLEFT 1
#define GP2X_BUTTON_UPRIGHT 7
#define GP2X_BUTTON_DOWNLEFT 3
#define GP2X_BUTTON_DOWNRIGHT 5
#define GP2X_BUTTON_CLICK 18
#define GP2X_BUTTON_A 12
#define GP2X_BUTTON_B 13
#define GP2X_BUTTON_X 14
#define GP2X_BUTTON_Y 15
#define GP2X_BUTTON_L 10
#define GP2X_BUTTON_R 11
#define GP2X_BUTTON_START 8
#define GP2X_BUTTON_SELECT 9
#define GP2X_BUTTON_VOLUP 16
#define GP2X_BUTTON_VOLDOWN 17

#define B_DOWN SDL_JOYBUTTONDOWN
#define B_UP SDL_JOYBUTTONUP

#define REG jbutton.button

#define QUANTITY 32


#elif PANDORA

#define PND_RIGHTBUTTON 0
#define PND_LOWBUTTON 1
#define PND_TOPBUTTON 2
#define PND_LEFTBUTTON 3
#define PND_L 4
#define PND_R 5
#define PND_L2 6 // (not supported)
#define PND_R2 7 // (not supported)
#define PND_SELECT 8
#define PND_START 9
#define PND_PANDORA 10

#define B_DOWN SDL_JOYBUTTONDOWN
#define B_UP SDL_JOYBUTTONUP

#define REG jbutton.button

#define QUANTITY 32


#else

// TODO: What if Keyboard and Joystick are both connected? There is only one REG.

#define B_DOWN SDL_KEYDOWN
#define B_UP SDL_KEYUP

#define REG key.keysym.sym

#define QUANTITY 352

#endif

namespace Control
{

    SDL_Event event;
    std::deque<int> events;

    /* Posible Keyboard keys */
    std::bitset<QUANTITY> decision_set;
    std::bitset<QUANTITY> cancel_set;
    std::bitset<QUANTITY> up_set;
    std::bitset<QUANTITY> down_set;
    std::bitset<QUANTITY> right_set;
    std::bitset<QUANTITY> left_set;

    int n_keys[N_KEYS] = {0,0,0,0,0,0,0,0};

    bool stop = false;
    bool in_map = false;

    //SDL_Joystick* joystick;

    int delay = 30;
    int in_delay = 4;

    int in_delay_tmp = 0;

    void cleanup()
    {
        //SDL_JoystickClose(joystick);
    }

    void set_keys()
    {   
        // No Joystick for now
        //SDL_JoystickEventState(SDL_ENABLE);
        //joystick = SDL_JoystickOpen(0);

        #ifdef PSP
        decision_set.set(CROSS);
        decision_set.set(SQUARE);

        cancel_set.set(TRIANGLE);
        cancel_set.set(CIRCLE);

        up_set.set(UP);
        down_set.set(DOWN);
        right_set.set(RIGHT);
        left_set.set(LEFT);

        #elif GP2X

        decision_set.set(GP2X_BUTTON_X);
        decision_set.set(GP2X_BUTTON_A);

        cancel_set.set(GP2X_BUTTON_Y);
        cancel_set.set(GP2X_BUTTON_B);

        up_set.set(GP2X_BUTTON_UP);
        down_set.set(GP2X_BUTTON_DOWN);
        right_set.set(GP2X_BUTTON_RIGHT);
        left_set.set(GP2X_BUTTON_LEFT);

        #elif PANDORA

        decision_set.set(PND_RIGHTBUTTON);
        decision_set.set(PND_LOWBUTTON);

        cancel_set.set(PND_TOPBUTTON);
        cancel_set.set(PND_LEFTBUTTON);

        up_set.set(SDLK_UP);
        down_set.set(SDLK_DOWN);
        right_set.set(SDLK_RIGHT);
        left_set.set(SDLK_LEFT);


        #else

        decision_set.set(SDLK_z);
        decision_set.set(SDLK_SPACE);
        decision_set.set(SDLK_RETURN);

        cancel_set.set(SDLK_x);
        cancel_set.set(SDLK_c);
        cancel_set.set(SDLK_v);
        cancel_set.set(SDLK_b);
        cancel_set.set(SDLK_n);
        cancel_set.set(SDLK_ESCAPE);

        up_set.set(SDLK_UP);
        up_set.set(SDLK_k);
        down_set.set(SDLK_DOWN);
        down_set.set(SDLK_j);
        right_set.set(SDLK_RIGHT);
        right_set.set(SDLK_l);
        left_set.set(SDLK_LEFT);
        left_set.set(SDLK_h);

        #endif
    }

    void set_delay_default()
    {
        //delay = 30
        //in_delay = 4
        delay = 0;
        in_delay = 0;

    }

    void set_delay(int d)
    {
        delay = d;
    }
    int get_delay()
    {
        return delay;
    }

    void set_in_delay(int d)
    {
        in_delay = d;
    }
    int get_in_delay()
    {
        return in_delay;
    }

    int pop_action()
    {
        if (events.empty())
        {
            return -1;
        }
        else
        {
            int tmp;

            tmp = events.front();
            events.pop_front();

            return tmp;
        }
    }

    void update_keys()
    {
        if (stop) return;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                events.push_back(EXIT);
                break;
            case B_DOWN:
                if (decision_set.test(event.REG))
                {
                    if ((n_keys[DECISION] == 0) && (events.empty()))
                    {
                        events.push_back(DECISION);
                        n_keys[DECISION]++;
                    }
                }
                else
                    if (cancel_set.test(event.REG))
                    {
                        if ((n_keys[CANCEL] == 0) && (events.empty()))
                        {
                            events.push_back(CANCEL);
                            n_keys[CANCEL]++;
                        }

                    }
                    else
                        if (up_set.test(event.REG))
                        {
                            if ((n_keys[UP] == 0) && (events.empty()))
                            {
                                events.push_back(UP);
                                n_keys[UP]++;
                            }

                        }
                        else
                            if (down_set.test(event.REG))
                            {
                                if ((n_keys[DOWN] == 0) && (events.empty()))
                                {
                                    events.push_back(DOWN);
                                    n_keys[DOWN]++;
                                }
                            }
                            else
                                if (right_set.test(event.REG))
                                {
                                    if ((n_keys[RIGHT] == 0) && (events.empty()))
                                    {
                                        events.push_back(RIGHT);
                                        n_keys[RIGHT]++;
                                    }
                                }
                                else
                                    if (left_set.test(event.REG))
                                    {
                                        if ((n_keys[LEFT] == 0) && (events.empty()))
                                        {
                                            events.push_back(LEFT);
                                            n_keys[LEFT]++;
                                        }
                                    }
                break;
            case B_UP:
                if (decision_set.test(event.REG))
                    n_keys[DECISION] = 0;
                else
                    if (cancel_set.test(event.REG))
                        n_keys[CANCEL] = 0;
                    else
                        if (up_set.test(event.REG))
                            n_keys[UP] = 0;
                        else
                            if (down_set.test(event.REG))
                                n_keys[DOWN] = 0;
                            else
                                if (right_set.test(event.REG))
                                    n_keys[RIGHT] = 0;
                                else
                                    if (left_set.test(event.REG))
                                        n_keys[LEFT] = 0;
                break;
            default:
                ;
            }
        }

        /* Key repeat */
        int i;
        if ((in_map) && !(events.empty())) return;
        for (i = 0; i < N_KEYS; i++)
        {
            if (n_keys[i] > delay)
            {
                if (in_delay_tmp > in_delay)
                {
                    events.push_back(i);
                    in_delay_tmp = 0;
                }
                else in_delay_tmp++;
            }
            else
            {
                if (n_keys[i] != 0) n_keys[i]++;
            }
        }
    }
}
