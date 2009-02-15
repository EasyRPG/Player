#ifndef _H_MESSAGE
#define _H_MESSAGE 1
#include "../../interface/Windows/Window_Base.h"

enum m_pos
{
    UP,
    CENTER,
    DOWN
};

typedef struct
{
    bool transparent;
    m_pos place;
    bool detect_hero;

} message_options;

class CMessage: public Window_Base
{
    private:
        bool next;
        int blink;
        bool cursor;
        std::bitset<255> type_set;
        static message_options opt;

    public:
        bool done;
        CMessage(const std::string& sys);
        ~CMessage();

        void draw(SDL_Surface *dst);
        void draw_blink(SDL_Surface *dst);
        void add_text(const std::string& ctext, int line);
        bool is_done() { return done; }
        void clean();
//        bool next_command { return next; }

};

#endif
