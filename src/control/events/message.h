#ifndef _H_MESSAGE
#define _H_MESSAGE 1
#include "../../interface/Windows/Window_Base.h"

/*enum m_pos
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

} message_options;*/

class Message: public Window_Base
{
    private:
        std::bitset<255> type_set;

    public:

        Message(int SizeX, int SizeY, int PosX, int PosY, const std::string& SysIMg);

        void add_text(const std::string& ctext, int line);

};

#endif
