#ifndef __scene_title__
#define __scene_title__

#include "scene.h"
#include "window_base.h" // Change for Window_Command when done

class Scene_Title: public Scene {

public:
	Scene_Title();
	~Scene_Title();

	void main_function();
	void update();
	void command_new_game();
	void command_continue();
	void command_shutdown();
	
	//Window_Command command_window;
	Window_Base* command_window;
};
#endif // __scene_title__
