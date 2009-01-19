#include "control.h"

namespace Control
{

    SDL_Event event;
    std::deque<int> events;

    /* Posible Keyboard keys */
    Set decision_set(352);
    Set cancel_set(352);
    Set up_set(352);
    Set down_set(352);
    Set right_set(352);
    Set left_set(352);

    int n_keys[N_KEYS] = {0,0,0,0,0,0,0,0};

    bool stop = false;
    bool in_map = false;

    int delay = 30;
    int in_delay = 4;

    int in_delay_tmp = 0;

    void set_keys()
    {
        decision_set.insert(SDLK_z);
        decision_set.insert(SDLK_SPACE);
        decision_set.insert(SDLK_RETURN);

        cancel_set.insert(SDLK_x);
        cancel_set.insert(SDLK_c);
        cancel_set.insert(SDLK_v);
        cancel_set.insert(SDLK_b);
        cancel_set.insert(SDLK_n);
        cancel_set.insert(SDLK_ESCAPE);

        up_set.insert(SDLK_UP);
        up_set.insert(SDLK_k);
        down_set.insert(SDLK_DOWN);
        down_set.insert(SDLK_j);
        right_set.insert(SDLK_RIGHT);
        right_set.insert(SDLK_l);
        left_set.insert(SDLK_LEFT);
        left_set.insert(SDLK_h);
    }

    void set_delay_default()
    {
        delay = 30;
        in_delay = 4;

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
            case SDL_KEYDOWN:
                if (decision_set.belongs(event.key.keysym.sym))
                {
                    if ((n_keys[DECISION] == 0) && (events.empty()))
                    {
                        events.push_back(DECISION);
                        n_keys[DECISION]++;
                    }
                }
                else
                    if (cancel_set.belongs(event.key.keysym.sym))
                    {
                        if ((n_keys[CANCEL] == 0) && (events.empty()))
                        {
                            events.push_back(CANCEL);
                            n_keys[CANCEL]++;
                        }

                    }
                    else
                        if (up_set.belongs(event.key.keysym.sym))
                        {
                            if ((n_keys[ARROW_UP] == 0) && (events.empty()))
                            {
                                events.push_back(ARROW_UP);
                                n_keys[ARROW_UP]++;
                            }

                        }
                        else
                            if (down_set.belongs(event.key.keysym.sym))
                            {
                                if ((n_keys[ARROW_DOWN] == 0) && (events.empty()))
                                {
                                    events.push_back(ARROW_DOWN);
                                    n_keys[ARROW_DOWN]++;
                                }
                            }
                            else
                                if (right_set.belongs(event.key.keysym.sym))
                                {
                                    if ((n_keys[ARROW_RIGHT] == 0) && (events.empty()))
                                    {
                                        events.push_back(ARROW_RIGHT);
                                        n_keys[ARROW_RIGHT]++;
                                    }
                                }
                                else
                                    if (left_set.belongs(event.key.keysym.sym))
                                    {
                                        if ((n_keys[ARROW_LEFT] == 0) && (events.empty()))
                                        {
                                            events.push_back(ARROW_LEFT);
                                            n_keys[ARROW_LEFT]++;
                                        }
                                    }
                break;
            case SDL_KEYUP:
                if (decision_set.belongs(event.key.keysym.sym))
                    n_keys[DECISION] = 0;
                else
                    if (cancel_set.belongs(event.key.keysym.sym))
                        n_keys[CANCEL] = 0;
                    else
                        if (up_set.belongs(event.key.keysym.sym))
                            n_keys[ARROW_UP] = 0;
                        else
                            if (down_set.belongs(event.key.keysym.sym))
                                n_keys[ARROW_DOWN] = 0;
                            else
                                if (right_set.belongs(event.key.keysym.sym))
                                    n_keys[ARROW_RIGHT] = 0;
                                else
                                    if (left_set.belongs(event.key.keysym.sym))
                                        n_keys[ARROW_LEFT] = 0;
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

    /*void update_keys()
    {
        if (stop) return;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                events.push_back(EXIT);
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                    if ((n_keys[ARROW_UP] == 0) && (events.empty()))
                    {
                        events.push_back(ARROW_UP);
                        n_keys[ARROW_UP]++;
                    }

                    break;
                case SDLK_DOWN:
                    if ((n_keys[ARROW_DOWN] == 0) && (events.empty()))
                    {
                        events.push_back(ARROW_DOWN);
                        n_keys[ARROW_DOWN]++;
                    }
                    break;
                case SDLK_RIGHT:
                    if ((n_keys[ARROW_RIGHT] == 0) && (events.empty()))
                    {
                        events.push_back(ARROW_RIGHT);
                        n_keys[ARROW_RIGHT]++;
                    }
                    break;
                case SDLK_LEFT:
                    if ((n_keys[ARROW_LEFT] == 0) && (events.empty()))
                    {
                        events.push_back(ARROW_LEFT);
                        n_keys[ARROW_LEFT]++;
                    }
                    break;
                case SDLK_z:
                    if ((n_keys[DECISION] == 0) && (events.empty()))
                    {
                        events.push_back(DECISION);
                        n_keys[DECISION]++;
                    }
                    break;
                case SDLK_x:
                    if ((n_keys[CANCEL] == 0) && (events.empty()))
                    {
                        events.push_back(CANCEL);
                        n_keys[CANCEL]++;
                    }
                    break;
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    if ((n_keys[SHIFT] == 0) && (events.empty()))
                    {
                        events.push_back(SHIFT);
                        n_keys[SHIFT]++;
                    }
                    break;
                case SDLK_ESCAPE:
                    events.push_back(EXIT);
                    break;


                default:
                    break;


                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                    n_keys[ARROW_UP] = 0;
                    break;
                case SDLK_DOWN:
                    n_keys[ARROW_DOWN] = 0;
                    break;
                case SDLK_z:
                    n_keys[DECISION] = 0;
                    break;
                case SDLK_x:
                    n_keys[CANCEL] = 0;
                    break;
                case SDLK_LEFT:
                    n_keys[ARROW_LEFT] = 0;
                    break;
                case SDLK_RIGHT:
                    n_keys[ARROW_RIGHT] = 0;
                    break;
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    n_keys[SHIFT] = 0;
                    break;
                default:
                    ;
                }

            }
        }
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
    }*/
}
