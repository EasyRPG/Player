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
        std::bitset<255> type_set;

        static message_options opt;




    public:

        bool is_visible;

        CMessage(const std::string& sys);
        void add_text(const std::string& ctext, int line);

};

#endif
