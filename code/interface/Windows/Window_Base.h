#ifndef WINDOW_BASE_H_
#define WINDOW_BASE_H_

#include "../../sprites/Sistem.h"
#include "../../tools/font.h"
#include "../../sprites/sprite.h"
#include <vector>
#include <string>

class Window_Base  {

protected:
	Sistem System;
	Sprite tapiz;
	Font fuente;
	int pos_X,pos_Y;
	int Size_X,Size_Y;
	std:: vector <Sprite> Vtext_Sprite;
	std:: vector <int> V_Sprite;//appuntadores a sprites
	Sprite text;
	char stringBuffer[255];
	int falla;
	bool disposing;

public:
	//Window_Base();		///constructor
	//~Window_Base();	///destructor
	bool visible;
	void dispose();
	void init(int SizeX,int SizeY,int PosX,int PosY);
	void draw(SDL_Surface* Screen);
	void add_text(const char * ctext, int x, int y);
	void add_sprite(Sprite * the_sprite, int x, int y);

}; 

#endif
