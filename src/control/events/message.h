#ifndef _H_MESSAGE
#define _H_MESSAGE 1
#include "../../interface/Windows/Window_Base.h"
#include "../../attributes/Player_Team.h"


enum m_pos
{
    UP = 0,
    CENTER = 1,
    DOWN = 2
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
        bool cursor_visible;
        std::bitset<255> type_set;
        static message_options opt;
        void draw_blink(SDL_Surface *dst);

    public:
        bool done;
        CMessage(const std::string& sys);
        ~CMessage();
        void draw(SDL_Surface *dst);
        void add_text(const std::string& ctext, int line);
        bool is_done() { return done; }
        void clean();
        
        static void set_transparency(bool b) { opt.transparent = b; }
        static void set_position(m_pos p) { opt.place = p; }
        static void set_detect_hero(bool b) { opt.detect_hero = b; }
};

#endif
