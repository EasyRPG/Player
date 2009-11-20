#ifndef __scene__
#define __scene__

#define SCENE_NULL 0
#define SCENE_TITLE 1
#define SCENE_MAP 2
#define SCENE_MENU 3
#define SCENE_ITEM 4
#define SCENE_SKILL 5
#define SCENE_EQUIP 6
#define SCENE_STATUS 7
#define SCENE_FILE 8
#define SCENE_SAVE 9
#define SCENE_LOAD 10
#define SCENE_END 11
#define SCENE_BATTLE 12
#define SCENE_SHOP 13
#define SCENE_NAME 14
#define SCENE_GAMEOVER 15
#define SCENE_DEBUG 16

class Scene {
public:
	virtual ~Scene() {};

	virtual void main_function() =0;
};

#endif // __scene__
