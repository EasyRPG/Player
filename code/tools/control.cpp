#include "control.h"

std::deque<int> Control::events;
SDL_Event Control::event;
int Control::n_keys[N_KEYS] ={0,0,0,0,0,0,0,0};

int Control::delay = 30;
int Control::in_delay = 4;

int Control::in_delay_tmp = 0;

bool Control::stop = false;
bool Control::in_map = false;


int Control::pop_action()
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

void Control::update_keys()
{
    if (stop) return;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
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
                        }                        break;
                    case SDLK_ESCAPE:
                        events.push_back(EXIT);
                        break;


                    default:
                        break;


                }
                break;

            case SDL_KEYUP:
                switch(event.key.keysym.sym)
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
                    default: ;
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
}
