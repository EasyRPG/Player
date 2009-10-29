#ifndef __input__
#define __input__

class Input {

public:
	static void initialize();

	static void update();
	
	static bool is_pressed(int key);
	static bool is_triggered(int key);
	static bool is_repeated(int key);
	static bool is_released(int key);
	static int dir4();
	static int dir8();
};
#endif // __input__
