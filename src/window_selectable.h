#ifndef _H_WINDOW_SELECTABLE_
#define _H_WINDOW_SELECTABLE_

#include "window_base.h"

class Window_Selectable: public Window_Base {
protected:
    int item_max;
    int column_max;
    int index;

public:
    Window_Selectable(int x, int y, int width, int height):
      Window_Base(x, y, width, height),
        item_max(1), 
        column_max(1),
        index(-1) {}

    ~Window_Selectable() {}

};

#endif
