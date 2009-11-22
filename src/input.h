#ifndef __input__
#define __input__

#include "control.h"

namespace Input {

    extern int actual_action;
    extern int previous_action;

	void initialize();

	void update();
	
	bool is_pressed(int key);
	bool is_triggered(int key);
	bool is_repeated(int key);
	bool is_released(int key);
	int dir4();
	int dir8();
}
#endif // __input__
