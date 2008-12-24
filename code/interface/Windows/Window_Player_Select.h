#ifndef WINDOW_PLAYER_SELECT_H_
#define WINDOW_PLAYER_SELECT_H_

#include "../../sprites/Sistem.h"
#include "../Menu_Easy.h"
#include "../../sprites/Sistem.h"
#include "../../tools/font.h"
#include "../../sprites/sprite.h"
#include <vector>
#include <string>


class Window_Player_Select   
{
protected:
	Sistem System;
	Sprite tapiz;
	Sprite cursor;
	Font fuente;
	Menu_Easy menu;
	std:: vector <Sprite> Vtext_Sprite;
	int pos_X,pos_Y;
	int Size_X,Size_Y,Size_of_Block;
	int Comand_X,Comand_Y;
	std:: vector <int> V_Sprite;//appuntadores a sprites
	int Cur_pos_X,Cur_pos_Y;
	int Moves_cursorY;
	bool disposing;
	Sprite text;

public:
	//Window_Player_Select();		///constructor
	///~Window_Player_Select();		///destructor
	bool visible;
	bool visible_window;
	void updatekey();
	int getindexX();
	int getindexY();
	void restarmenu();
	bool desition();
	void dispose();
	void init_curXY(int x,int y);
	void init(Audio * theaudio, bool * run,int ComandX,int ComandY,int SizeX,int SizeY,int PosX,int PosY,int curX,int curY);
	void draw(SDL_Surface* Screen);
	void add_text(const char * ctext, int x, int y);
	void add_sprite(Sprite * the_sprite, int x, int y);
	void set_curY(int y);
};

#endif
