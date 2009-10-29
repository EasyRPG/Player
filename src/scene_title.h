#ifndef __scene_title__
#define __scene_title__

#include "scene.h"

class Scene_Title: public Scene {

public:
	Scene_Title();
	~Scene_Title();

	void main_function();
	void update();
	void command_new_game();
	void command_continue();
	void command_shutdown();
};
#endif // __scene_title__
