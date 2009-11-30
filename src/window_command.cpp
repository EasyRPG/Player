#include "window_command.h"

Window_Command::Window_Command(int width, std::vector<std::string> opt):
    Window_Selectable(0, 0, width, opt.size() * 32 + 32)
{
    item_max = opt.size();
    commands = opt;
    index = 0;
}
