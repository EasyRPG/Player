/*Window_Player_Select.h, EasyRPG player Window_Player_Select class declaration file.
    Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
	std:: vector <Sprite> Vtext_Sprite;
	int pos_X,pos_Y;
	int Size_X,Size_Y,Size_of_Block;
	int Comand_X,Comand_Y;
	std:: vector <int> V_Sprite;//appuntadores a sprites
	int Cur_pos_X,Cur_pos_Y;
	int Moves_cursorY;
	bool disposing;
	Sprite text, sha_text;

public:
	Menu_Easy menu;

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
	void init(Sound_Manager * theaudio, bool * run,int ComandX,int ComandY,int SizeX,int SizeY,int PosX,int PosY,int curX,int curY,const char *SysIMg);
	void draw(SDL_Surface* Screen);
	void add_text(std::string ctext, int x, int y);
	void add_sprite(Sprite * the_sprite, int x, int y);
	void set_curY(int y);
};

#endif
