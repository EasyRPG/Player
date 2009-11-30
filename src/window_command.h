#ifndef _H_WINDOW_COMMAND_
#define _H_WINDOW_COMMAND_

#include <vector>
#include "window_selectable.h"

class Window_Command: public Window_Selectable {
public:
    Window_Command(int width, std::vector<std::string> opt);

    ~Window_Command();

protected:
    std::vector<std::string> commands;

};



#endif
