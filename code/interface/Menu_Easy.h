#ifndef MENU_EASY_H_
#define MENU_EASY_H_

#include "../tools/audio.h"
#include "../tools/key.h"

typedef struct menucomand
{
	int des1;
	int des2;
	int selected_moster;
} MC;

class Menu_Easy {

protected:
	int indexX,indexY;
	int maxindexY,maxindexX;
	Audio * myaudio;
	bool * running;
	bool desided;
	int delay;

public:
	//Menu_Easy();		///constructor
	//~Menu_Easy();		///destructor
	bool desition();
	int getindexY();
	int getindexX();
	void updatekey();
	void restarmenu();
	void init(Audio * theaudio, bool * run,int X,int Y);
};

#endif
