#include "input.h"

namespace Input {
    int actual_action = -1;
    int previous_action = -1;

    void initialize()
    {
        Control::set_keys();
        Control::set_delay_default();
    }

    void update()
    {
        previous_action = actual_action;
        Control::update_keys();
        actual_action = Control::pop_action();
    }

    bool is_pressed(int key)
    {
	    return (actual_action == key);
    }

    bool is_triggered(int key)
    {
	    return (actual_action == key);
    }

    bool is_repeated(int key)
    {
	    return false;
    }

    bool is_released(int key)
    {
	    return (actual_action != key);
    }

    int dir4()
    {
	    return 0;
    }

    int dir8()
    {
	    return 0;
    }

}
